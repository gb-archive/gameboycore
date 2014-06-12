

#ifndef GB_EMULATOR
#define GB_EMULATOR

#include "GBCore.h"

//typedef int (*InstructionImpl)(Core*);

struct Instruction{
	int (*impl)(struct Core*);
	uint8_t cycles;
};

void bootCore(struct Core* core);
uint8_t* getAddress(struct Core*, uint16_t);

int addAA(struct Core*);
int addAB(struct Core*);
int addAC(struct Core*);
int addAD(struct Core*);
int addAE(struct Core*);
int addAH(struct Core*);
int addAL(struct Core*);
int addAHL(struct Core*);
int addAByte(struct Core*);

int execute(struct Core*, uint8_t);

int transfer(uint8_t* to, uint8_t* from);

/* Core functions */

int NOP(struct Core*);

#endif // Z80_INSTRUCTION_SET
