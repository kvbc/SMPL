#ifndef SMPL_PARSER_H
#define SMPL_PARSER_H

#include "obj.h"


enum sp_AstType
{
	// Binary Operators
	OPR_OR = 1,
	OPR_AND,
	OPR_LT, OPR_LE,
	OPR_GT, OPR_GE,
	OPR_NE, OPR_EQ,
	OPR_PLUS,
	OPR_MINUS,
	OPR_MUL,
	OPR_DIV,
	OPR_MOD,

	// Unary operators
	OPR_POS,
	OPR_NEG,
	OPR_NOT,

	// Expressions
	XP_FN,
	XP_NUM,
	XP_STR,
	XP_VAR,
	XP_CALL,
	XP_NAME,
	XP_IDENT,
	XP_DIM,

	// Statements
	ST_DIM,
	ST_DO,
	ST_DEF,
	ST_END,
	ST_FOR,
	ST_GOSUB,
	ST_GOTO,
	ST_IF,
	ST_INPUT,
	ST_LOOP,
	ST_NEXT,
	ST_ON,
	ST_PRINT,
	ST_REPEAT,
	ST_RETURN,
	ST_WHILE
};


struct sp_ast
{
	uint ln;
	uint lbl;
	sp_AstType t;
	sp_Object *o;
	struct sp_ast *a, *b, *c, *d;	/* children nodes */
	struct sp_ast *e;				/* next node */
	struct sp_ast *tail;			/* last node */
	struct sp_ast *gcnext;			/* next node in gc */
};


#define last_astt			ST_WHILE


#define first_astt_opr		OPR_OR
#define last_astt_opr		OPR_NOT
#define first_astt_unropr	OPR_POS


#endif // SMPL_PARSER_H