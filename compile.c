/*
*
* compile.c
* Compile the AST into lower-level executable instructions.
*
*/


/* SMPL headers */
#include "opcodes.h"
#include "state.h"
#include "parser.h"
#include "smpl.h"
#include "mem.h"
#include "error.h"
#include "limits.h"
#include "auxlib.h"
#include "debug.h"


/*
*
* Function declarations
*
*/


static void expr (sp_State *S, sp_Ast *ast);
static void otexpr (sp_State *S, sp_Ast *root, sp_Ast *ast, sp_ObjectType ot, int handlerr);

static sp_Ast *stmt (sp_State *S, sp_Ast *ast);
static void stmtlist (sp_State *S, sp_Ast *ast);


/*
*
* Compilation
*
*/


/* common helpers */
#define here(S)					((S)->instop)					/* instruction stack top index */
#define blank(S)				((S)->instop++)					/* increase instruction stack's top, return previous top index */
#define ins_at(S,i)				((S)->inss[(i)])				/* instruction at index 'i' */
#define top_ins(S)				ins_at((S), (S)->instop - 1)	/* top instruction */


#define compiler_error		sp_error_push
#define compiler_error_A	sp_errorA_push
#define compiler_error_AB	sp_errorAB_push
#define compiler_error_tkA	spX_tkerrorA_push
#define compiler_error_tkAB spX_tkerrorAB_push


/*
** Emit an instruction at the given index
*/
static void emitat (sp_State *S, uint idx, sp_Instruction i)
{
	ins_at(S,idx) = i;
}


/*
** Push an instruction
*/
static void emit (sp_State *S, sp_Instruction i)
{
#ifdef SMPL_DEBUG
	sp_OpCode op = GET_OPCODE(i);
	spD_print(S, 0, "| emit %s [%u]\n", spD_op2str(op), op);
#endif
	spX_pushins(S, i);
}


/*
** Assign top argument to the top instruction
*/
static void setoparg (sp_State *S)
{
	SET_ARG(top_ins(S), S->opargtop - 1);
}


/*
** Emit an jump
*/
static void emitjmp (sp_State *S, sp_Instruction jmp, uint dest)
{
	emit(S, jmp);
	emit(S, dest);
}


/*
** Emit an forward jump.
** Return the index of the destination
** on the instruction stack to be set later on.
*/
static uint emitfwjmp (sp_State *S, sp_Instruction jmp)
{
	emit(S, jmp);
	return blank(S); /* destination */
}


/*
** Emit an conditional jump
*/
static void emitcondjmp (sp_State *S, sp_Ast *cond, sp_Instruction jmp, uint dest)
{
	expr(S, cond);
	emitjmp(S, jmp, dest);
}


/*
** Emit an conditional forward jump
*/
static uint emitcondfwjmp (sp_State *S, sp_Ast *cond, sp_Instruction jmp)
{
	expr(S, cond);
	return emitfwjmp(S, jmp);
}


/*
** Push an argument
*/
static void pusharg (sp_State *S, sp_Object *o)
{
	spX_pushoparg(S, o);
}


/*
** Emit the instruction and assign it an argument
*/
static void emitarg (sp_State *S, sp_Instruction i, sp_Object *o)
{
	emit(S, i);
	pusharg(S, o);
	setoparg(S);
}


/*
*
* Compilation helpers
*
*/


/*
** Get AST node from label
*/
static sp_Ast *getlbl (sp_State *S, uint lbl)
{
	sp_Ast *ast = S->gcast;
	do
	{
		if(ast->lbl == lbl)
			return ast;
		ast = ast->gcnext;
	}
	while(ast);
	return NULL;
}


static sp_ObjectType Sobjt (sp_State *S, sp_String *s)
{
	char lc = s->src[s->len-1];
	if(lc == '%')
		return OT_NUM;
	if(lc == '$')
		return OT_STR;
	return OT_ANY;
}


static inline void register_entry (sp_State *S, sp_String *s, sp_ObjectType t)
{
	sp_Sset_onglobal(S, s, t);
}


/*
** Check if an identifier list includes the given identifier
*/
static bool idlist_includes (sp_State *S, sp_Ast *list, sp_String *id)
{
	do
	{
		if(spS_equal(obj_str(list->o), id->src, id->len))
			return true;
		list = list->e;
	}
	while(list);
	return false;
}


