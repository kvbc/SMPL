/*
*
* parser.c
*
*/


/* SMPL headers */
#include "state.h"
#include "parser.h"
#include "lex.h"
#include "obj.h"
#include "error.h"
#include "mem.h"
#include "smpl.h"
#include "debug.h"

/* C headers */
#include <stdbool.h>


/*
*
* AST Node
*
*/


/* node allocation */
#define node_new_0(S,t)			(node_new(S,t,0,0,0,0))	/* new node with no children nodes */
#define node_new_1(S,t,a)		(node_new(S,t,a,0,0,0))	/* new node with 1 children node */
#define node_new_2(S,t,a,b)		(node_new(S,t,a,b,0,0))	/* new node with 2 children nodes */
#define node_new_3(S,t,a,b,c)	(node_new(S,t,a,b,c,0))	/* new node with 3 children nodes */
#define node_new_4(S,t,a,b,c,d)	(node_new(S,t,a,b,c,d))	/* new node with 4 children nodes */

/* node manipulation */
#define node_set_num(a,n)		(spO_set_number((a)->o, (n)))						/* set the numeric value of an node to 'n' */
#define node_set_str(S,a,s)		(spO_set_string((S), (a)->o, (s)->src, (s)->len))	/* set the string value of an node to 's' */


/*
*
* Token
*
*/


#define tk_block_end(t)			((t)==KW_LOOP || (t)==KW_UNTIL || (t)==KW_WEND || (t)==TK_EOF)	/* end of block? */
#define EOS(S)					((S)->nl || (S)->t == ':' || (S)->t == TK_EOF)					/* end of statement? (':', new-line, end of file) */
#define EOF(S)					((S)->c < 0)													/* end of file? */
#define EOL(S)					(EOF((S)) || (S)->c == '\n' || (S)->c == '\r')					/* end of line?		 ('\n', '\r', end of file) */


/*
*
* Syntax error
*
*/


/* push an syntax error to the error stack */
#define syntax_error(S,ec)			sp_error_push((S), (S)->ln, (ec))

#define syntax_error_A(S,ec,a)		sp_errorA_push((S), (S)->ln, (ec), (a))
#define syntax_error_AB(S,ec,a,b)	sp_errorAB_push((S), (S)->ln, (ec), (a), (b))

#define syntax_tkerror_A(S,ec,a)	spX_tkerrorA_push((S), (S)->ln, (ec), (a))
#define syntax_tkerror_AB(S,ec,a,b)	spX_tkerrorAB_push((S), (S)->ln, (ec), (a), (b))


/*
*
* Function declarations
*
*/


/* expr */
static sp_Ast *expr (sp_State *S);
static sp_Ast *exprlist (sp_State *S, sp_Token sep, int *nl, uint32_t *len);

/* stmt */
static sp_Ast *stmt (sp_State *S);
static sp_Ast *tkstmt (sp_State *S, sp_Token t);
static sp_Ast *nlstmt (sp_State *S);
static sp_Ast *nlstmtlist (sp_State *S);

/* id stmt */
static sp_Ast *istmt (sp_State *S, sp_String *id);
static sp_Ast *isstmt (sp_State *S, sp_String *id);
static sp_Ast *itkstmt (sp_State *S, sp_Token t, sp_String *id);
static sp_Ast *inlstmt (sp_State *S, sp_String *id);
static sp_Ast *inlstmtlist (sp_State *S, sp_String *id);


/*
*
* Node manipulation
*
*/


/*
** Free an AST node
*/
static void node_free (sp_Ast *ast)
{
	spO_free(ast->o);
}


/*
** Free the AST
*/
static void ast_free (sp_Ast *ast)
{
	while(ast)
	{
		node_free(ast);
		ast = ast->gcnext;
	}
}


/*
** Allocate an new AST node and initialize it.
** Return an pointer to it.
*/
static sp_Ast *node_new (sp_State *S, sp_AstType t, sp_Ast *a, sp_Ast *b, sp_Ast *c, sp_Ast *d)
{
	sp_Ast *ast = spM_alloc_sizeof(S, *ast);

	ast->t = t;
	ast->a = a;
	ast->b = b;
	ast->c = c;
	ast->d = d;
	ast->e = NULL;
	ast->tail = NULL;
	ast->lbl = 0;
	ast->ln = S->ln;
	ast->o = spO_new(S);

	ast->gcnext = S->gcast;
	S->gcast = ast;

	return ast;
}


