
#ifndef INSTRUCTION_SET
#define INSTRUCTION_SET

#include "add.h"
#include "decrement.h"
#include "decrement_reg_pair.h"
#include "exchange.h"
#include "increment.h"
#include "increment_reg_pair.h"
#include "load.h"
#include "subtract.h"
#include "nop.h"
#include "double_add.h"
#include "stack.h"
#include "branch.h"
#include "shift.h"

#define REDIRECT 0xCB

struct Instruction{
	int (*impl)(struct Core*);
	uint8_t cycles;
};

//! First set of instructions
struct Instruction instructionSet1[256] = {
	{&NOP, 4} /*0x0*/,
	{&ldBCWord, 4} /*0x1*/,
	{&ldBCA, 4} /*0x2*/,
	{&incBC, 6} /*0x3*/,
	{&NOP, 4} /*0x4*/,
	{&decB, 4} /*0x5*/,
	{&ldBByte, 4} /*0x6*/,
	{&NOP, 4} /*0x7*/,
	{&NOP, 4} /*0x8*/,
	{&addHLBC, 4} /*0x9*/,
	{&ldABC, 4} /*0xa*/,
	{&NOP, 4} /*0xb*/,
	{&incC, 4} /*0xc*/,
	{&decC, 4} /*0xd*/,
	{&ldCByte, 4} /*0xe*/,
	{&NOP, 4} /*0xf*/,
	{&NOP, 4} /*0x10*/,
	{&ldDEWord, 4} /*0x11*/,
	{&ldDEA, 4} /*0x12*/,
	{&NOP, 4} /*0x13*/,
	{&incD, 4} /*0x14*/,
	{&decD, 4} /*0x15*/,
	{&ldDByte, 4} /*0x16*/,
	{&NOP, 4} /*0x17*/,
	{&NOP, 4} /*0x18*/,
	{&addHLDE, 4} /*0x19*/,
	{&ldADE, 4} /*0x1a*/,
	{&NOP, 4} /*0x1b*/,
	{&incE, 4} /*0x1c*/,
	{&NOP, 4} /*0x1d*/,
	{&ldEByte, 4} /*0x1e*/,
	{&NOP, 4} /*0x1f*/,
	{&NOP, 4} /*0x20*/,
	{&ldHLWord, 4} /*0x21*/,
	{&ldAddrHL, 4} /*0x22*/,
	{&NOP, 4} /*0x23*/,
	{&incH, 4} /*0x24*/,
	{&decH, 4} /*0x25*/,
	{&ldHByte, 4} /*0x26*/,
	{&NOP, 4} /*0x27*/,
	{&NOP, 4} /*0x28*/,
	{&addHLHL, 4} /*0x29*/,
	{&ldHLAddr, 4} /*0x2a*/,
	{&NOP, 4} /*0x2b*/,
	{&incL, 4} /*0x2c*/,
	{&decL, 4} /*0x2d*/,
	{&ldLByte, 4} /*0x2e*/,
	{&notA, 4} /*0x2f*/,
	{&NOP, 4} /*0x30*/,
	{&ldSPWord, 4} /*0x31*/,
	{&ldAddrA, 4} /*0x32*/,
	{&NOP, 4} /*0x33*/,
	{&incHL, 4} /*0x34*/,
	{&decHL, 4} /*0x35*/,
	{&ldHLByte, 4} /*0x36*/,
	{&SCF, 4} /*0x37*/,
	{&NOP, 4} /*0x38*/,
	{&addHLSP, 4} /*0x39*/,
	{&ldAWord, 4} /*0x3a*/,
	{&NOP, 4} /*0x3b*/,
	{&incA, 4} /*0x3c*/,
	{&decA, 4} /*0x3d*/,
	{&ldAByte, 4} /*0x3e*/,
	{&CCF, 4} /*0x3f*/,
	{&ldBB, 4} /*0x40*/,
	{&ldBC, 4} /*0x41*/,
	{&ldBD, 4} /*0x42*/,
	{&ldBE, 4} /*0x43*/,
	{&ldBH, 4} /*0x44*/,
	{&ldBL, 4} /*0x45*/,
	{&ldBHL, 4} /*0x46*/,
	{&ldBA, 4} /*0x47*/,
	{&ldCB, 4} /*0x48*/,
	{&ldCC, 4} /*0x49*/,
	{&ldCD, 4} /*0x4a*/,
	{&ldCE, 4} /*0x4b*/,
	{&ldCH, 4} /*0x4c*/,
	{&ldCL, 4} /*0x4d*/,
	{&ldCHL, 4} /*0x4e*/,
	{&ldCA, 4} /*0x4f*/,
	{&ldDB, 4} /*0x50*/,
	{&ldDC, 4} /*0x51*/,
	{&ldDD, 4} /*0x52*/,
	{&ldDE, 4} /*0x53*/,
	{&ldDH, 4} /*0x54*/,
	{&ldDL, 4} /*0x55*/,
	{&ldDHL, 4} /*0x56*/,
	{&ldDA, 4} /*0x57*/,
	{&ldEB, 4} /*0x58*/,
	{&ldEC, 4} /*0x59*/,
	{&ldED, 4} /*0x5a*/,
	{&ldEE, 4} /*0x5b*/,
	{&ldEH, 4} /*0x5c*/,
	{&ldEL, 4} /*0x5d*/,
	{&ldEHL, 4} /*0x5e*/,
	{&ldEA, 4} /*0x5f*/,
	{&ldHB, 4} /*0x60*/,
	{&ldHC, 4} /*0x61*/,
	{&ldHD, 4} /*0x62*/,
	{&ldHE, 4} /*0x63*/,
	{&ldHH, 4} /*0x64*/,
	{&ldHL, 4} /*0x65*/,
	{&ldHHL, 4} /*0x66*/,
	{&ldHA, 4} /*0x67*/,
	{&ldLB, 4} /*0x68*/,
	{&ldLC, 4} /*0x69*/,
	{&ldLD, 4} /*0x6a*/,
	{&ldLE, 4} /*0x6b*/,
	{&ldLH, 4} /*0x6c*/,
	{&ldLL, 4} /*0x6d*/,
	{&ldLHL, 4} /*0x6e*/,
	{&ldLA, 4} /*0x6f*/,
	{&ldHLB, 4} /*0x70*/,
	{&ldHLC, 4} /*0x71*/,
	{&ldHLD, 4} /*0x72*/,
	{&ldHLE, 4} /*0x73*/,
	{&ldHLH, 4} /*0x74*/,
	{&ldHLL, 4} /*0x75*/,
	{&NOP, 4} /*0x76*/,
	{&ldHLA, 4} /*0x77*/,
	{&ldAB, 4} /*0x78*/,
	{&ldAC, 4} /*0x79*/,
	{&ldAD, 4} /*0x7a*/,
	{&ldAE, 4} /*0x7b*/,
	{&ldAH, 4} /*0x7c*/,
	{&ldAL, 4} /*0x7d*/,
	{&ldAHL, 4} /*0x7e*/,
	{&ldAA, 4} /*0x7f*/,
	{&addAB, 4} /*0x80*/,
	{&addAC, 4} /*0x81*/,
	{&addAD, 4} /*0x82*/,
	{&addAE, 4} /*0x83*/,
	{&addAH, 4} /*0x84*/,
	{&addAL, 4} /*0x85*/,
	{&addAHL, 7} /*0x86*/,
	{&addAA, 4} /*0x87*/,
	{&addcAB, 4} /*0x88*/,
	{&addcAC, 4} /*0x89*/,
	{&addcAD, 4} /*0x8a*/,
	{&addcAE, 4} /*0x8b*/,
	{&addcAH, 4} /*0x8c*/,
	{&addcAL, 4} /*0x8d*/,
	{&addcAAddr, 4} /*0x8e*/,
	{&addcAA, 4} /*0x8f*/,
	{&subAB, 4} /*0x90*/,
	{&subAC, 4} /*0x91*/,
	{&subAD, 4} /*0x92*/,
	{&subAE, 4} /*0x93*/,
	{&subAH, 4} /*0x94*/,
	{&subAL, 4} /*0x95*/,
	{&subAHL, 7} /*0x96*/,
	{&subAA, 4} /*0x97*/,
	{&subcAB, 4} /*0x98*/,
	{&subcAC, 4} /*0x99*/,
	{&subcAD, 4} /*0x9a*/,
	{&subcAE, 4} /*0x9b*/,
	{&subcAH, 4} /*0x9c*/,
	{&subcAL, 4} /*0x9d*/,
	{&subcA_HL_, 4} /*0x9e*/,
	{&subcAA, 4} /*0x9f*/,
	{&NOP, 4} /*0xa0*/,
	{&NOP, 4} /*0xa1*/,
	{&NOP, 4} /*0xa2*/,
	{&NOP, 4} /*0xa3*/,
	{&NOP, 4} /*0xa4*/,
	{&NOP, 4} /*0xa5*/,
	{&NOP, 4} /*0xa6*/,
	{&NOP, 4} /*0xa7*/,
	{&NOP, 4} /*0xa8*/,
	{&NOP, 4} /*0xa9*/,
	{&NOP, 4} /*0xaa*/,
	{&NOP, 4} /*0xab*/,
	{&NOP, 4} /*0xac*/,
	{&NOP, 4} /*0xad*/,
	{&NOP, 4} /*0xae*/,
	{&NOP, 4} /*0xaf*/,
	{&NOP, 4} /*0xb0*/,
	{&NOP, 4} /*0xb1*/,
	{&NOP, 4} /*0xb2*/,
	{&NOP, 4} /*0xb3*/,
	{&NOP, 4} /*0xb4*/,
	{&NOP, 4} /*0xb5*/,
	{&NOP, 4} /*0xb6*/,
	{&NOP, 4} /*0xb7*/,
	{&NOP, 4} /*0xb8*/,
	{&NOP, 4} /*0xb9*/,
	{&NOP, 4} /*0xba*/,
	{&NOP, 4} /*0xbb*/,
	{&NOP, 4} /*0xbc*/,
	{&NOP, 4} /*0xbd*/,
	{&NOP, 4} /*0xbe*/,
	{&NOP, 4} /*0xbf*/,
	{&NOP, 4} /*0xc0*/,
	{&NOP, 4} /*0xc1*/,
	{&NOP, 4} /*0xc2*/,
	{&NOP, 4} /*0xc3*/,
	{&NOP, 4} /*0xc4*/,
	{&NOP, 4} /*0xc5*/,
	{&addAByte, 7} /*0xc6*/,
	{&NOP, 4} /*0xc7*/,
	{&NOP, 4} /*0xc8*/,
	{&NOP, 4} /*0xc9*/,
	{&NOP, 4} /*0xca*/,
	{&NOP, 4} /*0xcb*/,
	{&NOP, 4} /*0xcc*/,
	{&NOP, 4} /*0xcd*/,
	{&addcAByte, 4} /*0xce*/,
	{&NOP, 4} /*0xcf*/,
	{&NOP, 4} /*0xd0*/,
	{&NOP, 4} /*0xd1*/,
	{&NOP, 4} /*0xd2*/,
	{&NOP, 4} /*0xd3*/,
	{&NOP, 4} /*0xd4*/,
	{&NOP, 4} /*0xd5*/,
	{&NOP, 4} /*0xd6*/,
	{&NOP, 4} /*0xd7*/,
	{&NOP, 4} /*0xd8*/,
	{&NOP, 4} /*0xd9*/,
	{&NOP, 4} /*0xda*/,
	{&NOP, 4} /*0xdb*/,
	{&NOP, 4} /*0xdc*/,
	{&NOP, 4} /*0xdd*/,
	{&subcAByte, 4} /*0xde*/,
	{&NOP, 4} /*0xdf*/,
	{&NOP, 4} /*0xe0*/,
	{&NOP, 4} /*0xe1*/,
	{&NOP, 4} /*0xe2*/,
	{&exSPHL, 4} /*0xe3*/,
	{&NOP, 4} /*0xe4*/,
	{&NOP, 4} /*0xe5*/,
	{&NOP, 4} /*0xe6*/,
	{&NOP, 4} /*0xe7*/,
	{&NOP, 4} /*0xe8*/,
	{&NOP, 4} /*0xe9*/,
	{&NOP, 4} /*0xea*/,
	{&exDEHL, 4} /*0xeb*/,
	{&NOP, 4} /*0xec*/,
	{&NOP, 4} /*0xed*/,
	{&NOP, 4} /*0xee*/,
	{&NOP, 4} /*0xef*/,
	{&NOP, 4} /*0xf0*/,
	{&NOP, 4} /*0xf1*/,
	{&NOP, 4} /*0xf2*/,
	{&NOP, 4} /*0xf3*/,
	{&NOP, 4} /*0xf4*/,
	{&NOP, 4} /*0xf5*/,
	{&NOP, 4} /*0xf6*/,
	{&NOP, 4} /*0xf7*/,
	{&NOP, 4} /*0xf8*/,
	{&ldSPHL, 4} /*0xf9*/,
	{&NOP, 4} /*0xfa*/,
	{&NOP, 4} /*0xfb*/,
	{&NOP, 4} /*0xfc*/,
	{&NOP, 4} /*0xfd*/,
	{&NOP, 4} /*0xfe*/,
	{&NOP, 4} /*0xff*/
};

