
#include "gameboy/cpu.h"

#include <stdexcept>
#include <string>
#include <cstring>
#include <iostream>

#define WORD(hi, lo) ( (((hi) & 0xFFFF) << 8) | ((lo) & 0xFFFF) )

#include "bitutil.h"
#include "shiftrotate.h"

namespace gb
{
	CPU::CPU(const MMU::Ptr& mmu, const GPU::Ptr gpu) :
		mmu_(mmu),
		gpu_(gpu),
		alu_(af_.lo),
		lcd_(*mmu_.get()),
		timer_(*mmu.get()),
		halted_(false),
		stopped_(false),
		interrupt_master_enable_(false),
		interrupt_master_enable_pending_(-1),
		interrupt_master_disable_pending_(-1),
		debug_mode_(false),
		cycle_count_(0),
		interrupt_flags_(mmu_->get(memorymap::INTERRUPT_FLAG)),
		interrupt_enable_(mmu_->get(memorymap::INTERRUPT_ENABLE)),
		div_(nullptr),
		cgb_mode_(false)
	{
		reset();
	}

	void CPU::step()
	{
		uint8_t cycles = 1;

		if (!halted_)
		{
			// fetch next opcode
			uint8_t opcode = mmu_->read(pc_.val++);

			// $CB means decode from the second page of instructions
			if (opcode != 0xCB)
			{
				// decode from first page
				decode1(opcode);

				// look up the number of cycles for this opcode
				cycles = getOpcodeInfo(opcode, OpcodePage::PAGE1).cycles;
			}
			else
			{
				// read the second page opcode
				opcode = mmu_->read(pc_.val++);
				// decode from second page
				decode2(opcode);

				// look up the number of cycles for this opcode
				cycles = getOpcodeInfo(opcode, OpcodePage::PAGE2).cycles;
			}
		}

		if (!stopped_)
		{
			div_->val += cycles;
		//	lcd_.clock(cycles, interrupt_master_enable_);
			gpu_->update(cycles, interrupt_master_enable_);
			timer_.clock(cycles);
		}

		checkPowerMode();
		checkInterrupts();
	}