/*
** Create an new AST node and set its object value to number 'n'
*/
static sp_Ast *node_new_num (sp_State *S, sp_AstType t, sp_Number n)
{
	sp_Ast *ast = node_new_0(S, t);
	node_set_num(ast, n);
	return ast;
}


/*
** Create an new AST node and set its object value to SMPL string 's'
*/
static sp_Ast *node_new_str (sp_State *S, sp_AstType t, sp_String *s)
{
	sp_Ast *ast = node_new_0(S, t);
	node_set_str(S, ast, s);
	return ast;
}


/*
*
* Parser
*
*/


/*
** Initialize the parser
*/
void spP_init (sp_State *S)
{
	S->gcast = NULL;
}


/*
** Free the parser
*/
void spP_free (sp_State *S)
{
	ast_free(S->gcast);
}


/*
*
* Lexer helpers
*
*/


/*
** If the current token is 't' return 1.
** Otherwise push an syntax error and return 0.
*/
static int match (sp_State *S, sp_Token t)
{
	if(S->t != t)
	{
		printf("ERR: expected %s, got %s\n", spD_tk2str(t), spD_tk2str(S->t));
		syntax_tkerror_A(S, EC_EXP_SB, t);
		return 0;
	}
	return 1;
}


/*
** If the current token is 't1' or 't2' return 1.
** Otherwise push an syntax error and return 0.
*/
static int match2 (sp_State *S, sp_Token t1, sp_Token t2)
{
	if(S->t != t1 && S->t != t2)
	{
		syntax_tkerror_AB(S, EC_EXP_SBS, t1, t2);
		return 0;
	}
	return 1;
}


/*
** Check if the next token is 't'.
** If not, push an syntax error.
*/
static void expect (sp_State *S, sp_Token t)
{
	spL_next(S);
	match(S, t);
}


/*
** If the current token is 't', skip it.
** Otherwise push an syntax error.
*/
static void accept (sp_State *S, sp_Token t)
{
	if(match(S, t))
	{
		spL_next(S);
	}
}


/*
*
* Expression parsing helpers
*
*/


/*
** Parse an function identifier
*/
static sp_Ast *funcid (sp_State *S)
{
	match2(S, TK_IDENT, TK_NAME);
	sp_AstType t = (S->t == TK_IDENT)
		? XP_IDENT
		: XP_NAME;
	sp_Ast *ast = node_new_str(S, t, S->s);
	spL_next(S);
	return ast;
}


/*
** Parse an identifier
*/
static sp_Ast *identifier (sp_State *S)
{
	match(S, TK_IDENT);
	sp_Ast *ast = node_new_str(S, XP_IDENT, S->s);
	spL_next(S);
	return ast;
}


static sp_Ast *idlist (sp_State *S)
{
	sp_Ast *head, *tail;
	head = tail = identifier(S);
	while(S->t == ',')
	{
		spL_next(S);
		tail = tail->e = identifier(S);
	}
	return head;
}


static int idlist_includes (sp_State *S, sp_Ast *list, sp_String *s)
{
	while(list != NULL)
	{
		if(spS_equal(obj_str(list->o), s->src, s->len))
			return 1;
		list = list->e;
	}
	return 0;
}


/*
** Parse function arguments
*/
static sp_Ast *funcargs (sp_State *S)
{
	if(S->t == ')')
		return NULL;
	int nl;
	uint32_t len;
	return exprlist(S, ',', &nl, &len);
}


/*
*
* Expression parsing
*
*/


#define tk_astt(t,at) case (t): return (at);	/* Token -> AstType */
#define def_astt(at)  default: return (at);		/* Default AstType */


/*
** Convert Token 't' representing an unary operator to its corresponding AstType.
** Return 0 if invalid.
*/
static sp_AstType unropr (sp_Token t)
{
	switch(t)
	{
	tk_astt('+',	OPR_POS);
	tk_astt('-',	OPR_NEG);
	tk_astt(KW_NOT, OPR_NOT);
	def_astt(0);
	}
}


