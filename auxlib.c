/*
*
* auxlib.c
* (Auxiliary library)
* A little library containing many auxiliary functions.
*
*/


/* SMPL headers */
#include "smpl.h"
#include "state.h"
#include "limits.h"
#include "error.h"
#include "auxlib.h"
#include "mem.h"

/* C headers */
#include <stdio.h>
#include <string.h>


/*
** Check if the top object on the runtime stack is matching the mask 'm'.
** If it's not then throw an error with error code 'ec',
** otherwise return the object.
*/
sp_Object *sp_asserttp (sp_State *S, sp_ObjectType m, sp_ErrorCode ec)
{
	sp_Object *o = sp_top(S, 0);
	if(obj_type(o) & m)
	{
		return o;
	}
	sp_error_throw(S, S->ln, ec);
}


/*
** Check if the top object on the runtime stack is an number.
** If so then pop it and return it's numeric value,
** otherwise throw an error.
*/
sp_Number sp_checknum (sp_State *S)
{
	sp_Number n = obj_num(sp_asserttp(S, OT_NUM, EC_EXP_NUM));
	sp_pop(S, 1);
	return n;
}


/*
** Check if the top object on the runtime stack is an string.
** If so then pop it and return it's string value,
** otherwise throw an error.
*/
sp_String *sp_checkstr (sp_State *S)
{
	sp_String *s = obj_str(sp_asserttp(S, OT_STR, EC_EXP_STR));
	sp_pop(S, 1);
	return s;
}


/*
** Check if the top object on the runtime stack is an C function.
** If so then pop it and return the C function,
** otherwise throw an error.
*/
sp_CFunction sp_checkcf (sp_State *S)
{
	sp_CFunction cf = obj_cfunc(sp_asserttp(S, OT_CFUNC, EC_EXP_CF));
	sp_pop(S, 1);
	return cf;
}


sp_String *sp_topstr (sp_State *S)
{
	return obj_str(sp_asserttp(S, OT_STR, EC_EXP_STR));
}


void sp_do (sp_State *S)
{
	spP_init(S);
	spP_parse(S);
	spC_compile(S);
	spV_execute(S);
}


void sp_dostring (sp_State *S, char *src, uint len)
{
	spX_loadbuff(S, src, len);
	sp_do(S);
}


void sp_doliteral (sp_State *S, char *src)
{
	spX_loadstr(S, src);
	sp_do(S);
}


sp_ErrorCode sp_dofile (sp_State *S,  char *fn)
{
	sp_ErrorCode ec = spX_loadfile(S, fn);
	if(ec)
		return ec;
	sp_do(S);
	return EC_NONE;
}


/*
** Load an buffer 'src' with length 'len'
*/
void spX_loadbuff (sp_State *S, char *src, uint len)
{
	S->p = src;
	S->e = src + len;
}


/*
** Load an literal 's'
*/
void spX_loadstr (sp_State *S, char *s)
{
	spX_loadbuff(S, s, sp_strlen(s));
}


/*
** Load the file 'fn'
*/
sp_ErrorCode spX_loadfile (sp_State *S, char *fn)
{
	FILE *f = fopen(fn, "rb");

	if(f == NULL)
	{
		/* open failed */
		return EC_CNT_F_OP;
	}

	if(fseek(f, 0, SEEK_END) < 0)
	{
		/* seek failed */
		fclose(f);
		return EC_CNT_F_SK;
	}

	int len = ftell(f);
	if(len < 0)
	{
		/* tell failed, cannot tell the length */
		fclose(f);
		return EC_CNT_F_TL;
	}

	if(fseek(f, 0, SEEK_SET) < 0)
	{
		/* seek failed */
		fclose(f);
		return EC_CNT_F_SK;
	}

	/* allocate the buffer */
	char *buff = spM_alloc(S, len + 1);

	int read = fread(buff, 1, len, f);
	if(read != len)
	{
		/* read failed */
		fclose(f);
		return EC_CNT_F_RD;
	}
	buff[len] = '\0';

	S->fn = fn;

	fclose(f);
	spX_loadbuff(S, buff, len);

	return EC_NONE;
}


/*
** Push an object 'o' onto the runtime stack.
** If the stack is not admissible, throw an error.
*/
void spX_push(sp_State* S, sp_Object* o)
{
	spM_assert_push(S, S->stack, S->top, SMPL_STACK_SIZE, S->ln, EC_MAX_STACK, o);
}


/*
** Push an label with index 'i' and the given id onto the label stack.
** If the stack is not admissible, push an error.
*/
void spX_pushlbl(sp_State* S, uint i, uint id)
{
	spM_check_push(S, S->lbls, S->lbltop, SMPL_MAX_LBLS, S->ln, EC_MAX_LBLS, to_label(i, id));
}


/*
** Push an instruction 'i' onto the instruction stack.
** If the stack is not admissible, push an error.
*/
void spX_pushins(sp_State* S, sp_Instruction i)
{
	spM_check_push(S, S->inss, S->instop, SMPL_MAX_INSS, S->ln, EC_MAX_INS, i);
}


/*
** Push an error on line 'ln' and error code 'ec' onto the error stack
** If the stack is not admissible, throw an error.
*/
void spX_pusherr(sp_State* S, uint ln, sp_ErrorCode ec, char *a, char *b)
{
	spM_assert_push(S, S->errs, S->errtop, SMPL_MAX_ERRORS, ln, EC_MAX_ERRS, ERR(ln, ec, a, b));
}


/*
** Push an opcode argument 'o' onto the opargs stack.
** If the stack is not admissible, push an error.
*/
void spX_pushoparg(sp_State* S, sp_Object* o)
{
	spM_check_push(S, S->opargs, S->opargtop, SMPL_MAX_INSS, S->ln, EC_MAX_ARGS, o);
}

void spX_tkerrorA_push (sp_State *S, uint ln, sp_ErrorCode ec, sp_Token a)
{
	sp_errorA_push(S, ln, ec, spD_tk2str(a));
}


void spX_tkerrorAB_push (sp_State *S, uint ln, sp_ErrorCode ec, sp_Token a, sp_Token b)
{
	sp_errorAB_push(S, ln, ec, spD_tk2str(a), spD_tk2str(b));
}