	void CPU::decode1(uint8_t opcode)
	{
		static uint16_t old_pc;

		// store current program counter location so it can be reused for disassembly output
		old_pc = pc_.val;

		switch (opcode)
		{
		// NOP
		case 0x00:
			break;
		// STOP
		case 0x10:
			stop();
			break;

		/* Load Instructions */

		// 8 bit loads immediate
		case 0x3E: // LD A,d8
			af_.hi = load8Imm();
			break;
		case 0x06: // LD B,d8
			bc_.hi = load8Imm();
			break;
		case 0x0E: // LD C,d8
			bc_.lo = load8Imm();
			break;
		case 0x16: // LD D,d8
			de_.hi = load8Imm();
			break;
		case 0x1E: // LD E,d8
			de_.lo = load8Imm();
			break;
		case 0x26: // LD H,d8
			hl_.hi = load8Imm();
			break;
		case 0x2E: // LD L,d8
			hl_.lo = load8Imm();
			break;
		case 0x36: // LD (HL),d8
			mmu_->write(load8Imm(), hl_.val);
			break;

		// load 16 bit immediate
		case 0x01: // LD BC,d16
			bc_.val = load16Imm();
			break;
		case 0x11: // LD DE,d16
			de_.val = load16Imm();
			break;
		case 0x21: // LD HL,d16
			hl_.val = load16Imm();
			break;
		case 0x31: // LD SP,d16
			sp_.val = load16Imm();
			break;

		// load A into memory
		case 0x02:
			mmu_->write(af_.hi, bc_.val);
			break;
		case 0x12:
			mmu_->write(af_.hi, de_.val);
			break;

		// load A from memory
		case 0x0A: // LD A,(BC)
			af_.hi = mmu_->read(bc_.val);
			break;
		case 0x1A: // LD A,(DE)
			af_.hi = mmu_->read(de_.val);
			break;

		// transfer (Register to register, memory to register)
		case 0x40: // LD B,B
			bc_.hi = bc_.hi;
			break;
		case 0x41: // LD B,C
			bc_.hi = bc_.lo;
			break;
		case 0x42: // LD B,D
			bc_.hi = de_.hi;
			break;
		case 0x43: // LD B,E
			bc_.hi = de_.lo;
			break;
		case 0x44: // LD B,H
			bc_.hi = hl_.hi;
			break;
		case 0x45: // LD B,L
			bc_.hi = hl_.lo;
			break;
		case 0x46: // LD B,(HL)
			bc_.hi = mmu_->read(hl_.val);
			break;
		case 0x47: // LD B,A
			bc_.hi = af_.hi;
			break;
		case 0x48: // LD C,B
			bc_.lo = bc_.hi;
			break;
		case 0x49: // LD C,C
			bc_.lo = bc_.lo;
			break;
		case 0x4A: // LD C,D
			bc_.lo = de_.hi;
			break;
		case 0x4B: // LD C,E
			bc_.lo = de_.lo;
			break;
		case 0x4C: // LD C,H
			bc_.lo = hl_.hi;
			break;
		case 0x4D: // LD C,L
			bc_.lo = hl_.lo;
			break;
		case 0x4E: // LD C,(HL)
			bc_.lo = mmu_->read(hl_.val);
			break;
		case 0x4F: // LD C,A
			bc_.lo = af_.hi;
			break;

		case 0x50: // LD D,B
			de_.hi = bc_.hi;
			break;
		case 0x51: // LD D,C
			de_.hi = bc_.lo;
			break;
		case 0x52: // LD D,D
			de_.hi = de_.hi;
			break;
		case 0x53: // LD D,E
			de_.hi = de_.lo;
			break;
		case 0x54: // LD D,H
			de_.hi = hl_.hi;
			break;
		case 0x55: // LD D,L
			de_.hi = hl_.lo;
			break;
		case 0x56: // LD D,(HL)
			de_.hi = mmu_->read(hl_.val);
			break;
		case 0x57: // LD D,A
			de_.hi = af_.hi;
			break;
		case 0x58: // LD E,B
			de_.lo = bc_.hi;
			break;
		case 0x59: // LD E,C
			de_.lo = bc_.lo;
			break;
		case 0x5A: // LD E,D
			de_.lo = de_.hi;
			break;
		case 0x5B: // LD E,E
			de_.lo = de_.lo;
			break;
		case 0x5C: // LD E,H
			de_.lo = hl_.hi;
			break;
		case 0x5D: // LD E,L
			de_.lo = hl_.lo;
			break;
		case 0x5E: // LD E,(HL)
			de_.lo = mmu_->read(hl_.val);
			break;
		case 0x5F: // LD E,A
			de_.lo = af_.hi;
			break;

		case 0x60: // LD H,B
			hl_.hi = bc_.hi;
			break;
		case 0x61: // LD H,C
			hl_.hi = bc_.lo;
			break;
		case 0x62: // LD H,D
			hl_.hi = de_.hi;
			break;
		case 0x63: // LD H,E
			hl_.hi = de_.lo;
			break;
		case 0x64: // LD H,H
			hl_.hi = hl_.hi;
			break;
		case 0x65: // LD H,L
			hl_.hi = hl_.lo;
			break;
		case 0x66: // LD H,(HL)
			hl_.hi = mmu_->read(hl_.val);
			break;
		case 0x67: // LD H,A
			hl_.hi = af_.hi;
			break;
		case 0x68: // LD L,B
			hl_.lo = bc_.hi;
			break;
		case 0x69: // LD L,C
			hl_.lo = bc_.lo;
			break;
		case 0x6A: // LD L,D
			hl_.lo = de_.hi;
			break;
		case 0x6B: // LD L,E
			hl_.lo = de_.lo;
			break;
		case 0x6C: // LD L,H
			hl_.lo = hl_.hi;
			break;
		case 0x6D: // LD L,L
			hl_.lo = hl_.lo;
			break;
		case 0x6E: // LD L,(HL)
			hl_.lo = mmu_->read(hl_.val);
			break;
		case 0x6F: // LD L,A
			hl_.lo = af_.hi;
			break;

		case 0x78: // LD A,B
			af_.hi = bc_.hi;
			break;
		case 0x79: // LD A,C
			af_.hi = bc_.lo;
			break;
		case 0x7A: // LD A,D
			af_.hi = de_.hi;
			break;
		case 0x7B: // LD A,E
			af_.hi = de_.lo;
			break;
		case 0x7C: // LD A,H
			af_.hi = hl_.hi;
			break;
		case 0x7D: // LD A,L
			af_.hi = hl_.lo;
			break;
		case 0x7E: // LD A,(HL)
			af_.hi = mmu_->read(hl_.val);
			break;
		case 0x7F: // LD A,A
			af_.hi = af_.hi;
			break;

			// register to memory
		case 0x70: // LD (HL),B
			mmu_->write(bc_.hi, hl_.val);
			break;
		case 0x71: // LD (HL),C
			mmu_->write(bc_.lo, hl_.val);
			break;
		case 0x72: // LD (HL),D
			mmu_->write(de_.hi, hl_.val);
			break;
		case 0x73: // LD (HL),E
			mmu_->write(de_.lo, hl_.val);
			break;
		case 0x74: // LD (HL),H
			mmu_->write(hl_.hi, hl_.val);
			break;
		case 0x75: // LD (HL),L
			mmu_->write(hl_.lo, hl_.val);
			break;
		case 0x77: // LD (HL),A
			mmu_->write(af_.hi, hl_.val);
			break;

		// Load Increment/Decrement
		// (HL+/-) <- A & A <- (HL+/-)
		case 0x22: // LD (HL+),A
			mmu_->write(af_.hi, hl_.val++);
			break;
		case 0x32: // LD (HL-),A
			mmu_->write(af_.hi, hl_.val--);
			break;
		case 0x2A: // LD A,(HL+)
			af_.hi = mmu_->read(hl_.val++);
			break;
		case 0x3A: // LD A,(HL-)
			af_.hi = mmu_->read(hl_.val--);
			break;

		// IN/OUT Instructions. Load and Store to IO Registers (immediate or using C register). IO Offset is $FF00
		case 0xE0: // LDH (a8),A
			out(0xFF00 + load8Imm());
			break;
		case 0xF0: // LDH A,(a8)
			in(0xFF00 + load8Imm());
			break;
		case 0xE2: // LD (C),A
			out(0xFF00 + bc_.lo);
			break;
		case 0xF2: // LD A,(C)
			in(0xFF00 + bc_.lo);
			break;
		case 0xEA: // LD (a16),A
			out(load16Imm());
			break;
		case 0xFA: // LD A,(a16)
			in(load16Imm());
			break;

		/* Increment Instruction */

		// 16 bit increment
		case 0x03: // INC BC
			inc(bc_.val);
			break;
		case 0x13: // INC DE
			inc(de_.val);
			break;
		case 0x23: // INC HL
			inc(hl_.val);
			break;
		case 0x33: // INC SP
			inc(sp_.val);
			break;

		// 16 bit decrement
		case 0x0B: // DEC BC
			dec(bc_.val);
			break;
		case 0x1B: // DEC DE
			dec(de_.val);
			break;
		case 0x2B: // DEC HL
			dec(hl_.val);
			break;
		case 0x3B: // DEC SP
			dec(sp_.val);
			break;

		// 8 bit increment
		case 0x04: // INC B
			inc(bc_.hi);
			break;
		case 0x0C: // INC C
			inc(bc_.lo);
			break;
		case 0x14: // INC D
			inc(de_.hi);
			break;
		case 0x1C: // INC E
			inc(de_.lo);
			break;
		case 0x24: // INC H
			inc(hl_.hi);
			break;
		case 0x2C: // INC L
			inc(hl_.lo);
			break;
		case 0x34: // INC (HL)
			inca(hl_.val);
			break;
		case 0x3C: // INC A
			inc(af_.hi);
			break;

		// 8 bit decrement
		case 0x05: // DEC B
			dec(bc_.hi);
			break;
		case 0x0D: // DEC C
			dec(bc_.lo);
			break;
		case 0x15: // DEC D
			dec(de_.hi);
			break;
		case 0x1D: // DEC C
			dec(de_.lo);
			break;
		case 0x25: // DEC H
			dec(hl_.hi);
			break;
		case 0x2D: // DEC L
			dec(hl_.lo);
			break;
		case 0x35: // DEC (HL)
			deca(hl_.val);
			break;
		case 0x3D: // DEC A
			dec(af_.hi);
			break;

		/* Stack Instructions */

		// Push
		case 0xC5: // PUSH BC
			push(bc_.val);
			break;
		case 0xD5: // PUSH DE
			push(de_.val);
			break;
		case 0xE5: // PUSH HL
			push(hl_.val);
			break;
		case 0xF5: // PUSH AF
			push(af_.val);
			break;

		// Pop
		case 0xC1: // POP BC
			bc_.val = pop();
			break;
		case 0xD1: // POP DE
			de_.val = pop();
			break;
		case 0xE1: // POP HL
			hl_.val = pop();
			break;
		case 0xF1: // POP AF
			af_.val = pop();
			af_.lo &= 0xF0; // explicitly clear lower 4 bits
			break;

		// Load

		case 0x08: // LD (a16),SP
			mmu_->write(sp_.val, load16Imm());
			break;
		case 0xF8: // LD HL,SP+r8
			//hl_.val = (uint16_t)((int16_t)sp_.val + (int8_t)load8Imm());
			hl_.val = ldHLSPe();
			break;
		case 0xF9: // LD SP,HL
			sp_.val = hl_.val;
			break;

		case 0x76:
			halted_ = true;
			break;

		/* Jumps */
		case 0xC3: // JP a16
			jp(load16Imm());
			break;

		case 0xE9:	// JP (HL)
			jp(hl_.val);
			break;

		// conditional jumps
		case 0xC2: // JP NZ,nn
			if (IS_CLR(af_.lo, Flags::Z))
				jp(load16Imm());
			else
				pc_.val += 2;
			break;
		case 0xCA: // JP Z,nn
			if (IS_SET(af_.lo, Flags::Z))
				jp(load16Imm());
			else
				pc_.val += 2;
			break;
		case 0xD2: // JP NC,nn
			if (IS_CLR(af_.lo, Flags::C))
				jp(load16Imm());
			else
				pc_.val += 2;
			break;
		case 0xDA: // JP C,nn
			if (IS_SET(af_.lo, Flags::C))
				jp(load16Imm());
			else
				pc_.val += 2;
			break;

		// relative jumps
		case 0x18: // JR r8
			jr((int8_t)load8Imm());
			break;

		// relative conditional jumps
		case 0x20: // JR NZ,n
			if (IS_CLR(af_.lo, Flags::Z))
				jr((int8_t)load8Imm());
			else
				pc_.val++; // skip next byte
			break;
		case 0x28: // JR Z,n
			if (IS_SET(af_.lo, Flags::Z))
				jr((int8_t)load8Imm());
			else
				pc_.val++; // skip next byte
			break;
		case 0x30: // JR NC,n
			if(IS_CLR(af_.lo, Flags::C))
				jr((int8_t)load8Imm());
			else
				pc_.val++; // skip next byte
			break;
		case 0x38: // JR C,n
			if (IS_SET(af_.lo, Flags::C))
				jr((int8_t)load8Imm());
			else
				pc_.val++; // skip next byte
			break;

		/* Call */
		case 0xCD: // CALL nn
			call(load16Imm());
			break;

		// call condition
		case 0xC4: // CALL NZ,nn
			if (IS_CLR(af_.lo, Flags::Z))
				call(load16Imm());
			else
				pc_.val += 2;
			break;
		case 0xCC: // CALL Z,nn
			if (IS_SET(af_.lo, Flags::Z))
				call(load16Imm());
			else
				pc_.val += 2;
			break;
		case 0xD4: // CALL NC,nn
			if (IS_CLR(af_.lo, Flags::C))
				call(load16Imm());
			else
				pc_.val += 2;
			break;
		case 0xDC: // CALL C,nn
			if (IS_SET(af_.lo, Flags::C))
				call(load16Imm());
			else
				pc_.val += 2;
			break;

		/* Returns */
		case 0xC9: // RET
			ret();
			break;

		// conditional returns
		case 0xC0: // RET NZ
			if (IS_CLR(af_.lo, Flags::Z)) ret();
			break;
		case 0xC8: // RET Z
			if (IS_SET(af_.lo, Flags::Z)) ret();
			break;
		case 0xD0: // RET NC
			if (IS_CLR(af_.lo, Flags::C)) ret();
			break;
		case 0xD8: // RET C
			if (IS_SET(af_.lo, Flags::C)) ret();
			break;

		// return from interrupt
		case 0xD9: // RETI
			reti();
			break;

		/* Reset Instructions */
		case 0xC7: // RST $00
			call(0x00);
			break;
		case 0xCF: // RST $08
			call(0x08);
			break;
		case 0xD7: // RST $10
			call(0x10);
			break;
		case 0xDF: // RST $18
			call(0x18);
			break;
		case 0xE7: // RST $20
			call(0x20);
			break;
		case 0xEF: // RST $28
			call(0x28);
			break;
		case 0xF7: // RST $30
			call(0x30);
			break;
		case 0xFF: // RST $38
			call(0x38);
			break;

		/* Decimal Adjust */
		case 0x27:
			daa();
			break;

		/* Complement */

		// Register A
		case 0x2F: // CPL
			TGL(af_.hi, 0xFF);
			SET(af_.lo, CPU::Flags::N);
			SET(af_.lo, CPU::Flags::H);
			break;
		// Carry Flag
		case 0x3F: // CCF
			TGL(af_.lo, CPU::Flags::C);
			CLR(af_.lo, CPU::Flags::N);
			CLR(af_.lo, CPU::Flags::H);
			break;

		/* Set Carry Flag */
		case 0x37: // SCF
			SET(af_.lo, CPU::Flags::C);
			CLR(af_.lo, CPU::Flags::N);
			CLR(af_.lo, CPU::Flags::H);
			break;

		/* Disable and Enable Interrupt */
		case 0xF3: // DI
			interrupt_master_disable_pending_ = 0;
			break;
		case 0xFB: // EI
			interrupt_master_enable_pending_ = 0;
			break;

		/* Arithmetic Operations */
		// add 8 bit
		case 0x87: // ADD A,A
			alu_.add(af_.hi, af_.hi);
			break;
		case 0x80: // ADD A,B
			alu_.add(af_.hi, bc_.hi);
			break;
		case 0x81: // ADD A,C
			alu_.add(af_.hi, bc_.lo);
			break;
		case 0x82: // ADD A,D
			alu_.add(af_.hi, de_.hi);
			break;
		case 0x83: // ADD A,E
			alu_.add(af_.hi, de_.lo);
			break;
		case 0x84: // ADD A,H
			alu_.add(af_.hi, hl_.hi);
			break;
		case 0x85: // ADD A,L
			alu_.add(af_.hi, hl_.lo);
			break;
		case 0x86: // ADD A,(HL)
			alu_.add(af_.hi, mmu_->read(hl_.val));
			break;
		case 0xC6: // ADD A,n
			alu_.add(af_.hi, load8Imm());
			break;

		// add with carry
		case 0x8F: // ADC A,A
			alu_.addc(af_.hi, af_.hi);
			break;
		case 0x88: // ADC A,B
			alu_.addc(af_.hi, bc_.hi);
			break;
		case 0x89: // ADC A,C
			alu_.addc(af_.hi, bc_.lo);
			break;
		case 0x8A: // ADC A,D
			alu_.addc(af_.hi, de_.hi);
			break;
		case 0x8B: // ADC A,E
			alu_.addc(af_.hi, de_.lo);
			break;
		case 0x8C: // ADC A,H
			alu_.addc(af_.hi, hl_.hi);
			break;
		case 0x8D: // ADC A,L
			alu_.addc(af_.hi, hl_.lo);
			break;
		case 0x8E: // ADC A,(HL)
			alu_.addc(af_.hi, mmu_->read(hl_.val));
			break;
		case 0xCE: // ADC A,n
			alu_.addc(af_.hi, load8Imm());
			break;

		// 16 bit addition
		case 0x09: // ADD HL,BC
			alu_.add(hl_.val, bc_.val);
			break;
		case 0x19: // ADD HL,DE
			alu_.add(hl_.val, de_.val);
			break;
		case 0x29: // ADD HL,HL
			alu_.add(hl_.val, hl_.val);
			break;
		case 0x39: // ADD HL,SP
			alu_.add(hl_.val, sp_.val);
			break;

		case 0xE8: // ADD SP,n
			alu_.addr(sp_.val, (int8_t)load8Imm());
			break;

		// subtract
		case 0x97: // SUB A,A
			alu_.sub(af_.hi, af_.hi);
			break;
		case 0x90: // SUB A,B
			alu_.sub(af_.hi, bc_.hi);
			break;
		case 0x91: // SUB A,C
			alu_.sub(af_.hi, bc_.lo);
			break;
		case 0x92: // SUB A,D
			alu_.sub(af_.hi, de_.hi);
			break;
		case 0x93: // SUB A,E
			alu_.sub(af_.hi, de_.lo);
			break;
		case 0x94: // SUB A,H
			alu_.sub(af_.hi, hl_.hi);
			break;
		case 0x95: // SUB A,L
			alu_.sub(af_.hi, hl_.lo);
			break;
		case 0x96: // SUB A,(HL)
			alu_.sub(af_.hi, mmu_->read(hl_.val));
			break;
		case 0xD6: // SUB A,n
			alu_.sub(af_.hi, load8Imm());
			break;

		// substract with carry
		case 0x9F: // SBC A,A
			alu_.subc(af_.hi, af_.hi);
			break;
		case 0x98: // SBC A,B
			alu_.subc(af_.hi, bc_.hi);
			break;
		case 0x99: // SBC A,C
			alu_.subc(af_.hi, bc_.lo);
			break;
		case 0x9A: // SBC A,D
			alu_.subc(af_.hi, de_.hi);
			break;
		case 0x9B: // SBC A,E
			alu_.subc(af_.hi, de_.lo);
			break;
		case 0x9C: // SBC A,H
			alu_.subc(af_.hi, hl_.hi);
			break;
		case 0x9D: // SBC A,L
			alu_.subc(af_.hi, hl_.lo);
			break;
		case 0x9E: // SBC A,(HL)
			alu_.subc(af_.hi, mmu_->read(hl_.val));
			break;
		case 0xDE: // SBC A,n
			alu_.subc(af_.hi, load8Imm());
			break;

		/* Logical Operations */
		case 0xA7: // AND A,A
			alu_.anda(af_.hi, af_.hi);
			break;
		case 0xA0: // AND A,B
			alu_.anda(af_.hi, bc_.hi);
			break;
		case 0xA1: // AND A,C
			alu_.anda(af_.hi, bc_.lo);
			break;
		case 0xA2: // AND A,D
			alu_.anda(af_.hi, de_.hi);
			break;
		case 0xA3: // AND A,E
			alu_.anda(af_.hi, de_.lo);
			break;
		case 0xA4: // AND A,H
			alu_.anda(af_.hi, hl_.hi);
			break;
		case 0xA5: // AND A,L
			alu_.anda(af_.hi, hl_.lo);
			break;
		case 0xA6: // AND A,(HL)
			alu_.anda(af_.hi, mmu_->read(hl_.val));
			break;
		case 0xE6: // AND A,n
			alu_.anda(af_.hi, load8Imm());
			break;

		case 0xB7: // OR A,A
			alu_.ora(af_.hi, af_.hi);
			break;
		case 0xB0: // OR A,B
			alu_.ora(af_.hi, bc_.hi);
			break;
		case 0xB1: // OR A,C
			alu_.ora(af_.hi, bc_.lo);
			break;
		case 0xB2: // OR A,D
			alu_.ora(af_.hi, de_.hi);
			break;
		case 0xB3: // OR A,E
			alu_.ora(af_.hi, de_.lo);
			break;
		case 0xB4: // OR A,H
			alu_.ora(af_.hi, hl_.hi);
			break;
		case 0xB5: // OR A,L
			alu_.ora(af_.hi, hl_.lo);
			break;
		case 0xB6: // OR A,(HL)
			alu_.ora(af_.hi, mmu_->read(hl_.val));
			break;
		case 0xF6: // OR A,n
			alu_.ora(af_.hi, load8Imm());
			break;

		case 0xAF: // XOR A,A
			alu_.xora(af_.hi, af_.hi);
			break;
		case 0xA8: // XOR A,B
			alu_.xora(af_.hi, bc_.hi);
			break;
		case 0xA9: // XOR A,C
			alu_.xora(af_.hi, bc_.lo);
			break;
		case 0xAA: // XOR A,D
			alu_.xora(af_.hi, de_.hi);
			break;
		case 0xAB: // XOR A,E
			alu_.xora(af_.hi, de_.lo);
			break;
		case 0xAC: // XOR A,H
			alu_.xora(af_.hi, hl_.hi);
			break;
		case 0xAD: // XOR A,L
			alu_.xora(af_.hi, hl_.lo);
			break;
		case 0xAE: // XOR A,(HL)
			alu_.xora(af_.hi, mmu_->read(hl_.val));
			break;
		case 0xEE: // OR A,n
			alu_.xora(af_.hi, load8Imm());
			break;

		/* Comparison */
		case 0xBF: // CP A,A
			alu_.compare(af_.hi, af_.hi);
			break;
		case 0xB8: // CP A,B
			alu_.compare(af_.hi, bc_.hi);
			break;
		case 0xB9: // CP A,C
			alu_.compare(af_.hi, bc_.lo);
			break;
		case 0xBA: // CP A,D
			alu_.compare(af_.hi, de_.hi);
			break;
		case 0xBB: // CP A,E
			alu_.compare(af_.hi, de_.lo);
			break;
		case 0xBC: // CP A,H
			alu_.compare(af_.hi, hl_.hi);
			break;
		case 0xBD: // CP A,L
			alu_.compare(af_.hi, hl_.lo);
			break;
		case 0xBE: // CP A,(HL)
			alu_.compare(af_.hi, mmu_->read(hl_.val));
			break;
		case 0xFE: // CP A,n
			alu_.compare(af_.hi, load8Imm());
			break;

		/* Rotate A*/

		case 0x07: // RLCA
			af_.hi = rlca(af_.hi, af_.lo);
			break;
		case 0x17: // RLA
			af_.hi = rla(af_.hi, af_.lo);
			break;
		case 0x0F: // RRCA
			af_.hi = rrca(af_.hi, af_.lo);
			break;
		case 0x1F: // RRA
			af_.hi = rra(af_.hi, af_.lo);
			break;

		default:
			std::cout << "Unimplemented Instruction: $" << std::hex << (int)opcode << std::endl;
			throw std::runtime_error("");
			break;
		}

		if (debug_mode_)
		{
			printDisassembly(opcode, old_pc, OpcodePage::PAGE1);
		}
	}