/*
** Convert Token 't' representing an binary operator to its corresponding AstType.
** Return 0 if invalid.
*/
static sp_AstType binopr (sp_Token t)
{
	switch(t)
	{
	tk_astt(KW_OR,	OPR_OR);
	tk_astt(KW_AND, OPR_AND);
	tk_astt(TK_LT,	OPR_LT);
	tk_astt(TK_LE,	OPR_LE);
	tk_astt(TK_GT,	OPR_GT);
	tk_astt(TK_GE,	OPR_GE);
	tk_astt(TK_NE,	OPR_NE);
	tk_astt(TK_EQ,	OPR_EQ);
	tk_astt('+',	OPR_PLUS);
	tk_astt('-',	OPR_MINUS);
	tk_astt('*',	OPR_MUL);
	tk_astt('/',	OPR_DIV);
	tk_astt('%',	OPR_MOD);
	def_astt(0);
	}
}


/* Priority of binary operators */
static const uint priority[] =
{
	0,			/*  invalid  */
	1, 2,		/*  OR, AND  */
	3, 3,		/*  <, <=    */
	3, 3,		/*  >, >=    */
	3, 3,		/*  <>, =    */
	4, 4,		/*  +, -     */
	5, 5, 5		/*  *, /, %  */
};

/* Priority of unary operators */
#define UNARY_PRIORITY 6 /* -, +, NOT */


/*
** Parse an primary expression
*/
static sp_Ast *primary (sp_State *S, uint unkerr)
{
	sp_Ast *ast;
	switch(S->t)
	{
	case '(':
	{
		spL_next(S);
		ast = expr(S);
		accept(S, ')');
		break;
	}
	case TK_IDENT:
	{
		ast = node_new_str(S, XP_IDENT, S->s);
		spL_next(S);
		if(S->t == '(')
		{
			spL_next(S);
			ast->t = XP_DIM;
			int nl;
			uint32_t len;
			ast->a = exprlist(S, ',', &nl, &len);
			ast->b = len;
			accept(S, ')');
		}
		break;
	}
	case KW_FN:
	{
		uint ln = S->ln;
		spL_next(S);
		sp_Ast *a = funcid(S);
		accept(S, '(');
		sp_Ast *b = expr(S);
		accept(S, ')');
		sp_Ast *ast_fn = node_new_2(S, XP_FN, a, b);
		ast_fn->ln = ln; /* used in compiler.c, need to overwrite cuz takes S->ln by default */
		return ast_fn;
	}
	case TK_NAME:
	{
		ast = node_new_str(S, XP_NAME, S->s);
		spL_next(S);
		if(S->t == '(')
		{
			spL_next(S);
			ast->t = XP_CALL;
			ast->a = funcargs(S);
			accept(S, ')');
		}
		return ast;
	}
	case TK_NUMBER:
	{
		ast = node_new_num(S, XP_NUM, S->n);
		spL_next(S);
		break;
	}
	case TK_STRING:
	{
		ast = node_new_str(S, XP_STR, S->s);
		spL_next(S);
		break;
	}
	default:
		if(unkerr)
			syntax_tkerror_A(S, EC_UNEXP_SB, S->t);
		else
			return NULL;
		spL_next(S);
		break;
	}

	return ast;
}


/*
** Parse an subexpression
*/
static sp_Ast *subexpr (sp_State *S, uint limit, uint unkerr)
{
	sp_Ast *a;

	sp_AstType uop = unropr(S->t);
	if(uop)
	{
		/* is an unary operator */
		spL_next(S);
		a = node_new_1(S, uop, subexpr(S, UNARY_PRIORITY, unkerr));
	}
	else
	{
		/* not an unary operator, must be a primary expression */
		a = primary(S, unkerr);
	}

	sp_AstType op = binopr(S->t);
	while(priority[op] > limit)
	{
		spL_next(S);
		sp_Ast *b = subexpr(S, priority[op], unkerr);
		a = node_new_2(S, op, a, b);
		op = binopr(S->t);
	}

	return a;
}


/*
** Parse an expression
*/
static sp_Ast *expr (sp_State *S)
{
	return subexpr(S, 0, 1);
}


/*
** Parse an silent expression
*/
static sp_Ast *sexpr (sp_State *S)
{
	return subexpr(S, 0, 0);
}


/*
** Parse an expression list, which is separated by 'sep'
*/
static sp_Ast *exprlist (sp_State *S, sp_Token sep, int *nl, uint32_t *len)
{
	*len = 1;
	sp_Ast *head, *tail;
	head = tail = expr(S);
	while(S->t == sep)
	{
		(*len)++;
		uint ln = S->ln;
		spL_next(S);
		if(S->ln != ln)
		{
			*nl = 0;
			return head;
		}
		sp_Ast *t = sexpr(S);
		tail = tail->e = t;
		if(t == NULL)
		{
			*nl = 0;
			return head;
		}
	}
	*nl = 1;
	return head;
}