/*
** Compile function arguments and return their count
*/
static uint funcargs (sp_State *S, sp_Ast *ast)
{
	uint argn = 0;
	while(ast)
	{
		expr(S, ast);
		++argn;
		ast = ast->e;
	}
	return argn;
}


/*
*
* Expression compilation
*
*/


#define is_opr(astt)			((astt) <= last_astt_opr)		/* is AstType an operator */
#define is_binopr(astt)			((astt) < first_astt_unropr)	/* is AstType an binary operator */

/* convert an AstType representing an operator to its corresponding OpCode */
#define asttopr_to_opopr(astt)	(first_op_opr + (astt) - first_astt_opr)


/*
** Compile an unary expresion
*/
static void otunrexpr (sp_State *S, sp_Ast *root, sp_Ast *ast, sp_ObjectType ot)
{
	spDO_printf("%s", spDO_asttopr(ast->t));
	otexpr(S, root, ast->a, ot, 1);
	emit(S, asttopr_to_opopr(ast->t));
}


/*
** Compile an binary expression
*/
static void otbinexpr (sp_State *S, sp_Ast *root, sp_Ast *ast, sp_ObjectType ot)
{
	spCDO_printinc(S, ast, "> %s %s\n", spDO_astt2str(ast->t), spDO_asttopr(ast->t));
	otexpr(S, root, ast->a, ot, 1);
	otexpr(S, root, ast->b, ot, 1);
	emit(S, asttopr_to_opopr(ast->t));
	spDO_dectabs(S);
}

/*
** Compile an expression
*/
static void otexpr (sp_State *S, sp_Ast *root, sp_Ast *ast, sp_ObjectType ot, int handlerr)
{
	if(is_opr(ast->t))
	{
		/* operator */
		if(is_binopr(ast->t))
			otbinexpr(S, root, ast, ot); /* binary expression */
		else
			otunrexpr(S, root, ast, ot); /* unary expression */
	}
	else
	{
		/* expression */
		switch(ast->t)
		{
		case XP_FN:
		{
			spCDO_printinc(S, ast, "FN %s\n", obj_str(ast->a->o)->src);
			sp_Object *func = sp_Sget_ofglobal(S, obj_str(ast->a->o));
			if(func)
			{
				/* function exists */
				sp_ObjectType argt = obj_fi(func);
				sp_ObjectType expret = obj_foarg(func);

				/* we don't want our root->d to change */
				otexpr(S, ast, ast->b, argt, 0);

				if(argt == OT_NUM)
				{
					// if any string - error
					if(ast->d == 1)
					{
						compiler_error_AB(S, ast->ln, EC_ARG_TP_MISMATCH, "number", "string");
					}
				}
				else if(argt == OT_STR)
				{
					// if no string - error
					if(ast->d == 0)
					{
						compiler_error_AB(S, ast->ln, EC_ARG_TP_MISMATCH, "string", "number");
					}
				}

				if(ot == OT_NUM)
				{
					// if any string - error
					if(expret == 1)
					{
						compiler_error_AB(S, ast->ln, EC_TP_MISMATCH, "number", "string");
					}
				}
				else if(ot == OT_STR)
				{
					// if no string - error
					if(expret == 0)
					{
						root->d = 2; /* omit the LET error */
						compiler_error_AB(S, ast->ln, EC_TP_MISMATCH, "string", "number");
					}
				}

				emitarg(S, OP_FN, ast->a->o);
			}
			else
			{
				/* undefined function */
				compiler_error_AB(S, ast->ln, EC_UND, "function", obj_str(ast->a->o)->src);
			}
			spDO_dectabs(S);
			break;
		}
		case XP_CALL:
		{
			uint argn = funcargs(S, ast->a);
			emitarg(S, OP_CALL, ast->o);
			emit(S, argn);
			break;
		}
		case XP_NAME:
		case XP_IDENT:
		{
			// if(1)
			sp_Object *entry = sp_Sget_onglobal(S, obj_str(ast->o));
			if(entry)
			{
				/* entry exists */

				sp_ObjectType _ot = obj_num(entry);
				if(_ot == OT_STR)
				{
					root->d = 1;
				}

				if(_ot & (OT_NUM | OT_STR))
					_ot |= (OT_NUM | OT_STR);
				if(ot & _ot)
				{
					spCDO_print(S, ast, "> %s\n", obj_str(ast->o)->src);
					emitarg(S, OP_GETNAME, ast->o);
				}
				else
				{
					compiler_error_AB(S, ast->ln, EC_TP_MISMATCH, spD_objt2str((ot)), spD_objt2str(_ot));
				}
			}
			else
			{
				/* undefined entry */
				compiler_error_AB(S, ast->ln, EC_UND, "entry", obj_str(ast->o)->src);
			}
			break;
		}
		case XP_DIM:
		{
			// if(1)
			if(sp_Sget_onglobal(S, obj_str(ast->o)))
			{
				/* entry exists */
				sp_Ast *dims = ast->a;
				do
				{
					expr(S, dims);
					dims = dims->e;
				}
				while(dims);

				spCDO_print(S, ast, "> %s\n", obj_str(ast->o)->src);
				emitarg(S, OP_GETDIM, ast->o);
				emit(S, ast->b);
			}
			else
			{
				/* undefined entry */
				compiler_error_AB(S, ast->ln, EC_UND, "entry", obj_str(ast->o)->src);
			}
			break;
		}
		case XP_NUM:
		{
			if(ot & (OT_NUM | OT_STR))
			{
				spCDO_print(S, ast, "> %lf\n", obj_num(ast->o));
				emitarg(S, OP_LOADN, ast->o);
			}
			else if(handlerr)
			{
				compiler_error_AB(S, ast->ln, EC_TP_MISMATCH, spD_objt2str(ot), "number");
			}
			break;
		}
		case XP_STR:
		{
			root->d = 1;
			if(ot & OT_STR)
			{
				spCDO_print(S, ast, "> \"%s\"\n", obj_str(ast->o)->src);
				emitarg(S, OP_LOADS, ast->o);
			}
			else if(handlerr)
			{
				compiler_error_AB(S, ast->ln, EC_TP_MISMATCH, spD_objt2str(ot), "string");
			}
			break;
		}
		}
	}
}


