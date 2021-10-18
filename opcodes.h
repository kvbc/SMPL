#ifndef SMPL_OPCODES_H
#define SMPL_OPCODES_H

#include "smpl.h"


/*
*
* Instruction are 32 bit unsigned integers.
* All instructions have an opcode in the lowest 6 bits
* and index of it's argument (stored on its own stack)
* in the highest 26 bits.
*
* 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0
* arg index                                               opcode
*
*/


enum sp_OpCode
{
	OP_DUP,
	OP_POP,

	OP_LOADN,
	OP_LOADS,

	OP_SETVAR,
	OP_SETDIM,
	OP_GETNAME,
	OP_GETDIM,
	
	OP_END,
	OP_FN,
	OP_FNEND,
	OP_CALL,
	OP_INPUT,

	OP_PRINT,
	OP_PRINTNL,

	OP_SAVE,
	OP_RESTORE,

	OP_JUMP,
	OP_JLBL,
	OP_JTRUE,
	OP_JFALSE,

	OP_DEF,
	OP_DIM,

	OP_OR,
	OP_AND,
	OP_LT, OP_LE,
	OP_GT, OP_GE,
	OP_NE, OP_EQ,
	OP_PLUS,
	OP_MINUS,
	OP_MUL,
	OP_DIV,
	OP_MOD,

	OP_POS,
	OP_NEG,
	OP_NOT
}; /* < 64 (2^6) */


#define last_op			OP_NOT
#define first_op_opr	OP_OR


/*
*
* Bitwise operations on instructions
*
*/


#define GET_ARG(i)		((i) >> 6)											/* get index of the argument of instruction 'i' */
#define GET_OPCODE(i)	((i) & 0x3F)										/* get opcode of instruction 'i' */
#define SET_ARG(i,a)	((i) = ((i) | 0xFFFFFFC0) & (((a) << 6) + 0x3F))	/* assign argument index 'a' to instruction 'i' */


#endif // SMPL_OPCODES_H