	void CPU::decode2(uint8_t opcode)
	{
		static uint16_t old_pc;

		// store current program counter location so it can be reused for disassembly output
		old_pc = pc_.val;

		uint8_t tmp;

		switch (opcode)
		{
		/* SWAP */
		case 0x37: // SWAP A
			af_.hi = swap(af_.hi);
			break;
		case 0x30: // SWAP B
			bc_.hi = swap(bc_.hi);
			break;
		case 0x31: // SWAP C
			bc_.lo = swap(bc_.lo);
			break;
		case 0x32: // SWAP D
			de_.hi = swap(de_.hi);
			break;
		case 0x33: // SWAP E
			de_.lo = swap(de_.lo);
			break;
		case 0x34: // SWAP H
			hl_.hi = swap(hl_.hi);
			break;
		case 0x35: // SWAP L
			hl_.lo = swap(hl_.lo);
			break;
		case 0x36: // SWAP (HL)
			mmu_->write(swap(mmu_->read(hl_.val)), hl_.val);
			break;

		/* Rotate */

		case 0x00: // RLC B
			bc_.hi = rotateLeft(bc_.hi, 1, af_.lo);
			break;
		case 0x01: // RLC C
			bc_.lo = rotateLeft(bc_.lo, 1, af_.lo);
			break;
		case 0x02: // RLC D
			de_.hi = rotateLeft(de_.hi, 1, af_.lo);
			break;
		case 0x03: // RLC E
			de_.lo = rotateLeft(de_.lo, 1, af_.lo);
			break;
		case 0x04: // RLC H
			hl_.hi = rotateLeft(hl_.hi, 1, af_.lo);
			break;
		case 0x05: // RLC L
			hl_.lo = rotateLeft(hl_.lo, 1, af_.lo);
			break;
		case 0x06: // RLC (HL)
			mmu_->write(rotateLeft(mmu_->read(hl_.val), 1, af_.lo), hl_.val);
			break;
		case 0x07: // RLC A
			af_.hi = rotateLeft(af_.hi, 1, af_.lo);
			break;

		case 0x08: // RRC B
			bc_.hi = rotateRight(bc_.hi, 1, af_.lo);
			break;
		case 0x09: // RRC C
			bc_.lo = rotateRight(bc_.lo, 1, af_.lo);
			break;
		case 0x0A: // RRC D
			de_.hi = rotateRight(de_.hi, 1, af_.lo);
			break;
		case 0x0B: // RRC E
			de_.lo = rotateRight(de_.lo, 1, af_.lo);
			break;
		case 0x0C: // RRC H
			hl_.hi = rotateRight(hl_.hi, 1, af_.lo);
			break;
		case 0x0D: // RRC L
			hl_.lo = rotateRight(hl_.lo, 1, af_.lo);
			break;
		case 0x0E: // RRC (HL)
			mmu_->write(rotateRight(mmu_->read(hl_.val), 1, af_.lo), hl_.val);
			break;
		case 0x0F: // RRC A
			af_.hi = rotateRight(af_.hi, 1, af_.lo);
			break;

			//.........................

		case 0x10: // RL B
			bc_.hi = rotateLeftCarry(bc_.hi, 1, af_.lo);
			break;
		case 0x11: // RL C
			bc_.lo = rotateLeftCarry(bc_.lo, 1, af_.lo);
			break;
		case 0x12: // RL D
			de_.hi = rotateLeftCarry(de_.hi, 1, af_.lo);
			break;
		case 0x13: // RL E
			de_.lo = rotateLeftCarry(de_.lo, 1, af_.lo);
			break;
		case 0x14: // RL H
			hl_.hi = rotateLeftCarry(hl_.hi, 1, af_.lo);
			break;
		case 0x15: // RL L
			hl_.lo = rotateLeftCarry(hl_.lo, 1, af_.lo);
			break;
		case 0x16: // RL (HL)
			mmu_->write(rotateLeftCarry(mmu_->read(hl_.val), 1, af_.lo), hl_.val);
			break;
		case 0x17: // RL A
			af_.hi = rotateLeftCarry(af_.hi, 1, af_.lo);
			break;

		case 0x18: // RR B
			bc_.hi = rotateRightCarry(bc_.hi, 1, af_.lo);
			break;
		case 0x19: // RR C
			bc_.lo = rotateRightCarry(bc_.lo, 1, af_.lo);
			break;
		case 0x1A: // RR D
			de_.hi = rotateRightCarry(de_.hi, 1, af_.lo);
			break;
		case 0x1B: // RR E
			de_.lo = rotateRightCarry(de_.lo, 1, af_.lo);
			break;
		case 0x1C: // RR H
			hl_.hi = rotateRightCarry(hl_.hi, 1, af_.lo);
			break;
		case 0x1D: // RR L
			hl_.lo = rotateRightCarry(hl_.lo, 1, af_.lo);
			break;
		case 0x1E: // RR (HL)
			mmu_->write(rotateRightCarry(mmu_->read(hl_.val), 1, af_.lo), hl_.val);
			break;
		case 0x1F: // RR A
			af_.hi = rotateRightCarry(af_.hi, 1, af_.lo);
			break;

		/* Shift */

		case 0x20: // SLA B
			bc_.hi = shiftLeft(bc_.hi, 1, af_.lo);
			break;
		case 0x21: // SLA C
			bc_.lo = shiftLeft(bc_.lo, 1, af_.lo);
			break;
		case 0x22: // SLA D
			de_.hi = shiftLeft(de_.hi, 1, af_.lo);
			break;
		case 0x23: // SLA E
			de_.lo = shiftLeft(de_.lo, 1, af_.lo);
			break;
		case 0x24: // SLA H
			hl_.hi = shiftLeft(hl_.hi, 1, af_.lo);
			break;
		case 0x25: // SLA L
			hl_.lo = shiftLeft(hl_.lo, 1, af_.lo);
			break;
		case 0x26: // SLA (HL)
			mmu_->write(shiftLeft(mmu_->read(hl_.val), 1, af_.lo), hl_.val);
			break;
		case 0x27: // SLA A
			af_.hi = shiftLeft(af_.hi, 1, af_.lo);
			break;

		case 0x28: // SRA B
			bc_.hi = shiftRightA(bc_.hi, 1, af_.lo);
			break;
		case 0x29: // SRA C
			bc_.lo = shiftRightA(bc_.lo, 1, af_.lo);
			break;
		case 0x2A: // SRA D
			de_.hi = shiftRightA(de_.hi, 1, af_.lo);
			break;
		case 0x2B: // SRA E
			de_.lo = shiftRightA(de_.lo, 1, af_.lo);
			break;
		case 0x2C: // SRA H
			hl_.hi = shiftRightA(hl_.hi, 1, af_.lo);
			break;
		case 0x2D: // SRA L
			hl_.lo = shiftRightA(hl_.lo, 1, af_.lo);
			break;
		case 0x2E: // SRA (HL)
			mmu_->write(shiftRightA(mmu_->read(hl_.val), 1, af_.lo), hl_.val);
			break;
		case 0x2F: // SRA A
			af_.hi = shiftRightA(af_.hi, 1, af_.lo);
			break;

		case 0x38: // SRL B
			bc_.hi = shiftRightL(bc_.hi, 1, af_.lo);
			break;
		case 0x39: // SRL C
			bc_.lo = shiftRightL(bc_.lo, 1, af_.lo);
			break;
		case 0x3A: // SRL D
			de_.hi = shiftRightL(de_.hi, 1, af_.lo);
			break;
		case 0x3B: // SRL E
			de_.lo = shiftRightL(de_.lo, 1, af_.lo);
			break;
		case 0x3C: // SRL H
			hl_.hi = shiftRightL(hl_.hi, 1, af_.lo);
			break;
		case 0x3D: // SRL L
			hl_.lo = shiftRightL(hl_.lo, 1, af_.lo);
			break;
		case 0x3E: // SRL (HL)
			mmu_->write(shiftRightL(mmu_->read(hl_.val), 1, af_.lo), hl_.val);
			break;
		case 0x3F: // SRL A
			af_.hi = shiftRightL(af_.hi, 1, af_.lo);
			break;

		/* Bit */

		// bit 0
		case 0x40: // BIT 0,B
			bit(bc_.hi, 0);
			break;
		case 0x41: // BIT 0,C
			bit(bc_.lo, 0);
			break;
		case 0x42: // BIT 0,D
			bit(de_.hi, 0);
			break;
		case 0x43: // BIT 0,E
			bit(de_.lo, 0);
			break;
		case 0x44: // BIT 0,H
			bit(hl_.hi, 0);
			break;
		case 0x45: // BIT 0,L
			bit(hl_.lo, 0);
			break;
		case 0x46: // BIT 0,(HL)
			bit(mmu_->read(hl_.val), 0);
			break;
		case 0x47: // BIT 0,A
			bit(af_.hi, 0);
			break;
		// bit 1
		case 0x48: // BIT 1,B
			bit(bc_.hi, 1);
			break;
		case 0x49: // BIT 1,C
			bit(bc_.lo, 1);
			break;
		case 0x4A: // BIT 1,D
			bit(de_.hi, 1);
			break;
		case 0x4B: // BIT 1,E
			bit(de_.lo, 1);
			break;
		case 0x4C: // BIT 1,H
			bit(hl_.hi, 1);
			break;
		case 0x4D: // BIT 1,L
			bit(hl_.lo, 1);
			break;
		case 0x4E: // BIT 1,(HL)
			bit(mmu_->read(hl_.val), 1);
			break;
		case 0x4F: // BIT 1,A
			bit(af_.hi, 1);
			break;

		// bit 2
		case 0x50: // BIT 2,B
			bit(bc_.hi, 2);
			break;
		case 0x51: // BIT 2,C
			bit(bc_.lo, 2);
			break;
		case 0x52: // BIT 2,D
			bit(de_.hi, 2);
			break;
		case 0x53: // BIT 2,E
			bit(de_.lo, 2);
			break;
		case 0x54: // BIT 2,H
			bit(hl_.hi, 2);
			break;
		case 0x55: // BIT 2,L
			bit(hl_.lo, 2);
			break;
		case 0x56: // BIT 2,(HL)
			bit(mmu_->read(hl_.val), 2);
			break;
		case 0x57: // BIT 2,A
			bit(af_.hi, 2);
			break;

		// bit 3
		case 0x58: // BIT 3,B
			bit(bc_.hi, 3);
			break;
		case 0x59: // BIT 3,C
			bit(bc_.lo, 3);
			break;
		case 0x5A: // BIT 3,D
			bit(de_.hi, 3);
			break;
		case 0x5B: // BIT 3,E
			bit(de_.lo, 3);
			break;
		case 0x5C: // BIT 3,H
			bit(hl_.hi, 3);
			break;
		case 0x5D: // BIT 3,L
			bit(hl_.lo, 3);
			break;
		case 0x5E: // BIT 3,(HL)
			bit(mmu_->read(hl_.val), 3);
			break;
		case 0x5F: // BIT 3,A
			bit(af_.hi, 3);
			break;

		// bit 4
		case 0x60: // BIT 4,B
			bit(bc_.hi, 4);
			break;
		case 0x61: // BIT 4,C
			bit(bc_.lo, 4);
			break;
		case 0x62: // BIT 4,D
			bit(de_.hi, 4);
			break;
		case 0x63: // BIT 4,E
			bit(de_.lo, 4);
			break;
		case 0x64: // BIT 4,H
			bit(hl_.hi, 4);
			break;
		case 0x65: // BIT 4,L
			bit(hl_.lo, 4);
			break;
		case 0x66: // BIT 4,(HL)
			bit(mmu_->read(hl_.val), 4);
			break;
		case 0x67: // BIT 4,A
			bit(af_.hi, 4);
			break;

		// bit 5
		case 0x68: // BIT 5,B
			bit(bc_.hi, 5);
			break;
		case 0x69: // BIT 5,C
			bit(bc_.lo, 5);
			break;
		case 0x6A: // BIT 5,D
			bit(de_.hi, 5);
			break;
		case 0x6B: // BIT 5,E
			bit(de_.lo, 5);
			break;
		case 0x6C: // BIT 5,H
			bit(hl_.hi, 5);
			break;
		case 0x6D: // BIT 5,L
			bit(hl_.lo, 5);
			break;
		case 0x6E: // BIT 5,(HL)
			bit(mmu_->read(hl_.val), 5);
			break;
		case 0x6F: // BIT 5,A
			bit(af_.hi, 5);
			break;

		// bit 6
		case 0x70: // BIT 6,B
			bit(bc_.hi, 6);
			break;
		case 0x71: // BIT 6,C
			bit(bc_.lo, 6);
			break;
		case 0x72: // BIT 6,D
			bit(de_.hi, 6);
			break;
		case 0x73: // BIT 6,E
			bit(de_.lo, 6);
			break;
		case 0x74: // BIT 6,H
			bit(hl_.hi, 6);
			break;
		case 0x75: // BIT 6,L
			bit(hl_.lo, 6);
			break;
		case 0x76: // BIT 6,(HL)
			bit(mmu_->read(hl_.val), 6);
			break;
		case 0x77: // BIT 6,A
			bit(af_.hi, 6);
			break;
		// bit 7
		case 0x78: // BIT 7,B
			bit(bc_.hi, 7);
			break;
		case 0x79: // BIT 7,C
			bit(bc_.lo, 7);
			break;
		case 0x7A: // BIT 7,D
			bit(de_.hi, 7);
			break;
		case 0x7B: // BIT 7,E
			bit(de_.lo, 7);
			break;
		case 0x7C: // BIT 7,H
			bit(hl_.hi, 7);
			break;
		case 0x7D: // BIT 7,L
			bit(hl_.lo, 7);
			break;
		case 0x7E: // BIT 7,(HL)
			bit(mmu_->read(hl_.val), 7);
			break;
		case 0x7F: // BIT 7,A
			bit(af_.hi, 7);
			break;

		/* Reset */
		case 0x80: // RES 0,B
			CLR_BIT(bc_.hi, 0);
			break;
		case 0x81: // RES 0,C
			CLR_BIT(bc_.lo, 0);
			break;
		case 0x82: // RES 0,D
			CLR_BIT(de_.hi, 0);
			break;
		case 0x83: // RES 0,E
			CLR_BIT(de_.lo, 0);
			break;
		case 0x84: // RES 0,H
			CLR_BIT(hl_.hi, 0);
			break;
		case 0x85: // RES 0,L
			CLR_BIT(hl_.lo, 0);
			break;
		case 0x86: // RES 0,(HL)
			tmp = mmu_->read(hl_.val);
			CLR_BIT(tmp, 0);
			mmu_->write(tmp, hl_.val);
			break;
		case 0x87: // RES 0,A
			CLR_BIT(af_.hi, 0);
			break;
		case 0x88: // RES 1,B
			CLR_BIT(bc_.hi, 1);
			break;
		case 0x89: // RES 1,C
			CLR_BIT(bc_.lo, 1);
			break;
		case 0x8A: // RES 1,D
			CLR_BIT(de_.hi, 1);
			break;
		case 0x8B: // RES 1,E
			CLR_BIT(de_.lo, 1);
			break;
		case 0x8C: // RES 1,H
			CLR_BIT(hl_.hi, 1);
			break;
		case 0x8D: // RES 1,L
			CLR_BIT(hl_.lo, 1);
			break;
		case 0x8E: // RES 1,(HL)
			tmp = mmu_->read(hl_.val);
			CLR_BIT(tmp, 1);
			mmu_->write(tmp, hl_.val);
			break;
		case 0x8F: // RES 1,A
			CLR_BIT(af_.hi, 1);
			break;

		case 0x90: // RES 2,B
			CLR_BIT(bc_.hi, 2);
			break;
		case 0x91: // RES 2,C
			CLR_BIT(bc_.lo, 2);
			break;
		case 0x92: // RES 2,D
			CLR_BIT(de_.hi, 2);
			break;
		case 0x93: // RES 2,E
			CLR_BIT(de_.lo, 2);
			break;
		case 0x94: // RES 2,H
			CLR_BIT(hl_.hi, 2);
			break;
		case 0x95: // RES 2,L
			CLR_BIT(hl_.lo, 2);
			break;
		case 0x96: // RES 2,(HL)
			tmp = mmu_->read(hl_.val);
			CLR_BIT(tmp, 2);
			mmu_->write(tmp, hl_.val);
			break;
		case 0x97: // RES 2,A
			CLR_BIT(af_.hi, 2);
			break;
		case 0x98: // RES 3,B
			CLR_BIT(bc_.hi, 3);
			break;
		case 0x99: // RES 3,C
			CLR_BIT(bc_.lo, 3);
			break;
		case 0x9A: // RES 3,D
			CLR_BIT(de_.hi, 3);
			break;
		case 0x9B: // RES 3,E
			CLR_BIT(de_.lo, 3);
			break;
		case 0x9C: // RES 3,H
			CLR_BIT(hl_.hi, 3);
			break;
		case 0x9D: // RES 3,L
			CLR_BIT(hl_.lo, 3);
			break;
		case 0x9E: // RES 3,(HL)
			tmp = mmu_->read(hl_.val);
			CLR_BIT(tmp, 3);
			mmu_->write(tmp, hl_.val);
			break;
		case 0x9F: // RES 3,A
			CLR_BIT(af_.hi, 3);
			break;

		case 0xA0: // RES 4,B
			CLR_BIT(bc_.hi, 4);
			break;
		case 0xA1: // RES 4,C
			CLR_BIT(bc_.lo, 4);
			break;
		case 0xA2: // RES 4,D
			CLR_BIT(de_.hi, 4);
			break;
		case 0xA3: // RES 4,E
			CLR_BIT(de_.lo, 4);
			break;
		case 0xA4: // RES 4,H
			CLR_BIT(hl_.hi, 4);
			break;
		case 0xA5: // RES 4,L
			CLR_BIT(hl_.lo, 4);
			break;
		case 0xA6: // RES 4,(HL)
			tmp = mmu_->read(hl_.val);
			CLR_BIT(tmp, 4);
			mmu_->write(tmp, hl_.val);
			break;
		case 0xA7: // RES 4,A
			CLR_BIT(af_.hi, 4);
			break;
		case 0xA8: // RES 5,B
			CLR_BIT(bc_.hi, 5);
			break;
		case 0xA9: // RES 5,C
			CLR_BIT(bc_.lo, 5);
			break;
		case 0xAA: // RES 5,D
			CLR_BIT(de_.hi, 5);
			break;
		case 0xAB: // RES 5,E
			CLR_BIT(de_.lo, 5);
			break;
		case 0xAC: // RES 5,H
			CLR_BIT(hl_.hi, 5);
			break;
		case 0xAD: // RES 5,L
			CLR_BIT(hl_.lo, 5);
			break;
		case 0xAE: // RES 5,(HL)
			tmp = mmu_->read(hl_.val);
			CLR_BIT(tmp, 5);
			mmu_->write(tmp, hl_.val);
			break;
		case 0xAF: // RES 5,A
			CLR_BIT(af_.hi, 5);
			break;

		case 0xB0: // RES 6,B
			CLR_BIT(bc_.hi, 6);
			break;
		case 0xB1: // RES 6,C
			CLR_BIT(bc_.lo, 6);
			break;
		case 0xB2: // RES 6,D
			CLR_BIT(de_.hi, 6);
			break;
		case 0xB3: // RES 6,E
			CLR_BIT(de_.lo, 6);
			break;
		case 0xB4: // RES 6,H
			CLR_BIT(hl_.hi, 6);
			break;
		case 0xB5: // RES 6,L
			CLR_BIT(hl_.lo, 6);
			break;
		case 0xB6: // RES 6,(HL)
			tmp = mmu_->read(hl_.val);
			CLR_BIT(tmp, 6);
			mmu_->write(tmp, hl_.val);
			break;
		case 0xB7: // RES 6,A
			CLR_BIT(af_.hi, 6);
			break;
		case 0xB8: // RES 7,B
			CLR_BIT(bc_.hi, 7);
			break;
		case 0xB9: // RES 7,C
			CLR_BIT(bc_.lo, 7);
			break;
		case 0xBA: // RES 7,D
			CLR_BIT(de_.hi, 7);
			break;
		case 0xBB: // RES 7,E
			CLR_BIT(de_.lo, 7);
			break;
		case 0xBC: // RES 7,H
			CLR_BIT(hl_.hi, 7);
			break;
		case 0xBD: // RES 7,L
			CLR_BIT(hl_.lo, 7);
			break;
		case 0xBE: // RES 7,(HL)
			tmp = mmu_->read(hl_.val);
			CLR_BIT(tmp, 7);
			mmu_->write(tmp, hl_.val);
			break;
		case 0xBF: // RES 7,A
			CLR_BIT(af_.hi, 7);
			break;

		/* Set */

		case 0xC0: // SET 0,B
			SET_BIT(bc_.hi, 0);
			break;
		case 0xC1: // SET 0,C
			SET_BIT(bc_.lo, 0);
			break;
		case 0xC2: // SET 0,D
			SET_BIT(de_.hi, 0);
			break;
		case 0xC3: // SET 0,E
			SET_BIT(de_.lo, 0);
			break;
		case 0xC4: // SET 0,H
			SET_BIT(hl_.hi, 0);
			break;
		case 0xC5: // SET 0,L
			SET_BIT(hl_.lo, 0);
			break;
		case 0xC6: // SET 0,(HL)
			tmp = mmu_->read(hl_.val);
			SET_BIT(tmp, 0);
			mmu_->write(tmp, hl_.val);
			break;
		case 0xC7: // SET 0,A
			SET_BIT(af_.hi, 0);
			break;
		case 0xC8: // SET 1,B
			SET_BIT(bc_.hi, 1);
			break;
		case 0xC9: // SET 1,C
			SET_BIT(bc_.lo, 1);
			break;
		case 0xCA: // SET 1,D
			SET_BIT(de_.hi, 1);
			break;
		case 0xCB: // SET 1,E
			SET_BIT(de_.lo, 1);
			break;
		case 0xCC: // SET 1,H
			SET_BIT(hl_.hi, 1);
			break;
		case 0xCD: // SET 1,L
			SET_BIT(hl_.lo, 1);
			break;
		case 0xCE: // SET 1,(HL)
			tmp = mmu_->read(hl_.val);
			SET_BIT(tmp, 1);
			mmu_->write(tmp, hl_.val);
			break;
		case 0xCF: // SET 1,A
			SET_BIT(af_.hi, 1);
			break;

		case 0xD0: // SET 2,B
			SET_BIT(bc_.hi, 2);
			break;
		case 0xD1: // SET 2,C
			SET_BIT(bc_.lo, 2);
			break;
		case 0xD2: // SET 2,D
			SET_BIT(de_.hi, 2);
			break;
		case 0xD3: // SET 2,E
			SET_BIT(de_.lo, 2);
			break;
		case 0xD4: // SET 2,H
			SET_BIT(hl_.hi, 2);
			break;
		case 0xD5: // SET 2,L
			SET_BIT(hl_.lo, 2);
			break;
		case 0xD6: // SET 2,(HL)
			tmp = mmu_->read(hl_.val);
			SET_BIT(tmp, 2);
			mmu_->write(tmp, hl_.val);
			break;
		case 0xD7: // SET 2,A
			SET_BIT(af_.hi, 2);
			break;
		case 0xD8: // SET 3,B
			SET_BIT(bc_.hi, 3);
			break;
		case 0xD9: // SET 3,C
			SET_BIT(bc_.lo, 3);
			break;
		case 0xDA: // SET 3,D
			SET_BIT(de_.hi, 3);
			break;
		case 0xDB: // SET 3,E
			SET_BIT(de_.lo, 3);
			break;
		case 0xDC: // SET 3,H
			SET_BIT(hl_.hi, 3);
			break;
		case 0xDD: // SET 3,L
			SET_BIT(hl_.lo, 3);
			break;
		case 0xDE: // SET 3,(HL)
			tmp = mmu_->read(hl_.val);
			SET_BIT(tmp, 3);
			mmu_->write(tmp, hl_.val);
			break;
		case 0xDF: // SET 3,A
			SET_BIT(af_.hi, 3);
			break;

		case 0xE0: // SET 4,B
			SET_BIT(bc_.hi, 4);
			break;
		case 0xE1: // SET 4,C
			SET_BIT(bc_.lo, 4);
			break;
		case 0xE2: // SET 4,D
			SET_BIT(de_.hi, 4);
			break;
		case 0xE3: // SET 4,E
			SET_BIT(de_.lo, 4);
			break;
		case 0xE4: // SET 4,H
			SET_BIT(hl_.hi, 4);
			break;
		case 0xE5: // SET 4,L
			SET_BIT(hl_.lo, 4);
			break;
		case 0xE6: // SET 4,(HL)
			tmp = mmu_->read(hl_.val);
			SET_BIT(tmp, 4);
			mmu_->write(tmp, hl_.val);
			break;
		case 0xE7: // SET 4,A
			SET_BIT(af_.hi, 4);
			break;
		case 0xE8: // SET 5,B
			SET_BIT(bc_.hi, 5);
			break;
		case 0xE9: // SET 5,C
			SET_BIT(bc_.lo, 5);
			break;
		case 0xEA: // SET 5,D
			SET_BIT(de_.hi, 5);
			break;
		case 0xEB: // SET 5,E
			SET_BIT(de_.lo, 5);
			break;
		case 0xEC: // SET 5,H
			SET_BIT(hl_.hi, 5);
			break;
		case 0xED: // SET 5,L
			SET_BIT(hl_.lo, 5);
			break;
		case 0xEE: // SET 5,(HL)
			tmp = mmu_->read(hl_.val);
			SET_BIT(tmp, 5);
			mmu_->write(tmp, hl_.val);
			break;
		case 0xEF: // SET 5,A
			SET_BIT(af_.hi, 5);
			break;

		case 0xF0: // SET 6,B
			SET_BIT(bc_.hi, 6);
			break;
		case 0xF1: // SET 6,C
			SET_BIT(bc_.lo, 6);
			break;
		case 0xF2: // SET 6,D
			SET_BIT(de_.hi, 6);
			break;
		case 0xF3: // SET 6,E
			SET_BIT(de_.lo, 6);
			break;
		case 0xF4: // SET 6,H
			SET_BIT(hl_.hi, 6);
			break;
		case 0xF5: // SET 6,L
			SET_BIT(hl_.lo, 6);
			break;
		case 0xF6: // SET 6,(HL)
			tmp = mmu_->read(hl_.val);
			SET_BIT(tmp, 6);
			mmu_->write(tmp, hl_.val);
			break;
		case 0xF7: // SET 6,A
			SET_BIT(af_.hi, 6);
			break;
		case 0xF8: // SET 7,B
			SET_BIT(bc_.hi, 7);
			break;
		case 0xF9: // SET 7,C
			SET_BIT(bc_.lo, 7);
			break;
		case 0xFA: // SET 7,D
			SET_BIT(de_.hi, 7);
			break;
		case 0xFB: // SET 7,E
			SET_BIT(de_.lo, 7);
			break;
		case 0xFC: // SET 7,H
			SET_BIT(hl_.hi, 7);
			break;
		case 0xFD: // SET 7,L
			SET_BIT(hl_.lo, 7);
			break;
		case 0xFE: // SET 7,(HL)
			tmp = mmu_->read(hl_.val);
			SET_BIT(tmp, 7);
			mmu_->write(tmp, hl_.val);
			break;
		case 0xFF: // SET 7,A
			SET_BIT(af_.hi, 7);
			break;

		default:
			std::cout << "Unimplemented Instruction: " << std::hex << opcode << std::endl;
			throw std::runtime_error("");
			break;
		}

		if (debug_mode_)
		{
			printDisassembly(opcode, old_pc, OpcodePage::PAGE2);
		}
	}