static void expr (sp_State *S, sp_Ast *ast)
{
	otexpr(S, ast, ast, OT_ANY, 1);
}


/*
*
* Statement compilation
*
*/


/* save the index of current instruction stack's top */
#define save_idx(id) uint (id) = here(S)


/*
*
* LET statement
*
* a : identifier : variable's identifier
* b : expression : variable's value
*
*/


/*
 * Compile the LET statement.
 */
static void bas_stmt_let (sp_State *S, sp_Ast *ast)
{
	sp_Ast *exproot = ast->b;
	sp_String *ident = obj_str(ast->a->o);
	sp_ObjectType ot = Sobjt(S, ident);
	register_entry(S, ident, ot);
	otexpr(S, exproot, exproot, ot, 1);

	if(ot == OT_STR)
	if(exproot->d == NULL)
	{
		compiler_error_AB(S, ast->ln, EC_TP_MISMATCH, "string", "number");
	}

	if(ast->c)
	{
		sp_Ast *dims = ast->c;
		do
		{
			expr(S, dims);
			dims = dims->e;
		}
		while(dims);

		emitarg(S, OP_SETDIM, ast->a->o);
		emit(S, ast->d);
	}
	else
	{
		emitarg(S, OP_SETVAR, ast->a->o);
	}
}


/*
*
* DO statement
*
* a : statement list
* b : LOOP
*
*/


/*
 * Compile the DO statement.
 * For BASIC 3.5, 4.0, 4.0+ and 7.0.
 */
static void bas_35x4x4Px7_stmt_do (sp_State *S, sp_Ast *ast)
{
	save_idx(b); /* beginning of the loop */
	stmtlist(S, ast->a);
	sp_Instruction jmp = obj_num(ast->b->o)
		? OP_JFALSE /* UNTIL */
		: OP_JTRUE; /* WHILE */
	emitcondjmp(S, ast->b->a, jmp, b);
}


/*
*
* DEF statement
*
* a : identifier : function identifier
* b : identifier : parameter identifier
* c : expression : function's source
*
*/