/*
*
* Statement Parsing
*
*/


/*
 * Parse the REM statement.
 *
 * SYNTAX > REM <anything>
 */
static sp_Ast *bas_stmt_rem (sp_State *S)
{
	/* just whitespace after REM */
	if(S->nl)
		return nlstmt(S);
	/* non-whitespace after REM */
	do spL_nextc(S);
	while(!EOL(S));
	/* end of file */
	if(EOF(S))
	{
		S->t = TK_EOF;
		return NULL;
	}
	/* next statement */
	spL_next(S);
	return nlstmt(S);
}


/*
*
* RETURN statement
*
*/


/*
 * Parse the RETURN statement.
 *
 * SYNTAX > RETURN
 */
static sp_Ast *bas_stmt_return (sp_State *S)
{
	return node_new_0(S, ST_RETURN);
}


/*
*
* END statement
*
*/


/*
 * Parse the END statement.
 *
 * SYNTAX > END
 */
static sp_Ast *bas_stmt_end (sp_State *S)
{
	return node_new_0(S, ST_END);
}


/*
*
* LET statement
*
*/


/*
 * Parse the LET statement.
 *
 * SYNTAX > [LET] <id> = <val>
 */
static sp_Ast *bas_stmt_let (sp_State *S)
{
	sp_Ast *a = identifier(S);
	sp_Ast *c = NULL;
	sp_Ast *d = NULL;

	if(S->t == '(')
	{
		spL_next(S);
		int nl;
		uint32_t len;
		c = exprlist(S, ',', &nl, &len);
		d = len;
		accept(S, ')');
	}

	accept(S, TK_EQ);
	sp_Ast *b = expr(S);
	spPDO_print(S, "> VAR : '%s'\n", obj_str(a->o)->src);

	sp_Ast *ast = node_new_4(S, XP_VAR, a, b, c, d);
	return ast;
}


/*
*
* DIM statement
*
*/

/*
 * Parse the DIM statement.
 *
 * SYNTAX > DIM <id>(<dim> [{, <dim>}]) [{, <id>(<dim> [{, <dim>}])}]
 */

static sp_Ast *bas_stmt_dim (sp_State *S)
{
	sp_Ast *a = identifier(S);
	accept(S, '(');

	int nl;
	uint32_t len;
	sp_Ast *b = exprlist(S, ',', &nl, &len);
	accept(S, ')');

	sp_Ast *ast = node_new_2(S, ST_DIM, a, b);
	node_set_num(ast, len);
	return ast;
}


/*
*
* LOOP statement
*
*/


/*
 * Parse the LOOP statement.
 * For BASIC 3.5, 4.0, 4.0+ and 7.0
 *
 * SYNTAX > LOOP (UNTIL|WHILE) <expr>
 */
static sp_Ast *bas_35x4x4Px7_stmt_loop (sp_State *S)
{
	sp_Number until;
	if(S->t == KW_UNTIL)
		until = 1;
	else if(S->t == KW_WHILE)
		until = 0;
	else
		syntax_tkerror_AB(S, EC_EXP_SBS, KW_UNTIL, KW_WHILE); /* expected UNTIL or WHILE */
	spL_next(S);

	sp_Ast *ast = node_new_1(S, ST_LOOP, expr(S));
	node_set_num(ast, until);
	return ast;
}


/*
*
* DO statement
*
*/


/*
 * Parse the DO statement.
 * For BASIC 3.5, 4.0, 4.0+ and 7.0.
 *
 * SYNTAX > DO (UNTIL|WHILE) <expr> <stmts> LOOP
 *          DO <stmts> LOOP (UNTIL|WHILE) <expr>
 *
 * TODO: second syntax
 */
static sp_Ast *bas_35x4x4Px7_stmt_do (sp_State *S)
{
	sp_Ast *a = nlstmtlist(S);
	accept(S, KW_LOOP);
	return node_new_2(S, ST_DO, a, bas_35x4x4Px7_stmt_loop(S));
}


/*
*
* DEF statement
*
*/


/*
 * Parse the DEF statement.
 *
 * SYNTAX > DEF FN<funcid>(<prmtrid>) = <expr>
 */