	void CPU::checkInterrupts()
	{
		// when EI or DI is used to change the IME the change takes effect after the next instruction is executed

		if (interrupt_master_disable_pending_ >= 0)
		{
			interrupt_master_disable_pending_++;
			if (interrupt_master_disable_pending_ == 2)
			{
				interrupt_master_enable_ = false;
				interrupt_master_disable_pending_ = -1;
			}
		}

		if (interrupt_master_enable_pending_ >= 0)
		{
			interrupt_master_enable_pending_++;
			if (interrupt_master_enable_pending_ == 2)
			{
				interrupt_master_enable_ = true;
				interrupt_master_enable_pending_ = -1;
			}
		}

		if (interrupt_master_enable_)
		{
			// mask off disabled interrupts
			uint8_t pending_interrupts = interrupt_flags_ & interrupt_enable_;

			if (IS_SET(pending_interrupts, InterruptMask::JOYPAD))
				interrupt(InterruptVector::JOYPAD, InterruptMask::JOYPAD);

			if (IS_SET(pending_interrupts, InterruptMask::SERIAL_TRANSFER_COMPLETE))
				interrupt(InterruptVector::SERIAL_TRANSFER_COMPLETE, InterruptMask::SERIAL_TRANSFER_COMPLETE);
			
			if (IS_SET(pending_interrupts, InterruptMask::TIME_OVERFLOW))
				interrupt(InterruptVector::TIME_OVERFLOW, InterruptMask::TIME_OVERFLOW);
			
			if (IS_SET(pending_interrupts, InterruptMask::LCDC_STAT))
				interrupt(InterruptVector::LCDC_STAT, InterruptMask::LCDC_STAT);
			
			if (IS_SET(pending_interrupts, InterruptMask::VBLANK))
				interrupt(InterruptVector::VBLANK, InterruptMask::VBLANK);
		}
	}

