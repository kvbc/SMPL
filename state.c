/*
*
* state.c
*
*/


/* SMPL headers */
#include "lex.h"
#include "env.h"
#include "state.h"
#include "mem.h"
#include "limits.h"
#include "parser.h"

/* C headers */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/*
** Default SMPL I/O function for printing the number 'n'
*/
void sp_def_printn (sp_State *S, sp_Number n)
{
	printf("%lf", n);
}


/*
** Default SMPL I/O function for printing the string 's'
*/
void sp_def_prints (sp_State *S, char *s)
{
	printf("%s", s);
}


/*
** Default SMPL I/O function for getting an string from the input
*/
void sp_def_gets (sp_State *S)
{
	char *s = spM_alloc(S, 101);
	fgets(s, 101, stdin);
	spS_set(S->s, s, sp_strlen(s));
}


/*
** Free the runtime stack.
*/
static void free_stack (sp_State *S)
{
	sp_pop(S, S->top);
	free(S->stack);
}


/*
** Allocate an new SMPL state and initialize it.
** Return an pointer to it
*/
sp_State *sp_new_state (void)
{
	sp_State *S = malloc(sizeof *S);
	if(S == NULL)
	{
		S->status = EXIT_FAILURE;
		return NULL;
	}

	S->s = spS_new(S);

	/*
	** Initialize the pointers with NULL,
	** so in the case of an "out of memory" error,
	** the state doesn't free an "random" pointer address
	*/
	S->stack  = NULL;
	S->errs   = NULL;
	S->lbls   = NULL;
	S->vars   = NULL;
	S->opargs = NULL;
	S->inss   = NULL;

	S->stack  = spM_alloc(S, sizeof(*S->stack)  * SMPL_STACK_SIZE);
	S->errs   = spM_alloc(S, sizeof(*S->errs)   * SMPL_MAX_ERRORS);
	S->lbls   = spM_alloc(S, sizeof(*S->lbls)   * SMPL_MAX_LBLS);
	S->vars   = spM_alloc(S, sizeof(*S->vars)   * SMPL_MAX_VARIABLES);
	S->opargs = spM_alloc(S, sizeof(*S->opargs) * SMPL_MAX_INSS);
	S->inss   = spM_alloc(S, sizeof(*S->inss)   * SMPL_MAX_INSS);

	for(uint i = 0; i < SMPL_MAX_VARIABLES; i++)
	{
		S->vars[i] = NULL;
	}

	S->top      = 0;
	S->instop   = 0;
	S->errtop   = 0;
	S->lbltop   = 0;
	S->opargtop = 0;

#ifdef SMPL_DEBUG
	S->tabs = 0;
	S->tabc = " |   |   |   |   |   |   |   |   |   |"; /* 10 */
#endif

	sp_set_io(S, sp_def_printn, sp_def_prints, sp_def_gets);

	S->status = EXIT_SUCCESS;

	return S;
}


/*
** Free the SMPL state
*/
void sp_free_state (sp_State *S)
{
	for(uint i = 0; i < S->errtop; ++i)
	{
		sp_Error *e = &S->errs[i];
		printf("%s:%d: error (0x%X): ", S->fn, e->ln, e->ec);
		printf(spD_errc2str(e->ec), e->a, e->b);
		puts("");
	}

	spS_free(S->s);
	spE_free(S);

	free_stack(S);
	free(S->errs);
	free(S->inss);
	free(S->lbls);
	free(S->vars);
	free(S->opargs);

	free(S);
}
