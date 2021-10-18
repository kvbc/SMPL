#ifndef SMPL_H
#define SMPL_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// #define SMPL_DEBUG

#define sp_strlen strlen


typedef uint8_t uchar;
typedef uint32_t uint;
typedef double sp_Number;
typedef uint sp_Instruction;


/* struct typedefs */
typedef struct sp_State		sp_State;
// typedef struct sp_Variable	sp_Variable;
typedef struct sp_String	sp_String;
typedef struct sp_Object	sp_Object;
typedef struct sp_ast		sp_Ast;
typedef struct sp_Error		sp_Error;
typedef struct sp_Label		sp_Label;
typedef struct sp_Dim		sp_Dim;

/* enum typedefs */
typedef enum sp_ErrorCode	sp_ErrorCode;
typedef enum sp_AstType		sp_AstType;
typedef enum sp_Token		sp_Token;
typedef enum sp_OpCode		sp_OpCode;
// typedef enum sp_ObjectType	sp_ObjectType;
typedef uint8_t sp_ObjectType;

typedef struct sp_Function sp_Function;
typedef void (*sp_CFunction)   (sp_State *S, uint nargs);

// SMPL I/O function pointers
typedef void (*sp_printn_F) (sp_State *S, sp_Number n);
typedef void (*sp_prints_F) (sp_State *S, char *s);
typedef void (*sp_gets_F)   (sp_State *S);

// default SMPL I/O functions
void sp_def_printn(sp_State *S, sp_Number n);
void sp_def_prints(sp_State *S, char *s);
void sp_def_gets(sp_State *S);


// environment
void spE_init(sp_State *S);
void spE_free(sp_State *S);
sp_Object *spE_global_new(sp_State *S, char *src, uint len);
sp_Object *spE_global_ensure(sp_State *S, sp_ObjectType m, char *id, uint len);

sp_Object *spE_global_get(sp_State *S, sp_ObjectType t, char *src, uint len);
sp_Object *spE_global_get_string(sp_State *S, char *vs, uint vl);
sp_Object *spE_global_get_number(sp_State *S, char *vs, uint vl);
sp_Object *spE_global_get_CFunction(sp_State *S, char *vs, uint vl);
sp_Object *spE_global_get_function(sp_State *S, char *vs, uint vl);
sp_Object *spE_global_get_dim(sp_State *S, char *vs, uint vl);

void spE_global_set(sp_State *S, sp_ObjectType m, char *src, uint len, sp_Object *o);
void spE_global_set_string(sp_State *S, sp_ObjectType m, char *vs, uint vl, char *ss, uint sl);
void spE_global_set_number(sp_State *S, sp_ObjectType m, char *vs, uint vl, sp_Number n);
void spE_global_set_CFunction(sp_State *S, sp_ObjectType m, char *vs, uint vl, sp_CFunction cf);
void spE_global_set_function(sp_State *S, sp_ObjectType m, char *vs, uint vl, uint i, sp_Object *arg);
void spE_global_set_dim(sp_State *S, sp_ObjectType m, char *vs, uint vl, sp_Object **v, uint32_t len);

// state
sp_State *sp_new_state(void);
void sp_free_state(sp_State *S);

void sp_throw(sp_State *S);
void sp_error_throw(sp_State *S, uint ln, sp_ErrorCode ec);
void sp_error_push(sp_State *S, uint ln, sp_ErrorCode ec);
void sp_errorA_push(sp_State *S, uint ln, sp_ErrorCode ec, char *a);
void sp_errorAB_push(sp_State *S, uint ln, sp_ErrorCode ec, char *a, char *b);

sp_Object *sp_asserttp(sp_State *S, sp_ObjectType t, sp_ErrorCode ec);
sp_Number sp_checknum(sp_State *S);
sp_String *sp_checkstr(sp_State *S);
sp_CFunction sp_checkcf(sp_State *S);
sp_String *sp_topstr(sp_State *S);

void sp_pop(sp_State *S, uint n);
sp_Object *sp_top(sp_State *S, uint n);

void sp_push(sp_State *S, sp_Object *o);
void sp_push_number(sp_State *S, sp_Number n);
void sp_push_string(sp_State *S, char *src, uint len);
void sp_push_literal(sp_State *S, char *src);
void sp_push_CFunction(sp_State *S, sp_CFunction cf);

void sp_call(sp_State *S, uint argn);

void sp_unary_arith(sp_State *S, sp_AstType op);
void sp_binary_arith(sp_State *S, sp_AstType op);
void sp_arith(sp_State *S, sp_AstType op);

void sp_do(sp_State *S);
void sp_dostring(sp_State *S, char *src, uint len);
void sp_doliteral(sp_State *S, char *src);
sp_ErrorCode sp_dofile(sp_State *S, char *fn);