/*
 * Compile the DEF statement.
 */
static void bas_stmt_def (sp_State *S, sp_Ast *ast)
{
	emitarg(S, OP_DEF, ast->a->o); /* function id */
	pusharg(S, ast->b->o); /* function arg */
	uint jend = emitfwjmp(S, OP_JUMP); /* skip function source */

	sp_ObjectType ot = Sobjt(S, obj_str(ast->b->o));
	// sp_Object *entry = sp_Sget_onglobal(S, obj_str(ast->b->o));
	register_entry(S, obj_str(ast->b->o), ot);
	otexpr(S, ast, ast->c, ot, 1); /* function source */
	/*
	if(entry == NULL)
	{
		spO_free(entry);
	}
	*/
	sp_Sset_ofglobal(S, obj_str(ast->a->o), ot, ast->d);

	emit(S, OP_FNEND);
	emitat(S, jend, here(S)); /* the end of DEF statement */
}


/*
*
* DIM statement
*
*/


/*
 * Compile the DIM statement
 */
static void bas_stmt_dim (sp_State *S, sp_Ast *ast)
{
	sp_Object *o = spE_global_Snew(S, obj_str(ast->a->o));

	sp_Ast *dims = ast->b;
	do
	{
		expr(S, dims);
		dims = dims->e;
	}
	while(dims);

	emitarg(S, OP_DIM, ast->a->o); /* dim id */
	pusharg(S, o);

	uint32_t len = obj_num(ast->o);
	emit(S, len);
}


/*
*
* END statement
*
*/


/*
 * Compile the END statement.
 */
static void bas_stmt_end (sp_State *S, sp_Ast *ast)
{
	emit(S, OP_END);
}


/*
*
* FOR statement
*
* a : LET            : iteration variable assignment
* b : expression     : end value
* c : expression     : step value
* d : statement list
*
*/


/*
 * Compile the FOR statement.
 */
static void bas_stmt_for (sp_State *S, sp_Ast *ast)
{
	spDO_printf("\"%s\"\n", obj_str(ast->a->a->o)->src);

	sp_Ast *stm = ast;
	bas_stmt_let(S, stm->a); /* initial variable */

	/* end the loop if condition is false */
	save_idx(cc);
	emitarg(S, OP_GETNAME, stm->a->a->o);
	expr(S, stm->b); /* TO */
	emit(S, OP_LE);
	uint eol = emitfwjmp(S, OP_JFALSE);

	/* compile the body */
	stmtlist(S, ast->d);

	/* add STEP to the initial variable */
	emitarg(S, OP_GETNAME, stm->a->a->o);
	expr(S, stm->c); /* STEP */
	emit(S, OP_PLUS);
	emitarg(S, OP_SETVAR, stm->a->a->o);

	emitjmp(S, OP_JUMP, cc); /* jump to the condition checking part */
	emitat(S, eol, here(S)); /* set end of the loop */
}


/*
*
* INPUT statement
*
* a       : identifier list
* obj_str : optional message
*
*/


/*
 * Compile the INPUT statement.
 */
static void bas_stmt_input (sp_State *S, sp_Ast *ast)
{
	/* includes an optional message */
	if(ast->o->t == OT_STR)
	{
		emitarg(S, OP_LOADS, ast->o);
		emit(S, OP_PRINT);
	}

	/* load user input */
	emit(S, OP_INPUT);

	ast = ast->a;
	do
	{
		/* set all the provided variables */
		emit(S, OP_DUP);
		emitarg(S, OP_SETVAR, ast->o);

		sp_ObjectType ot = Sobjt(S, ast->o);
		sp_String *ident = obj_str(ast->o);
		register_entry(S, ident, ot);

		ast = ast->e;
	}
	while(ast);

	/* pop user's input */
	emit(S, OP_POP);
}


/*
*
* NEXT statement.
*
*/


/*
** Compile the NEXT statement
*/
static void bas_stmt_next (sp_State *S, sp_Ast *ast)
{
	// compiler_error_tkAB(S, ast->ln, EC_WO, KW_NEXT, KW_FOR);
}


/*
*
* PRINT statement
*
* a : expression list
* b : new-line flag
*
*/


/*
 * Compile the PRINT statement.
 */