static sp_Ast *bas_stmt_def (sp_State *S)
{
	accept(S, KW_FN);
	sp_Ast *a = funcid(S);
	accept(S, '(');
	sp_Ast *b = identifier(S);
	accept(S, ')');
	accept(S, TK_EQ);
	sp_Ast *c = expr(S);
	return node_new_3(S, ST_DEF, a, b, c);
}


/*
*
* NEXT statement
*
*/


/*
 * Parse the NEXT statement.
 *
 * SYNTAX > NEXT [<id> [{,<id>}]]
 */
static sp_Ast *bas_stmt_next (sp_State *S, sp_String *id)
{
	sp_Ast *a = NULL;
	if(S->t == TK_IDENT)
		a = idlist(S);
	return node_new_1(S, ST_NEXT, a);
}


/*
*
* FOR statement
*
*/


/*
 * Parse the FOR statement.
 *
 * SYNTAX:
 *        NL      ::= <newline> <linenum>    (*new-line*)
 *        FRSTMT  ::= <stmt other than NEXT> (*FOR statement*)
 *        NLSTMT  ::= <NL> <FRSTMT>          (*new-line statement*)
 *        SLSTMT  ::= : <FRSTMT>             (*single-line statement *)
 *        ENDNEXT ::= (<NL>|:) NEXT
 * FOR <id>=<num> TO <num> [STEP <num>] [{<NLSTMT>|<SLSTMT>}] <ENDNEXT> [<id> [{,<id>}]]
 */
static sp_Ast *bas_stmt_for (sp_State *S)
{
	sp_Ast *a = bas_stmt_let(S);
	accept(S, KW_TO);
	sp_Ast *b = expr(S);

	sp_Ast *c;
	if(S->t == KW_STEP)
	{
		spL_next(S);
		c = expr(S);
	}
	else
	{
		c = node_new_num(S, XP_NUM, 1.0);
	}

	sp_Ast *d = inlstmtlist(S, obj_str(a->a->o));

	return node_new_4(S, ST_FOR, a, b, c, d);
}


/*
*
* GOSUB statement
*
*/


/*
 * Parse the GOSUB statement.
 *
 * SYNTAX > GOSUB <linenum>
 */
static sp_Ast *bas_stmt_gosub (sp_State *S)
{
	sp_Number lbl = S->n;
	spL_next(S);
	return node_new_num(S, ST_GOSUB, lbl);
}


/*
*
* GOTO statement
*
*/


/*
 * Parse the GOTO statement.
 *
 * SYNTAX > (GOTO|(GO TO)) <linenum>
 *
 * TODO: GO TO
 */
static sp_Ast *bas_stmt_goto (sp_State *S)
{
	sp_Number lbl = S->n;
	spL_next(S);
	return node_new_num(S, ST_GOTO, lbl);
}


/*
*
* IF statement
*
*/


/*
 * Parse the IF-THEN statement.
 * For BASIC 2.0 and 4.0.
 *
 * SYNTAX > IF <expr> ((THEN <linenum|cmd>) | ((GOTO|GOSUB) <linenum>))
 */
static sp_Ast *bas2x4_stmt_if (sp_State *S)
{
	sp_Ast *a = expr(S);

	sp_Token t = S->t;
	spL_next(S);

	sp_Ast *b;
	sp_Ast *ast = node_new_0(S, ST_IF);

	if(t == KW_THEN)
	{
		if(S->t == TK_NUMBER)
		{
			/* THEN <linenum> */
			b = (uint)(S->n);
			ast->d = 1; /* goto ln */
			spL_next(S);
		}
		else
		{
			/* THEN <cmd> */
			b = stmt(S);
			ast->d = 0; /* compile cmd */
		}
	}
	else if(t == KW_GOTO || t == KW_GOSUB)
	{
		/* (GOTO|GOSUB) <linenum> */
		match(S, TK_NUMBER);
		b = (uint)(S->n);
		ast->d = (t == KW_GOTO) ? 1  /* goto ln */
		                        : 2; /* gosub ln */
		spL_next(S);
	}
	else
	{
		syntax_error_AB(S, EC_UND, "IF variant", S->s->src);
	}

	ast->a = a;
	ast->b = b;

	return ast;
}