//! Instructions on the extended page. After $CB redirection
struct Instruction instructionSet2[256] = {
	{&NOP, 4} /*0x0*/,
	{&NOP, 4} /*0x1*/,
	{&NOP, 4} /*0x2*/,
	{&NOP, 4} /*0x3*/,
	{&NOP, 4} /*0x4*/,
	{&NOP, 4} /*0x5*/,
	{&NOP, 4} /*0x6*/,
	{&NOP, 4} /*0x7*/,
	{&NOP, 4} /*0x8*/,
	{&NOP, 4} /*0x9*/,
	{&NOP, 4} /*0xa*/,
	{&NOP, 4} /*0xb*/,
	{&NOP, 4} /*0xc*/,
	{&NOP, 4} /*0xd*/,
	{&NOP, 4} /*0xe*/,
	{&NOP, 4} /*0xf*/,
	{&NOP, 4} /*0x10*/,
	{&NOP, 4} /*0x11*/,
	{&NOP, 4} /*0x12*/,
	{&NOP, 4} /*0x13*/,
	{&NOP, 4} /*0x14*/,
	{&NOP, 4} /*0x15*/,
	{&NOP, 4} /*0x16*/,
	{&NOP, 4} /*0x17*/,
	{&NOP, 4} /*0x18*/,
	{&NOP, 4} /*0x19*/,
	{&NOP, 4} /*0x1a*/,
	{&NOP, 4} /*0x1b*/,
	{&NOP, 4} /*0x1c*/,
	{&NOP, 4} /*0x1d*/,
	{&NOP, 4} /*0x1e*/,
	{&NOP, 4} /*0x1f*/,
	{&NOP, 4} /*0x20*/,
	{&NOP, 4} /*0x21*/,
	{&NOP, 4} /*0x22*/,
	{&NOP, 4} /*0x23*/,
	{&NOP, 4} /*0x24*/,
	{&NOP, 4} /*0x25*/,
	{&NOP, 4} /*0x26*/,
	{&NOP, 4} /*0x27*/,
	{&NOP, 4} /*0x28*/,
	{&NOP, 4} /*0x29*/,
	{&NOP, 4} /*0x2a*/,
	{&NOP, 4} /*0x2b*/,
	{&NOP, 4} /*0x2c*/,
	{&NOP, 4} /*0x2d*/,
	{&NOP, 4} /*0x2e*/,
	{&NOP, 4} /*0x2f*/,
	{&NOP, 4} /*0x30*/,
	{&NOP, 4} /*0x31*/,
	{&NOP, 4} /*0x32*/,
	{&NOP, 4} /*0x33*/,
	{&NOP, 4} /*0x34*/,
	{&NOP, 4} /*0x35*/,
	{&NOP, 4} /*0x36*/,
	{&NOP, 4} /*0x37*/,
	{&NOP, 4} /*0x38*/,
	{&NOP, 4} /*0x39*/,
	{&NOP, 4} /*0x3a*/,
	{&NOP, 4} /*0x3b*/,
	{&NOP, 4} /*0x3c*/,
	{&NOP, 4} /*0x3d*/,
	{&NOP, 4} /*0x3e*/,
	{&NOP, 4} /*0x3f*/,
	{&NOP, 4} /*0x40*/,
	{&NOP, 4} /*0x41*/,
	{&NOP, 4} /*0x42*/,
	{&NOP, 4} /*0x43*/,
	{&NOP, 4} /*0x44*/,
	{&NOP, 4} /*0x45*/,
	{&NOP, 4} /*0x46*/,
	{&NOP, 4} /*0x47*/,
	{&NOP, 4} /*0x48*/,
	{&NOP, 4} /*0x49*/,
	{&NOP, 4} /*0x4a*/,
	{&NOP, 4} /*0x4b*/,
	{&NOP, 4} /*0x4c*/,
	{&NOP, 4} /*0x4d*/,
	{&NOP, 4} /*0x4e*/,
	{&NOP, 4} /*0x4f*/,
	{&NOP, 4} /*0x50*/,
	{&NOP, 4} /*0x51*/,
	{&NOP, 4} /*0x52*/,
	{&NOP, 4} /*0x53*/,
	{&NOP, 4} /*0x54*/,
	{&NOP, 4} /*0x55*/,
	{&NOP, 4} /*0x56*/,
	{&NOP, 4} /*0x57*/,
	{&NOP, 4} /*0x58*/,
	{&NOP, 4} /*0x59*/,
	{&NOP, 4} /*0x5a*/,
	{&NOP, 4} /*0x5b*/,
	{&NOP, 4} /*0x5c*/,
	{&NOP, 4} /*0x5d*/,
	{&NOP, 4} /*0x5e*/,
	{&NOP, 4} /*0x5f*/,
	{&NOP, 4} /*0x60*/,
	{&NOP, 4} /*0x61*/,
	{&NOP, 4} /*0x62*/,
	{&NOP, 4} /*0x63*/,
	{&NOP, 4} /*0x64*/,
	{&NOP, 4} /*0x65*/,
	{&NOP, 4} /*0x66*/,
	{&NOP, 4} /*0x67*/,
	{&NOP, 4} /*0x68*/,
	{&NOP, 4} /*0x69*/,
	{&NOP, 4} /*0x6a*/,
	{&NOP, 4} /*0x6b*/,
	{&NOP, 4} /*0x6c*/,
	{&NOP, 4} /*0x6d*/,
	{&NOP, 4} /*0x6e*/,
	{&NOP, 4} /*0x6f*/,
	{&NOP, 4} /*0x70*/,
	{&NOP, 4} /*0x71*/,
	{&NOP, 4} /*0x72*/,
	{&NOP, 4} /*0x73*/,
	{&NOP, 4} /*0x74*/,
	{&NOP, 4} /*0x75*/,
	{&NOP, 4} /*0x76*/,
	{&NOP, 4} /*0x77*/,
	{&NOP, 4} /*0x78*/,
	{&NOP, 4} /*0x79*/,
	{&NOP, 4} /*0x7a*/,
	{&NOP, 4} /*0x7b*/,
	{&NOP, 4} /*0x7c*/,
	{&NOP, 4} /*0x7d*/,
	{&NOP, 4} /*0x7e*/,
	{&NOP, 4} /*0x7f*/,
	{&NOP, 4} /*0x80*/,
	{&NOP, 4} /*0x81*/,
	{&NOP, 4} /*0x82*/,
	{&NOP, 4} /*0x83*/,
	{&NOP, 4} /*0x84*/,
	{&NOP, 4} /*0x85*/,
	{&NOP, 4} /*0x86*/,
	{&NOP, 4} /*0x87*/,
	{&NOP, 4} /*0x88*/,
	{&NOP, 4} /*0x89*/,
	{&NOP, 4} /*0x8a*/,
	{&NOP, 4} /*0x8b*/,
	{&NOP, 4} /*0x8c*/,
	{&NOP, 4} /*0x8d*/,
	{&NOP, 4} /*0x8e*/,
	{&NOP, 4} /*0x8f*/,
	{&NOP, 4} /*0x90*/,
	{&NOP, 4} /*0x91*/,
	{&NOP, 4} /*0x92*/,
	{&NOP, 4} /*0x93*/,
	{&NOP, 4} /*0x94*/,
	{&NOP, 4} /*0x95*/,
	{&NOP, 4} /*0x96*/,
	{&NOP, 4} /*0x97*/,
	{&NOP, 4} /*0x98*/,
	{&NOP, 4} /*0x99*/,
	{&NOP, 4} /*0x9a*/,
	{&NOP, 4} /*0x9b*/,
	{&NOP, 4} /*0x9c*/,
	{&NOP, 4} /*0x9d*/,
	{&NOP, 4} /*0x9e*/,
	{&NOP, 4} /*0x9f*/,
	{&NOP, 4} /*0xa0*/,
	{&NOP, 4} /*0xa1*/,
	{&NOP, 4} /*0xa2*/,
	{&NOP, 4} /*0xa3*/,
	{&NOP, 4} /*0xa4*/,
	{&NOP, 4} /*0xa5*/,
	{&NOP, 4} /*0xa6*/,
	{&NOP, 4} /*0xa7*/,
	{&NOP, 4} /*0xa8*/,
	{&NOP, 4} /*0xa9*/,
	{&NOP, 4} /*0xaa*/,
	{&NOP, 4} /*0xab*/,
	{&NOP, 4} /*0xac*/,
	{&NOP, 4} /*0xad*/,
	{&NOP, 4} /*0xae*/,
	{&NOP, 4} /*0xaf*/,
	{&NOP, 4} /*0xb0*/,
	{&NOP, 4} /*0xb1*/,
	{&NOP, 4} /*0xb2*/,
	{&NOP, 4} /*0xb3*/,
	{&NOP, 4} /*0xb4*/,
	{&NOP, 4} /*0xb5*/,
	{&NOP, 4} /*0xb6*/,
	{&NOP, 4} /*0xb7*/,
	{&NOP, 4} /*0xb8*/,
	{&NOP, 4} /*0xb9*/,
	{&NOP, 4} /*0xba*/,
	{&NOP, 4} /*0xbb*/,
	{&NOP, 4} /*0xbc*/,
	{&NOP, 4} /*0xbd*/,
	{&NOP, 4} /*0xbe*/,
	{&NOP, 4} /*0xbf*/,
	{&NOP, 4} /*0xc0*/,
	{&NOP, 4} /*0xc1*/,
	{&NOP, 4} /*0xc2*/,
	{&NOP, 4} /*0xc3*/,
	{&NOP, 4} /*0xc4*/,
	{&NOP, 4} /*0xc5*/,
	{&NOP, 4} /*0xc6*/,
	{&NOP, 4} /*0xc7*/,
	{&NOP, 4} /*0xc8*/,
	{&NOP, 4} /*0xc9*/,
	{&NOP, 4} /*0xca*/,
	{&NOP, 4} /*0xcb*/,
	{&NOP, 4} /*0xcc*/,
	{&NOP, 4} /*0xcd*/,
	{&NOP, 4} /*0xce*/,
	{&NOP, 4} /*0xcf*/,
	{&NOP, 4} /*0xd0*/,
	{&NOP, 4} /*0xd1*/,
	{&NOP, 4} /*0xd2*/,
	{&NOP, 4} /*0xd3*/,
	{&NOP, 4} /*0xd4*/,
	{&NOP, 4} /*0xd5*/,
	{&NOP, 4} /*0xd6*/,
	{&NOP, 4} /*0xd7*/,
	{&NOP, 4} /*0xd8*/,
	{&NOP, 4} /*0xd9*/,
	{&NOP, 4} /*0xda*/,
	{&NOP, 4} /*0xdb*/,
	{&NOP, 4} /*0xdc*/,
	{&NOP, 4} /*0xdd*/,
	{&NOP, 4} /*0xde*/,
	{&NOP, 4} /*0xdf*/,
	{&NOP, 4} /*0xe0*/,
	{&NOP, 4} /*0xe1*/,
	{&NOP, 4} /*0xe2*/,
	{&NOP, 4} /*0xe3*/,
	{&NOP, 4} /*0xe4*/,
	{&NOP, 4} /*0xe5*/,
	{&NOP, 4} /*0xe6*/,
	{&NOP, 4} /*0xe7*/,
	{&NOP, 4} /*0xe8*/,
	{&NOP, 4} /*0xe9*/,
	{&NOP, 4} /*0xea*/,
	{&NOP, 4} /*0xeb*/,
	{&NOP, 4} /*0xec*/,
	{&NOP, 4} /*0xed*/,
	{&NOP, 4} /*0xee*/,
	{&NOP, 4} /*0xef*/,
	{&NOP, 4} /*0xf0*/,
	{&NOP, 4} /*0xf1*/,
	{&NOP, 4} /*0xf2*/,
	{&NOP, 4} /*0xf3*/,
	{&NOP, 4} /*0xf4*/,
	{&NOP, 4} /*0xf5*/,
	{&NOP, 4} /*0xf6*/,
	{&NOP, 4} /*0xf7*/,
	{&NOP, 4} /*0xf8*/,
	{&NOP, 4} /*0xf9*/,
	{&NOP, 4} /*0xfa*/,
	{&NOP, 4} /*0xfb*/,
	{&NOP, 4} /*0xfc*/,
	{&NOP, 4} /*0xfd*/,
	{&NOP, 4} /*0xfe*/,
	{&NOP, 4} /*0xff*/
};

#endif //  INSTRUCTION_SET