static void bas_stmt_print (sp_State *S, sp_Ast *ast)
{
	sp_Ast *stm = ast;
	ast = stm->a;
	while(ast)
	{
		expr(S, ast);
		emit(S, OP_PRINT);
		ast = ast->e;
	}
	if(stm->b)
	{
		emit(S, OP_PRINTNL);
	}
}


/*
*
* REPEAT statement
*
* a : statement list
* b : condition
*
*/


/*
 * Compile the REPEAT statement.
 */
static void bas_stmt_repeat (sp_State *S, sp_Ast *ast)
{
	save_idx(b); /* index of beginning of the loop */
	stmtlist(S, ast->a);
	emitcondjmp(S, ast->b, OP_JTRUE, b); /* jump to the beginning if condition is true */
}


/*
*
* WHILE statement
*
* a : condition
* b : statement list
*
*/


/*
 * Compile the WHILE statement.
 * For BASIC 7.0.
 */
static void bas7_stmt_while (sp_State *S, sp_Ast *ast)
{
	save_idx(cc); /* index of condition checking */
	uint jend = emitcondfwjmp(S, ast->a, OP_JFALSE); /* end the loop if condition is false */
	stmtlist(S, ast->b);
	emitjmp(S, OP_JUMP, cc); /* jump to the condition checking part */
	emitat(S, jend, here(S)); /* set end of the loop */
}


/*
*
* GOTO statement
*
* obj_num : lbl
*
*/


/*
 * Compile the GOTO statement
 * given raw information about the AST node.
 */
static void rawstmt_goto (sp_State *S, uint ln, uint lbl)
{
	emit(S, OP_JLBL);
	emit(S, ln); /* line number */
	emit(S, lbl); /* label */
}


/*
 * Compile the GOTO statement
 * given the AST node.
 */
static void bas_stmt_goto (sp_State *S, sp_Ast *ast)
{
	rawstmt_goto(S, ast->ln, obj_num(ast->o));
}


/*
*
* GOSUB statement
*
* obj_num : lbl
*
*/


/*
 * Compile the GOSUB statement
 * given raw information about the AST node.
 */
static void rawstmt_gosub (sp_State *S, uint ln, uint lbl)
{
	emit(S, OP_SAVE);
	rawstmt_goto(S, ln, lbl);
}


/*
 * Compile the GOSUB statement
 * given the AST node.
 */
static void bas_stmt_gosub (sp_State *S, sp_Ast *ast)
{
	emit(S, OP_SAVE);
	bas_stmt_goto(S, ast);
}


/*
*
* IF statement
*
* a : condition
* b : THEN-body
* c : ELSE-body
* d : THEN-variant
*
*/


/*
 * Compile the IF-THEN body.
 *
 * [D]
 *  0 = IF ... THEN <stmt>
 *  1 = IF ... GOTO <ln>
 *  2 = IF ... GOSUB <ln>
 */
static void stmt_if_body (sp_State *S, sp_Ast *ast)
{
	uint d = ast->d;
	switch(d)
	{
	case 0:
		/* compile stmt */
		stmt(S, ast->b);
		break;
	case 1:
		/* goto ln */
		rawstmt_goto(S, ast->ln, ast->b);
		break;
	case 2:
		/* gosub ln */
		rawstmt_gosub(S, ast->ln, ast->b);
		break;
	}
}


/*
 * Compile the IF statement.
 * For BASIC 2.0 and 4.0.
 *
 * ELSE clause isn't supported.
 */
static void bas2x4_stmt_if (sp_State *S, sp_Ast *ast)
{
	uint jend = emitcondfwjmp(S, ast->a, OP_JFALSE); /* skip the statement if condition is false */
	stmt_if_body(S, ast);
	emitat(S, jend, here(S)); /* set the end of IF */
}


/*
 * Compile the IF statement.
 * For BASIC 3.5, 4.0+ and 7.0.
 *
 * Else clause is supported, ast->c points to it's body or NULL if not present.
 */
