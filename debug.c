/*
*
* debug.c
* Helpers for type debugging
* 
*/


/* SMPL headers */
#include "debug.h"
#include "obj.h"
#include "opcodes.h"
#include "smpl.h"
#include "error.h"
#include "parser.h"
#include "lex.h"
#include "state.h"
#include <stdarg.h>


#ifdef SMPL_DEBUG
	void spD_print (sp_State *S, uint ln, char *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		printf("[%2u]%.*s ", ln, S->tabs, S->tabc);
		vprintf(fmt, args);

		va_end(args);
	}

	char *spD_asttopr (sp_AstType t)
	{
		static char *asttn[] =
		{
			"???",
			"||",
			"&&",
			"<",
			">=",
			">",
			">=",
			"!=",
			"==",
			"+",
			"-",
			"*",
			"/",
			"%",
			"+",
			"-",
			"!"
		};
		if(t > last_astt_opr)
			return "???";
		return asttn[t];
	}
#endif


/*
** Return the C string representation of object type 't'
*/
char *spD_objt2str(sp_ObjectType t)
{
	static char *objtn[] =
	{
		"???",
		"number",
		"string",
		"number | string",
		"function",
		"function | number",
		"function | string",
		"function | number | string",
		"C function",
		"C function | number",
		"C function | string",
		"C function | number | string",
		"C function | function",
		"C function | function | number",
		"C function | function | string",
		"any"
	};
	if(t > last_objt)
		return "???";
	return objtn[t];
}


/*
** Return the C string representation of token 't'
*/
char *spD_tk2str(sp_Token t)
{
	static char *tkn[] =
	{
		/* 0 - 127 */
		"NULL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",				/* 0 - 7 */
		"BS", "TAB", "LF", "VT", "FF", "CR", "SO", "SI",						/* 8 - 15 */
		"DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",					/* 16 - 23 */
		"CAN", "EM", "SUB", "ESC", "FS", "GS", "RS", "US",						/* 24 - 31 */
		"space", "'!'", "'\"'", "'#'", "'$'", "'%'", "'&'", "\"'\"",			/* 32 - 39 */
		"'('", "')'", "'*'", "'+'", "','", "'-'", "'.'", "'/'",					/* 40 - 47 */
		"'0'", "'1'", "'2'", "'3'", "'4'", "'5'", "'6'", "'7'", "'8'", "'9'",	/* 48 - 57 */
		"':'", "';'", "'<'", "'='", "'>'", "'?'", "'@'",						/* 58 - 64 */
		"'A'", "'B'", "'C'", "'D'", "'E'", "'F'", "'G'", "'H'", "'I'", "'J'",	/* 65 - 74 */
		"'K'", "'L'", "'M'", "'N'", "'O'", "'P'", "'Q'", "'R'", "'S'", "'T'",	/* 75 - 84 */
		"'U'", "'V'", "'W'", "'X'", "'Y'", "'Z'",								/* 85 - 90 */
		"'['", "'\\'", "']'", "'^'", "'_'", "\"`\"",							/* 91 - 96 */
		"'a'", "'b'", "'c'", "'d'", "'e'", "'f'", "'g'", "'h'", "'i'", "'j'",	/* 97 - 106 */
		"'k'", "'l'", "'m'", "'n'", "'o'", "'p'", "'q'", "'r'", "'s'", "'t'",	/* 107 - 116 */
		"'u'", "'v'", "'w'", "'x'", "'y'", "'z'",								/* 117 - 122 */
		"'{'", "'|'", "'}'", "'~'", "DEL",										/* 123 - 127 */
		/* > 255 */
		"identifier",
		"string",
		"number",
		"name",
		"end of file",
		"'<'",
		"'<='",
		"'>'",
		"'>='",
		"'<>'",
		"'='",
		"AND",
		"DEF",
		"DIM",
		"DO",
		"ELSE",
		"END",
		"FN",
		"FOR",
		"GOSUB",
		"GOTO",
		"IF",
		"INPUT",
		"LET",
		"LOOP",
		"NEXT",
		"NOT",
		"ON",
		"OR",
		"PRINT",
		"REM",
		"REPEAT",
		"RETURN",
		"STEP",
		"THEN",
		"TO",
		"UNTIL",
		"WEND",
		"WHILE"
	};
	if(t < FIRST_TK)
	{
		if(t < CHAR_MAX)
			return tkn[t];
		return "???";
	}
	return tkn[t-128];
}


/*
** Return the C string representation of AST type 't'
*/
char *spD_astt2str(sp_AstType t)
{
	static char *asttn[] =
	{
		"???",
		"OR",
		"AND",
		"LT",
		"LE",
		"GT",
		"GE",
		"NE",
		"EQ",
		"PLUS",
		"MINUS",
		"MUL",
		"DIV",
		"MOD",
		"POS",
		"NEG",
		"NOT",
		"FN",
		"NUM",
		"STR",
		"VAR",
		"CALL",
		"NAME",
		"IDENT",
		"DIM", //xp
		"DIM", //st
		"DO",
		"DEF",
		"END",
		"FOR",
		"GOSUB",
		"GOTO",
		"IF",
		"INPUT",
		"LOOP",
		"NEXT",
		"ON",
		"PRINT",
		"REPEAT",
		"RETURN",
		"WHILE"
	};
	if(t > last_astt)
		return "???";
	return asttn[t];
}


/*
** Return the C string represntation of opcode 'op'
*/
char *spD_op2str(sp_OpCode op)
{
	static char *opn[] =
	{
		"DUP",
		"POP",
		"LOADN",
		"LOADS",
		"SETVAR",
		"SETDIM",
		"GETNAME",
		"GETDIM",
		"END",
		"FN",
		"FNEND",
		"CALL",
		"INPUT",
		"PRINT",
		"PRINTNL",
		"SAVE",
		"RESTORE",
		"JUMP",
		"JLBL",
		"JTRUE",
		"JFALSE",
		"DEF",
		"DIM",
		"OR",
		"AND",
		"LT",
		"LE",
		"GT",
		"GE",
		"NE",
		"EQ",
		"PLUS",
		"MINUS",
		"MUL",
		"DIV",
		"MOD",
		"POS",
		"NEG",
		"NOT"
	};
	if(op > last_op)
		return "???";
	return opn[op];
}


/*
** Return the C string representation of error code 'ec'
*/
char *spD_errc2str(sp_ErrorCode ec)
{
	static char *errcn[] =
	{
		"none",
		"... and more",
		"variable stack exceeded its limit",
		"stack size exceeded its limit",
		"chunk has too many lines",
		"identifier too long",
		"maximum instructions",
		"maximum arguments",
		"maximum labels",
		"expected %s",
		"expected %s or %s",
		"expected number",
		"expected string",
		"expected C function",
		"unexpected %s",
		"undefined %s '%s'",
		"type mismatch: expected '%s' got '%s'",
		"argument type mismatch: expected '%s' got '%s'",
		"unbalanced parenthesis",
		"unterminated string",
		"uncallable object",
		"malformed number",
		"cannot read in file",
		"cannot open file",
		"cannot seek in file",
		"cannot tell in file",
		"out of memory",
		"%s without %s"
	};
	return errcn[ec];
}