/*
 * Parse the IF-THEN-ELSE statement.
 * For BASIC 3.5, 4.0+, 7.0
 *
 * ==========================================================================================
 *
 * Current state:
 *
 * SYNTAX > IF <expr> ((THEN <linenum|cmd>) | ((GOTO|GOSUB) <linenum>)) [ELSE <stmt>]
 *
 * ==========================================================================================
 *
 * Expected state:
 *
 * ELSE being an separate command, not only a keyword.
 * SYNTAX > IF <expr> ((THEN <linenum|cmd>) | ((GOTO|GOSUB) <linenum>))
 *
 * ==========================================================================================
 *
 * Although the expected syntax of BASIC 3.5, 4.0+ and 7.0 featured
 * the keyword ELSE as an separate command. I find it better to treat it
 * only as a keyword, so all the checks for the potential ELSE command
 * could be skipped. Thus we are left with it being just a keyword
 * and this function having only one check for it's presence.
 */
static sp_Ast *bas35x4Px7_stmt_if (sp_State *S)
{
	sp_Ast *ast = bas2x4_stmt_if(S);

	if(S->t == KW_ELSE)
	{
		spL_next(S);
		ast->c = stmt(S);
	}

	return ast;
}


/*
*
* INPUT statement
*
*/


/*
 * Parse the INPUT statement.
 *
 * SYNTAX > INPUT [<string>;] <var> [{,<var>}]
 */
static sp_Ast *bas_stmt_input (sp_State *S)
{
	sp_Ast *ast = node_new_0(S, ST_INPUT);
	if(S->t == TK_STRING)
	{
		node_set_str(S, ast, S->s);
		expect(S, ';');
		spL_next(S);
	}
	ast->a = idlist(S);
	return ast;
}


/*
*
* ON statement
*
*/


/*
 * Parse the ON statement.
 *
 * SYNTAX > ON <index> (GOTO|GOSUB) <line> [{,<line>}]
 *
 * TODO: completely wrong
 */
static sp_Ast *bas_stmt_on (sp_State *S)
{
	sp_Ast *a = expr(S);
	accept(S, KW_GOTO);
	return node_new_2(S, ST_ON, a, bas_stmt_goto(S));
}


/*
*
* PRINT statement
*
*/


/*
 * Parse the PRINT statement.
 *
 * DESIRED SYNTAX > PRINT [<expr> {(;|,) <expr>}]
 * CURRENT SYNTAX > PRINT [<expr> {; <expr>}]
 *
 * Trailing semicolon will not move the cursor to the new line, as it would without one.                                            [X]
 * Items can be entered with no separators, as long as they could be identified as separate expressions.                            [ ]
 * Numbers will be printed with a leading space for positive values or minus sign for negative, and always with a trailing space.   [ ]
 */
static sp_Ast *bas_stmt_print (sp_State *S)
{
	sp_Ast *a = NULL;
	sp_Ast *b = 1; /* end with a new line */
	if(S->t == ';')
	{
		b = 0;
		spL_next(S);
	}
	else if(!EOS(S))
	{
		int nl;
		uint32_t len;
		a = exprlist(S, ';', &nl, &len);
		b = nl;
	}

	spPDO_print(S, "> PRINT\n");
	spPDO_print(S, "    a (exprlist) : %d\n", a != NULL);
	spPDO_print(S, "    b (new line) : %d\n", b);

	return node_new_2(S, ST_PRINT, a, b);
}


/*
*
* UNTIL statement
*
*/


/*
 * Parse the UNTIL statement.
 *
 * SYNTAX > UNTIL <expr>
 */
static sp_Ast *bas_stmt_until (sp_State *S)
{
	return expr(S);
}


/*
*
* REPEAT statement
*
*/


/*
 * Parse the REPEAT statement.
 *
 * SYNTAX > REPEAT <stmts> UNTIL <expr>
 *
 * TODO: Apparently doesn't exist
 */
static sp_Ast *bas_stmt_repeat (sp_State *S)
{
	sp_Ast *a = nlstmtlist(S);
	accept(S, KW_UNTIL);
	return node_new_2(S, ST_REPEAT, a, bas_stmt_until(S));
}


/*
*
* WHILE statement
*
*/


/*
 * Parse the WHILE statement.
 * For BASIC 7.0.
 *
 * SYNTAX > WHILE <expr> <stmts> WEND
 */