	void CPU::checkPowerMode()
	{
		uint8_t pending_interrupts = interrupt_enable_ & interrupt_flags_;

		if (!stopped_)
		{
			if (pending_interrupts != 0)
			{
				halted_ = false;
			}
		}
		else
		{
			if (IS_SET(pending_interrupts, InterruptMask::JOYPAD))
			{
				stopped_ = false;
				halted_ = false;
			}
		}
	}

	void CPU::interrupt(InterruptVector vector, InterruptMask mask)
	{
		interrupt_master_enable_ = false;

		push(pc_.val);
		pc_.val = static_cast<uint16_t>(vector);

		CLR(interrupt_flags_, mask);
	}

	void CPU::printDisassembly(uint8_t opcode, uint16_t userdata_addr, OpcodePage page)
	{
		const int spaces_before_registers = 13;
		char str[32];

		OpcodeInfo opcodeinfo = getOpcodeInfo(opcode, page);

		if (opcodeinfo.userdata == OperandType::NONE)
		{
			std::sprintf(str, opcodeinfo.disassembly);
		}
		else
		{
			if (opcodeinfo.userdata == OperandType::IMM8)
			{
				uint8_t userdata = mmu_->read(userdata_addr);
				std::sprintf(str, opcodeinfo.disassembly, userdata);
			}
			else // OperandType::IMM16
			{
				uint8_t lo = mmu_->read(userdata_addr);
				uint8_t hi = mmu_->read(userdata_addr + 1);

				std::sprintf(str, opcodeinfo.disassembly, WORD(hi, lo));
			}
		}

		std::string padding(spaces_before_registers - std::strlen(str), ' ');

		// print debug info
		std::printf("%04X: %s%s| PC: %04X, A: %02X, BC: %02X%02X, DE: %02X%02X, HL: %02X%02X | SP: %04X -> %04X | F: %02X | IF: %02X, IE: %02X\n",
			userdata_addr - 1,
			str,
			padding.c_str(),
			pc_.val,
			af_.hi,
			bc_.hi,
			bc_.lo,
			de_.hi,
			de_.lo,
			hl_.hi,
			hl_.lo,
			sp_.val,
			WORD(mmu_->read(sp_.val + 1), mmu_->read(sp_.val)),
			af_.lo,
			interrupt_flags_,
			interrupt_enable_
		);
	}