static void bas35x4Px7_stmt_if (sp_State *S, sp_Ast *ast)
{
	if(ast->c)
	{
		/* IF ... ELSE */
		uint jelse = emitcondfwjmp(S, ast->a, OP_JFALSE); /* skip to ELSE if condition is false */
		stmt_if_body(S, ast);
		uint jend = emitfwjmp(S, OP_JUMP); /* condition was true, skip the ELSE */
		emitat(S, jelse, here(S)); /* set the beginning of ELSE */
		stmt(S, ast->c);
		emitat(S, jend, here(S)); /* set the end of IF */
	}
	else
	{
		/* IF */
		bas2x4_stmt_if(S, ast);
	}
}


/*
*
* ON statement
*
* a : expression
* b : GOTO
*
*/


/*
 * Compile the ON statement.
 */
static void bas_stmt_on (sp_State *S, sp_Ast *ast)
{
	expr(S, ast->a);
	uint jend = emitfwjmp(S, OP_JFALSE); /* skip GOTO if condition is false */
	bas_stmt_goto(S, ast->b);
	emitat(S, jend, here(S)); /* set the end of GOTO */
}


/*
*
* RETURN statement
*
*/


/*
 * Compile the RETURN statement.
 */
static void bas_stmt_return (sp_State *S, sp_Ast *ast)
{
	emit(S, OP_RESTORE);
}


#define astt_funcf(S, astt, func, dbgf, fmt, end)		\
	case (astt):										\
		dbgf((S), ast, (fmt), spD_astt2str((astt)));	\
		(func)((S), ast);								\
		end

/* AstType -> compile statement */
#define astt_func(S, astt, func, dbgf, end) \
		astt_funcf((S), (astt), (func), dbgf, "> %s\n", end)

#define Dastt_func(S, astt, func, dbgf, end) \
		astt_funcf((S), (astt), (func), dbgf, "> %s ", end)



/*
** Compile an statement
*/
static sp_Ast *stmt (sp_State *S, sp_Ast *ast)
{
	switch(ast->t)
	{
	 astt_func(S, XP_VAR,	 bas_stmt_let,         spCDO_printinc, break);
	 astt_func(S, ST_DO,	 bas_35x4x4Px7_stmt_do,     spCDO_printinc, break);
	 astt_func(S, ST_DEF,	 bas_stmt_def,    spCDO_printinc, break);
	 astt_func(S, ST_DIM,	 bas_stmt_dim,	  spCDO_printinc, break);
	 astt_func(S, ST_END,	 bas_stmt_end,    spCDO_printinc, break);
	Dastt_func(S, ST_FOR,	 bas_stmt_for,    spCDO_printinc, return ast);
	 astt_func(S, ST_GOSUB,	 bas_stmt_gosub,  spCDO_printinc, break);
	 astt_func(S, ST_GOTO,	 bas_stmt_goto,   spCDO_printinc, break);
	 astt_func(S, ST_IF,	 bas35x4Px7_stmt_if,     spCDO_printinc, break);
	 astt_func(S, ST_INPUT,	 bas_stmt_input,  spCDO_printinc, break);
	 astt_func(S, ST_NEXT,	 bas_stmt_next,   spCDO_decprint, return ast);
	 astt_func(S, ST_ON,	 bas_stmt_on,	  spCDO_printinc, break);
	 astt_func(S, ST_PRINT,	 bas_stmt_print,  spCDO_printinc, break);
	 astt_func(S, ST_REPEAT, bas_stmt_repeat, spCDO_printinc, break);
	 astt_func(S, ST_RETURN, bas_stmt_return, spCDO_printinc, break);
	 astt_func(S, ST_WHILE,	 bas7_stmt_while,  spCDO_printinc, break);
	}
	spDO_dectabs(S);

	if(ast != ast->tail)
	if(ast->e != NULL)
	{
		return stmt(S, ast->e);
	}

	return ast;
}


/*
** Compile an statement list
*/
static void stmtlist (sp_State *S, sp_Ast *ast)
{
	while(ast != NULL)
	{
		spX_pushlbl(S, here(S), ast->lbl);
		ast = stmt(S, ast);
		ast = ast->e;
	}
}


/*
** Compile the AST
*/
void spC_compile (sp_State *S)
{
	spDO_printf("========================================================================================\n"
	            "| COMPILATION\n"
		        "========================================================================================\n");

	stmtlist(S, S->ast);
	if(sp_any_error(S))
	{
		sp_throw(S);
	}
}