static sp_Ast *bas7_stmt_while (sp_State *S)
{
	sp_Ast *a = expr(S);
	sp_Ast *b = nlstmtlist(S);
	accept(S, KW_WEND);
	return node_new_2(S, ST_WHILE, a, b);
}


/*
*
* Statement parsing helpers
*
*/



#define tkn_func(tk,func) case (tk): spL_next(S); return (func)(S); break;	/* Token -> next token, parse statement */
#define tk_func(tk,func)  case (tk): return (func)(S); break;				/* Token ->	parse statement */


static sp_Ast *itkstmt (sp_State *S, sp_Token t, sp_String *id)
{
	switch(t)
	{
	case KW_NEXT:
	{
		spL_next(S);
		return bas_stmt_next(S, id);
	}
	tkn_func(KW_DO,		bas_35x4x4Px7_stmt_do);
	tkn_func(KW_DEF,	bas_stmt_def);
	tkn_func(KW_DIM,	bas_stmt_dim);
	tkn_func(KW_END,	bas_stmt_end);
	tkn_func(KW_FOR,	bas_stmt_for);
	tkn_func(KW_GOSUB,	bas_stmt_gosub);
	tkn_func(KW_GOTO,	bas_stmt_goto);
	tkn_func(KW_IF,		bas35x4Px7_stmt_if);
	tkn_func(KW_INPUT,	bas_stmt_input);
	tkn_func(KW_LET,	bas_stmt_let);
	tkn_func(KW_ON,		bas_stmt_on);
	tkn_func(KW_PRINT,	bas_stmt_print);
	tkn_func(KW_REM,	bas_stmt_rem);
	tkn_func(KW_REPEAT, bas_stmt_repeat);
	tkn_func(KW_RETURN, bas_stmt_return);
	tkn_func(KW_WHILE,	bas7_stmt_while);
	tk_func (TK_IDENT,	bas_stmt_let);
	}
	return NULL;
}


static sp_Ast *tkstmt (sp_State *S, sp_Token t)
{
	return itkstmt(S, t, NULL);
}


/*
** Parse an single statement
*/
static sp_Ast *isstmt (sp_State *S, sp_String *id)
{
	sp_Token t = S->t;
	sp_Ast *ast = itkstmt(S, S->t, id);
	if(ast == NULL && t != KW_REM && t != KW_NEXT && t != TK_EOF)
	{
		syntax_error(S, 0);
		while(!S->nl && S->t != TK_EOF)
		{
			spL_next(S);
		}
	}
	return ast;
}


static sp_Ast *sstmt (sp_State *S)
{
	return isstmt(S, NULL);
}


/*
** Parse an full statement
*/
static sp_Ast *istmt (sp_State *S, sp_String *id)
{
	sp_Ast *head = isstmt(S, id);
	if(head == NULL)
		return NULL;
	sp_Ast *tail = head;

	for(;;)
	{
		if(S->t != ':')
			break;
		spL_next(S);
		tail->e = istmt(S, id);
		if(tail->e == NULL)
			break;
		tail = tail->e;
		/*
		if(tail == NULL)
			spL_next(S);
		*/
	}
	head->tail = tail;
	return head;
	// return tail;
}


static sp_Ast *stmt (sp_State *S)
{
	return istmt(S, NULL);
}


/*
** Parse an new-line statement
*/
static sp_Ast *inlstmt (sp_State *S, sp_String *id)
{
	/*
	* nope
	*/
	if(S->t == TK_EOF)
		return NULL;

	match(S, TK_NUMBER);

	uint ln = S->ln;
	sp_Number n = S->n;

	spL_next(S);

	sp_Ast *ast = istmt(S, id);
	if(ast == NULL)
		return NULL;

	if(ast->lbl == 0)
	{
		/* special case for REM */
		ast->lbl = n;
		ast->ln = ln;
	}

	return ast;
}


static sp_Ast *nlstmt (sp_State *S)
{
	return inlstmt(S, NULL);
}