	uint8_t CPU::load8Imm()
	{
		return mmu_->read(pc_.val++);
	}

	uint16_t CPU::load16Imm()
	{
		uint8_t lo = load8Imm();
		uint8_t hi = load8Imm();

		return WORD(hi, lo);
	}

	void CPU::in(uint16_t addr)
	{
		// read from offset into IO registers
		af_.hi = mmu_->read(addr);
	}

	void CPU::out(uint16_t addr)
	{
		// write out to the IO registers given the offset
		mmu_->write(af_.hi, addr);
	}

	void CPU::inc(uint8_t& i)
	{
		bool half_carry = IS_HALF_CARRY(i, 1);

		i++;

		setFlag(CPU::Flags::Z, i == 0);
		setFlag(CPU::Flags::N, false);
		setFlag(CPU::Flags::H, half_carry);
	}

	void CPU::inc(uint16_t& i)
	{
		i++;
	}

	void CPU::dec(uint8_t& d)
	{
		bool half_carry = IS_HALF_BORROW(d, 1);

		d--;

		setFlag(CPU::Flags::Z, d == 0);
		setFlag(CPU::Flags::N, true);
		setFlag(CPU::Flags::H, half_carry);
	}

	void CPU::dec(uint16_t& d)
	{
		d--;
	}

