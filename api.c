/*
*
* api.c
* (Application Programming Interface)
* The core of SMPL's API.
* 
*/


/* SMPL headers */
#include "smpl.h"
#include "parser.h"
#include "state.h"
#include "mem.h"
#include "limits.h"
#include "obj.h"
#include "auxlib.h"

/* C headers */
#include <stdio.h>
#include <stdlib.h>


/*
** Stop the execution with failure status.
*/
void sp_throw (sp_State *S)
{
	S->status = EXIT_FAILURE;
	longjmp(S->err, 1);
}


/*
** Throw an error on line 'ln' with error code 'ec'.
*/
void sp_error_throw (sp_State *S, uint ln, sp_ErrorCode ec)
{
	printf("%s:%d: error (0x%X): %s\n", S->fn, ln, ec, spD_errc2str(ec));
	sp_throw(S);
}


/*
** Pop 'n' objects from the top of the stack.
*/
void sp_pop (sp_State *S, uint n)
{
	while(n)
	{
		--n;
		spO_free(S->stack[--S->top]);
	}
}


/*
** Return the 'n'th object from the top of the stack.
*/
sp_Object* sp_top (sp_State *S, uint n)
{
	return S->stack[S->top-1-n];
}


/*
** Push an error on line 'ln' with error code 'ec' onto the error stack.
*/
void sp_errorAB_push (sp_State *S, uint ln, sp_ErrorCode ec, char *a, char *b)
{
	spX_pusherr(S, ln, ec, a, b);
}


void sp_errorA_push (sp_State *S, uint ln, sp_ErrorCode ec, char *a)
{
	sp_errorAB_push(S, ln, ec, a, NULL);
}


void sp_error_push (sp_State *S, uint ln, sp_ErrorCode ec)
{
	sp_errorAB_push(S, ln, ec, NULL, NULL);
}


/*
** Push an object 'o' onto the stack.
*/
void sp_push (sp_State *S, sp_Object *o)
{
	spX_push(S, o);
}


/*
** Push an number 'n' onto the stack.
*/
void sp_push_number (sp_State *S, sp_Number n)
{
	sp_push(S, spO_new_number(S, n));
}


/*
** Push an string 'src' onto the stack.
*/
void sp_push_string (sp_State *S, char *src, uint len)
{
	sp_push(S, spO_new_string(S, src, len));
}


/*
** Push an literal 'src' onto the stack.
*/
void sp_push_literal (sp_State *S, char *src)
{
	sp_push_string(S, src, sp_strlen(src));
}


/*
** Push an C function 'cf' onto the stack.
*/
void sp_push_CFunction (sp_State *S, sp_CFunction cf)
{
	sp_push(S, spO_new_CFunction(S, cf));
}


/*
** Call the top C function with 'argn' arguments on the stack.
*/
void sp_call (sp_State *S, uint argn)
{
	(*sp_checkcf(S))(S, argn);
}


/*
** Pop an number from the stack and perform an unary operation on it, represented by the operator 'op'.
** Push the result onto the stack.
*/
void sp_unary_arith (sp_State *S, sp_AstType op)
{
	sp_Object *t = sp_top(S, 0);
	switch(op)
	{
	case OPR_POS:
		return;
	case OPR_NEG:
		obj_setnum(t, -obj_num(t));
		break;
	case OPR_NOT:
		obj_setnum(t, !obj_num(t));
		break;
	}
}


/*
** Pop 2 numbers from the stack and perform an binary operation on them, represented by the operator 'op'.
** Push the result onto the stack.
*/
void sp_binary_arith (sp_State *S, sp_AstType op)
{
	sp_Number r;
	sp_Number x = obj_num(sp_top(S, 0));
	sp_Number y = obj_num(sp_top(S, 1));
	sp_pop(S, 2);

	switch(op)
	{
	case OPR_OR:
		r = y || x;
		break;
	case OPR_AND:
		r = y && x;
		break;
	case OPR_LT:
		r = y < x;
		break;
	case OPR_LE:
		r = y <= x;
		break;
	case OPR_GT:
		r = y > x;
		break;
	case OPR_GE:
		r = y >= x;
		break;
	case OPR_NE:
		r = y != x;
		break;
	case OPR_EQ:
		r = y == x;
		break;
	case OPR_PLUS:
		r = y + x;
		break;
	case OPR_MINUS:
		r = y - x;
		break;
	case OPR_MUL:
		r = y * x;
		break;
	case OPR_DIV:
		r = y / x;
		break;
	case OPR_MOD:
		r = y - (int)(y/x) * x;
		break;
	}

	sp_push_number(S, r);
}


/*
** Perform an arithmetic operation on the top numbers, which the operator 'op' represents.
*/
void sp_arith (sp_State *S, sp_AstType op)
{
	if(op < OPR_POS)
		sp_binary_arith(S, op);
	else
		sp_unary_arith(S, op);
}


/*
** Get the global variable with the given id 'src' and object type matching the given mask 'm'.
** Create a new copy of it's object and push it onto the stack.
*/
void sp_mget_global (sp_State *S, sp_ObjectType m, char *src, uint len)
{
	sp_push(S, spO_new_copy(S, sp_smget_oglobal(S, m, src, len)));
}


/*
** Set the global variable with the given id 'src' to the popped object.
*/
void sp_set_global (sp_State *S, char *src, uint len)
{
	sp_sset_oglobal(S, src, len, sp_top(S, 0));
	sp_pop(S, 1);
}


/*
** Pop an object and print it
*/
void sp_print (sp_State *S)
{
	sp_Object *o = sp_top(S, 0);
	if(obj_isnum(o))
		sp_io_printnum(S, obj_num(o));
	else
		sp_io_printstr(S, obj_str(o)->src);
	sp_pop(S, 1);
}


/*
** Set I/O functions used by SMPL
*/
void sp_set_io (sp_State *S, sp_printn_F pnum, sp_prints_F pstr, sp_gets_F gstr)
{
	S->pnum = pnum;
	S->pstr = pstr;
	S->gstr = gstr;
}