void sp_set_global(sp_State *S, char *src, uint len);
void sp_mget_global(sp_State *S, sp_ObjectType m, char *src, uint len);

void sp_print(sp_State *S);
void sp_set_io(sp_State *S, sp_printn_F pnum, sp_prints_F pstr, sp_gets_F gstr);

#define sp_io_getstr(S)		((*(S)->gstr)((S)))
#define sp_io_printstr(S,s)	((*(S)->pstr)((S), (s)))
#define sp_io_printnum(S,n) ((*(S)->pnum)((S), (n)))


// object
sp_Object *spO_new(sp_State *S);
sp_Object *spO_new_copy(sp_State *S, sp_Object *o);
sp_Object *spO_new_number(sp_State *S, sp_Number n);
sp_Object *spO_new_string(sp_State *S, char *src, uint len);
sp_Object *spO_new_function(sp_State *S, uint i, sp_Object *arg);
sp_Object *spO_new_CFunction(sp_State *S, sp_CFunction cf);
sp_Object *spO_new_dim(sp_State *S, sp_Object **v, uint32_t len);
void spO_free(sp_Object *o);
void spO_free_value(sp_Object *o);
void spO_set_number(sp_Object *o, sp_Number n);
void spO_set_string(sp_State *S, sp_Object *o, char *src, uint len);
void spO_set_CFunction(sp_Object *o, sp_CFunction cf);
void spO_set_function(sp_Object *o, uint i, sp_Object *arg);
void spO_set_dim(sp_State *S, sp_Object *o, sp_Object **v, uint32_t len);
sp_Object *spO_copy(sp_State *S, sp_Object *o1, sp_Object *o2);


// parser
void spP_init(sp_State *S);
void spP_free(sp_State *S);
void spP_parse(sp_State *S);


// memory
void *spM_alloc(sp_State *S, uint size);
uint spM_check_admissible(sp_State *S, uint top, uint cap, uint ln, sp_ErrorCode ec);
void spM_assert_admissible(sp_State *S, uint top, uint cap, uint ln, sp_ErrorCode ec);


// string
sp_String *spS_new(sp_State *S);
sp_String *spS_new_set(sp_State *S, char *src, uint len);
void spS_free(sp_String *s);
sp_String *spS_set(sp_String *s, char *src, uint len);
bool spS_equal(sp_String *s, char *src, uint len);
void spS_toupper(sp_String *s);
void spS_tolower(sp_String *s);

// dim
sp_Dim *spA_new (sp_State *S);
void spA_free (sp_Dim *dim);
sp_Dim *spA_set (sp_State *S, sp_Dim *dim, sp_Object **v, uint32_t len);
sp_Dim *spA_set_vsafe (sp_State *S, sp_Dim *dim, sp_Object **v, uint32_t len);
sp_Dim *spA_set_vobjsafe (sp_State *S, sp_Dim *dim, sp_Object **v, uint32_t len);
sp_Dim *spA_copy (sp_State *S, sp_Dim *dim, sp_Object **v, uint32_t len);
sp_Dim *spA_new_set (sp_State *S, sp_Object **v, uint32_t len);

// lexer
void spL_init(sp_State *S);
void spL_nextc(sp_State *S);
void spL_next(sp_State *S);


// virtual machine
void spV_execute(sp_State *S);


// compile
void spC_compile(sp_State *S);

// debug
#ifdef SMPL_DEBUG
	void  spD_print		(sp_State *S, uint ln, char *fmt, ...);
	char *spD_asttopr	(sp_AstType t);
#endif

char *spD_tk2str(sp_Token t);
char *spD_objt2str(sp_ObjectType t);
char *spD_astt2str(sp_AstType t);
char *spD_errc2str(sp_ErrorCode ec);
char *spD_op2str(sp_OpCode op);

// auxiliary library
void spX_loadbuff(sp_State *S, char *src, uint len);
void spX_loadstr(sp_State *S, char *s);
sp_ErrorCode spX_loadfile(sp_State *S, char *fn);
void spX_push(sp_State* S, sp_Object* o);
void spX_pushlbl(sp_State* S, uint i, uint id);
void spX_pushins(sp_State* S, sp_Instruction i);
void spX_pusherr(sp_State* S, uint ln, sp_ErrorCode ec, char *a, char *b);
void spX_pushoparg(sp_State* S, sp_Object* o);
void spX_tkerrorA_push(sp_State *S, uint ln, sp_ErrorCode ec, sp_Token a);
void spX_tkerrorAB_push(sp_State *S, uint ln, sp_ErrorCode ec, sp_Token a, sp_Token b);


#endif // SMPL_H