	void CPU::inca(uint16_t addr)
	{
		uint8_t b = mmu_->read(addr);
		inc(b);
		mmu_->write(b, addr);
	}

	void CPU::deca(uint16_t addr)
	{
		uint8_t b = mmu_->read(addr);
		dec(b);
		mmu_->write(b, addr);
	}

	void CPU::push(uint16_t value)
	{
		uint8_t hi = (value & 0xFF00) >> 8;
		uint8_t lo = (value & 0x00FF);

		mmu_->write(hi, sp_.val - 1);
		mmu_->write(lo, sp_.val - 2);

		sp_.val -= 2;
	}

	uint16_t CPU::pop()
	{
		uint8_t lo = mmu_->read(sp_.val);
		uint8_t hi = mmu_->read(sp_.val + 1);

		sp_.val += 2;

		return WORD(hi, lo);
	}

	void CPU::jp(uint16_t addr)
	{
		pc_.val = addr;
	}

	void CPU::jr(int8_t r)
	{
		pc_.val += r;
	}

	void CPU::call(uint16_t addr)
	{
		push(pc_.val);
		pc_.val = addr;
	}

	void CPU::ret()
	{
		pc_.val = pop();
	}

	void CPU::reti()
	{
		ret();
		interrupt_master_enable_ = true;
	}

