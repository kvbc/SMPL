/*
*
* mem.c
* (Memory)
* Functions to help with memory manipulation.
*
*/


/* SMPL headers */
#include "smpl.h"
#include "error.h"
#include "state.h"

/* C headers */
#include <stdlib.h>


/*
** Allocate an memory buffer with the given size.
** Returns NULL if the given size is equal to 0.
** In case of allocation failure,
** SMPL throws an "Out of memory" error
*/
void *spM_alloc (sp_State *S, uint size)
{
	if(size == 0)
		return NULL;
	void *block = malloc(size);
	if(block == NULL)
		sp_error_throw(S, S->ln, EC_OUT_MEM);
	return block;
}


/*
** If the vector is NOT admissible
** (it's top index is equal to it's capacity)
** then PUSH an error with message 'err' on line 'ln'
*/
uint spM_check_admissible (sp_State *S, uint top, uint cap, uint ln, sp_ErrorCode ec)
{
	if(top == cap)
	{
		sp_error_push(S, ln, ec);
		return 0;
	}
	return 1;
}


/*
** If the vector is NOT admissible
** (it's top index is equal to it's capacity)
** then THROW an error with message 'err' on line 'ln'
*/
void spM_assert_admissible (sp_State *S, uint top, uint cap, uint ln, sp_ErrorCode ec)
{
	if(top == cap)
	{
		sp_error_throw(S, ln, ec);
	}
}
