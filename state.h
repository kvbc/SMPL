#ifndef SMPL_STATE_H
#define SMPL_STATE_H

#include <stdbool.h>
#include <setjmp.h>

#include "env.h"
#include "obj.h"
#include "error.h"
#include "lex.h"
#include "smpl.h"
#include "opcodes.h"

#define sp_on_error(S)	if(setjmp((S)->err))
#define sp_any_error(S) ((S)->errtop > 0)


struct sp_Label
{
	uint i;
	uint id;
};


struct sp_State
{
	// execution stack
	uint top;
	sp_Object **stack;

	// runtime environment
	/*
	uint vartop;
	sp_Variable *vars;
	*/
	sp_Object **vars;

	// parsing state
	sp_Ast *ast;
	sp_Ast *gcast;

	// lexing state
	char *fn;
	char *p, *e;
	uint ln;
	char c;
	bool nl;
	sp_Number n;
	sp_String *s;
	sp_Token t;

	uint status;

	sp_printn_F pnum;
	sp_prints_F pstr;
	sp_gets_F gstr;

	uint lbltop;
	sp_Label *lbls;

	uint errtop;
	sp_Error *errs;

	uint instop;
	sp_OpCode *inss;

	uint opargtop;
	sp_Object **opargs;

	jmp_buf err;

#ifdef SMPL_DEBUG
	uint tabs;
	char *tabc;
#endif
};


#endif // SMPL_STATE_H