/*
** Parse an new-line statement list
*/
static sp_Ast *inlstmtlist (sp_State *S, sp_String *id)
{
#ifdef SMPL_DEBUG
	if(id)
	{
		spPDO_printinc(S, "Entered for-loop '%s'\n", id->src);
	}
#endif

	if(tk_block_end(S->t))
		return NULL;
	sp_Ast *head = inlstmt(S, id);
	/*
	if(head == NULL)
	{
		return inlstmtlist(S, id);
	}
	*/
	if(head == NULL)
		return NULL;
	// if(1)
	if(id == NULL)
		if(S->t == TK_EOF)
			return head;
	// while(S->t != TK_EOF)

	sp_Ast *tail = head;
	sp_Ast *__head = head;

	for(;;)
	{
		if(id != NULL)
		{
			sp_Ast *_head = tail;
			for(;;)
			{
				if(_head->t == ST_NEXT)
				{
					sp_Ast *idl = _head->a;
					if(idl != NULL)
					{
						if(idlist_includes(S, idl, id))
						{
							spPDO_decprint(S, "Ended for-loop '%s'\n", id->src);
							head->tail = _head;
							return head;
						}
						else
						{
							syntax_tkerror_AB(S, EC_WO, KW_NEXT, KW_FOR);
							head->tail->e = head->tail;
						}
					}
					/*
					*
					* Fix the no id-list NEXT
					*
					*/
					return head;
				}
				else if(_head->t == ST_FOR)
				{
					sp_Ast *next = _head->d->tail;
					if(next->a)
					{
						if(idlist_includes(S, next->a, id))
						{
							head->tail = next;
							spPDO_decprint(S, "Ended for-loop '%s'\n", id->src);
							return head;
						}
					}
					else
					{
						head->tail = next;
						return head;
					}
				}
				if(_head->e == NULL)
				{
					break;
				}
				_head = _head->e;
			}
		}

		__head = __head->tail->e = inlstmt(S, id);

		if(__head == NULL)
		{
			break;
		}

		tail = __head;
	}

	if(id != NULL)
	{
		syntax_tkerror_AB(S, EC_WO, KW_FOR, KW_NEXT);
	}
	return head;
}


static sp_Ast *nlstmtlist (sp_State *S)
{
	return inlstmtlist(S, NULL);
}

static void ast_merge_sort_split (sp_Ast *head, sp_Ast **backref, sp_Ast **frontref)
{
	sp_Ast *slow = head;
	sp_Ast *fast = head->e;

	while(fast != NULL)
	{
		fast = fast->e;
		if(fast != NULL)
		{
			slow = slow->e;
			fast = fast->e;
		}
	}

	*frontref = head;
	*backref = slow->e;
	slow->e = NULL;
}

static sp_Ast *ast_merge_sort_merge (sp_Ast *back, sp_Ast *front)
{
	sp_Ast *result = NULL;

	if(back == NULL)
		return front;
	else if(front == NULL)
		return back;
	
	if(back->lbl <= front->lbl)
	{
		result = back;
		result->e = ast_merge_sort_merge(back->e, front);
	}
	else
	{
		result = front;
		result->e = ast_merge_sort_merge(back, front->e);
	}

	return result;
}

static void ast_merge_sort (sp_State *S, sp_Ast **headref)
{
	sp_Ast *head = *headref;
	sp_Ast *back;
	sp_Ast *front;

	if((head == NULL) || (head->e == NULL))
	{
		return;
	}

	ast_merge_sort_split(head, &back, &front);

	ast_merge_sort(S, &back);
	ast_merge_sort(S, &front);

	*headref = ast_merge_sort_merge(back, front);
}

/*
** Parse the current script source buffer
*/
void spP_parse (sp_State *S)
{
	spL_init(S);

	spDO_printf("========================================================================================\n"
	            "| PARSING\n"
		        "========================================================================================\n");

	S->ast = nlstmtlist(S);

	if(S->ast != NULL)
	{
		while(S->t != TK_EOF)
		{
			sp_Token t = S->t;
			spL_next(S);
			switch(t)
			{
			/* encountered an block-ending token */
			case KW_LOOP:
				bas_35x4x4Px7_stmt_loop(S);
				syntax_tkerror_AB(S, EC_WO, KW_LOOP, KW_DO);
				break;
			case KW_UNTIL:
				bas_stmt_until(S);
				syntax_tkerror_AB(S, EC_WO, KW_UNTIL, KW_REPEAT);
				break;
			case KW_WEND:
				syntax_tkerror_AB(S, EC_WO, KW_WEND, KW_WHILE);
				break;
			}
			// S->ast->e = nlstmtlist(S);
			S->ast->tail->e = nlstmtlist(S);
		}
	}

	/* got syntax errors, stop the compilation */
	if(sp_any_error(S))
	{
		sp_throw(S);
	}

	// ast_merge_sort(S, &S->ast);
}