	uint8_t CPU::swap(uint8_t byte)
	{
		uint8_t hi = (byte & 0xF0) >> 4;
		uint8_t lo = byte & 0x0F;

		uint8_t newByte = (lo << 4) | hi;

		setFlag(CPU::Flags::Z, newByte == 0);
		setFlag(CPU::Flags::N, false);
		setFlag(CPU::Flags::H, false);
		setFlag(CPU::Flags::C, false);

		return newByte;
	}

	void CPU::daa()
	{
		/*
		bool n = IS_SET(af_.lo, CPU::Flags::N) != 0;
		bool h = IS_SET(af_.lo, CPU::Flags::H) != 0;
		bool c = IS_SET(af_.lo, CPU::Flags::C) != 0;

		if (n)
		{
			// was a form of subtraction instruction

			// offset values
			uint8_t offsets[] = {
				0x00, 0xFA, 0xA0, 0x9A
			};

			// get index
			uint8_t idx = (((uint8_t)c) << 1) | (uint8_t)h;
			// get offset
			uint8_t offset = offsets[idx];

			af_.hi += offset;

			setFlag(CPU::Flags::C, c);
		}
		else
		{
			// was a form of addition instruction

			// get hi and lo nybbles of A
			uint8_t hi = (af_.hi & 0xF0) >> 4;
			uint8_t lo = (af_.lo & 0x0F);

			uint8_t offset_hi = 0;
			uint8_t offset_lo = 0;

			if (!c)
			{
				if (hi >= 0x09)
					offset_hi = 6;

				if ((lo >= 0x0A) || h)
					offset_lo = 6;

				uint8_t offset = NYB_CAT(offset_hi, offset_lo);
				bool carry = IS_FULL_CARRY(af_.hi, offset);

				af_.hi += offset;

				setFlag(CPU::Flags::C, carry);
			}
			else
			{
				offset_hi = 6;

				if ((lo >= 0x0A) || h)
				{
					offset_lo = 6;
				}

				af_.hi += NYB_CAT(hi, lo);

				setFlag(CPU::Flags::C, true);
			}

		}


		setFlag(CPU::Flags::Z, af_.hi == 0);
		setFlag(CPU::Flags::H, false);
		*/

		bool n = IS_SET(af_.lo, CPU::Flags::N) != 0;
		bool h = IS_SET(af_.lo, CPU::Flags::H) != 0;
		bool c = IS_SET(af_.lo, CPU::Flags::C) != 0;

		int a = (int) af_.hi;

		if (!n)
		{
			if (c || (a > 0x99)) {
				a = (a + 0x60) & 0xFF;
				setFlag(Flags::C, true);
			}
			if (h || ((a & 0x0F) > 9)) {
				a = (a + 0x06) & 0xFF;
				setFlag(Flags::H, false);
			}
		}
		else if(c && h)
		{
			a = (a + 0x9A) & 0xFF;
			setFlag(Flags::H, false);
		}
		else if (c)
		{
			a = (a + 0xA0) & 0xFF;
		}
		else if (h)
		{
			a = (a + 0xFA) & 0xFF;
			setFlag(Flags::H, false);
		}

		setFlag(Flags::Z, a == 0);

	//	setFlag(Flags::H, false);
	//	setFlag(Flags::C, (a & 0x100) == 0x100);

		//a &= 0xFF;
	
	//	setFlag(Flags::Z, a == 0);

		af_.hi = (uint8_t)a;
	}

	uint16_t CPU::ldHLSPe()
	{
		int8_t e = (int8_t)load8Imm();
		int result = sp_.val + e;

		setFlag(Flags::C, ((sp_.val ^ e ^ (result & 0xFFFF)) & 0x100) == 0x100);
		setFlag(Flags::H, ((sp_.val ^ e ^ (result & 0xFFFF)) & 0x10) == 0x10);

		setFlag(ALU::Flags::Z, false);
		setFlag(ALU::Flags::N, false);

		return (uint16_t)result;
	}

	void CPU::bit(uint8_t val, uint8_t n)
	{
		uint8_t b = (val & BV(n)) >> n;

		setFlag(CPU::Flags::Z, b == 0);
		setFlag(CPU::Flags::H, true);
		setFlag(CPU::Flags::N, false);
	}

	void CPU::stop()
	{
		if (cgb_mode_)
		{
			// TODO: CGB support
		}
		else
		{
			// TODO: Remove the KEY1 check
			auto key1_reg = mmu_->read(memorymap::KEY1_REGISER);

			// check for preparing speed switch
			if (key1_reg & 0x01) return;

			stopped_ = true;
			halted_ = true;
			pc_.val++;
		}
	}

	void CPU::setFlag(uint8_t mask, bool set)
	{
		if (set)
		{
			SET(af_.lo, mask);
		}
		else
		{
			CLR(af_.lo, mask);
		}
	}

	void CPU::reset()
	{
		af_.val = 0;
		bc_.val = 0;
		de_.val = 0;
		hl_.val = 0;
		sp_.val = memorymap::HIGH_RAM_END;
		pc_.val = memorymap::PROGRAM_START;

		cycle_count_ = 0;
		halted_ = false;
		stopped_ = false;
		interrupt_master_enable_ = false;
		interrupt_master_enable_pending_ = -1;
		interrupt_master_disable_pending_ = -1;

		div_ = (Register*)mmu_->getptr(memorymap::DIVIDER_LO_REGISTER);

		// set normal speed mode of CGB
		mmu_->write((uint8_t)0x00, memorymap::KEY1_REGISER);
	}

	void CPU::setDebugMode(bool debug_mode)
	{
		debug_mode_ = debug_mode;
	}

    bool CPU::isHalted() const
    {
        return halted_;
    }

    const MMU& CPU::getMMU() const
    {
        return *mmu_.get();
    }
	MMU& CPU::getMMU()
	{
		return *mmu_.get();
	}

	const LCDController& CPU::getLCDController() const
	{
		return lcd_;
	}

	LCDController& CPU::getLCDController()
	{
		return lcd_;
	}

	CPU::Status CPU::getStatus() const
	{
		Status status;
		status.af      = af_;
		status.bc      = bc_;
		status.de      = de_;
		status.hl      = hl_;
		status.sp      = sp_;
		status.pc      = pc_;
		status.halt    = halted_;
		status.stopped = stopped_;

		return status;
	}

}
