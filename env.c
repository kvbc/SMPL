/*
*
* env.c
* (Environment)
* Core of global object manipulation.
*
*/


/* SMPL headers */
#include "env.h"
#include "mem.h"
#include "state.h"
#include "limits.h"

/* C headers */
#include <stdlib.h>
#include <string.h>


static uint hash_name (char *name, uint len)
{
	uint hash = 5381;
	char c;

	while(c = *name++)
		hash = ((hash << 5) + hash) + c; // hash * 33 + c

	// printf("hash: %d (i=%d)\n", hash, hash % SMPL_MAX_VARIABLES);

	return hash % SMPL_MAX_VARIABLES;
}

/*
** Free the environment
*/
void spE_free (sp_State *S)
{
	for(uint i = 0; i < SMPL_MAX_VARIABLES; i++)
	{
		if(S->vars[i] != NULL)
		{
			spO_free(S->vars[i]);
		}
	}
}


/*
** Push an new global with the given id of length 'len'
** and return it's object.
*/
sp_Object *spE_global_new (sp_State *S, char *id, uint len)
{
	sp_Object *o = spO_new(S);

	uint hash = hash_name(id, len);

	if(S->vars[hash] != NULL)
	{
		// collision
	}

	S->vars[hash] = o;

	return o;
}


/*
** Get an global with the given id of length 'len'.
** If it doesn't exist then create an new one
** and return its object.
*/
sp_Object *spE_global_ensure (sp_State *S, sp_ObjectType m, char *id, uint len)
{
	sp_Object *o = spE_global_get(S, m, id, len);
	if(o == NULL)
		return spE_global_new(S, id, len);
	return o;
}


/*
*
* Getting global object of basic type
*
*/


/*
** Get an global with the given id of length 'len'
** and object type matching mask 'm'.
** Return it's object.
*/
sp_Object *spE_global_get (sp_State *S, sp_ObjectType m, char *id, uint len)
{
	uint hash = hash_name(id, len);
	sp_Object *o = S->vars[hash];
	if(o)
		if(obj_type(o) & m)
			return o;
	return NULL;
}


/*
** Get an global of string type with id 'vs' of length 'vl'
** and return its object.
*/
sp_Object *spE_global_get_string (sp_State *S, char *vs, uint vl)
{
	return spE_global_get(S, OT_STR, vs, vl);
}


/*
** Get an global of number type with id 'vs' of length 'vl'
** and return its object.
*/
sp_Object *spE_global_get_number (sp_State *S, char *vs, uint vl)
{
	return spE_global_get(S, OT_NUM, vs, vl);
}


/*
** Get an global of function type with id 'vs' of length 'vl'
** and return its object.
*/
sp_Object *spE_global_get_function (sp_State *S, char *vs, uint vl)
{
	return spE_global_get(S, OT_FUNC, vs, vl);
}


/*
** Get an global of C function type with id 'vs' of length 'vl'
** and return its object.
*/
sp_Object *spE_global_get_CFunction (sp_State *S, char *vs, uint vl)
{
	return spE_global_get(S, OT_CFUNC, vs, vl);
}


sp_Object *spE_global_get_dim (sp_State *S, char *vs, uint vl)
{
	return spE_global_get(S, OT_DIM, vs, vl);
}


/*
*
* Setting global objects to basic types
*
*/


/*
** Set an global with id 'vs' of length 'vl'
** and object type matching to the given mask 'm'
** to the given object 'o'.
*/
void spE_global_set (sp_State *S, sp_ObjectType m, char *vs, uint vl, sp_Object *o)
{
	spO_copy(S, spE_global_ensure(S, m, vs, vl), o);
}


/*
** Set an global with id 'vs' of length 'vl'
** and object type matching to the given mask 'm'
** to the given C string of source 'ss' and length 'sl'.
*/
void spE_global_set_string (sp_State *S, sp_ObjectType m, char *vs, uint vl, char *ss, uint sl)
{
	spO_set_string(S, spE_global_ensure(S, m, vs, vl), ss, sl);
}


/*
** Set an global with id 'vs' of length 'vl'
** and object type matching to the given mask 'm'
** to the given number 'n'.
*/
void spE_global_set_number (sp_State *S, sp_ObjectType m, char *vs, uint vl, sp_Number n)
{
	spO_set_number(spE_global_ensure(S, m, vs, vl), n);
}


/*
** Set an global with id 'vs' of length 'vl'
** and object type matching to the given mask 'm'
** to the given function with source 'i' and argument 'arg'.
*/
void spE_global_set_function (sp_State *S, sp_ObjectType m, char *vs, uint vl, uint i, sp_Object *arg)
{
	spO_set_function(spE_global_ensure(S, m, vs, vl), i, arg);
}


/*
** Set an global with id 'vs' of length 'vl'
** and object type matching to the given mask 'm'
** to the given C function 'cf'.
*/
void spE_global_set_CFunction (sp_State *S, sp_ObjectType m, char *vs, uint vl, sp_CFunction cf)
{
	spO_set_CFunction(spE_global_ensure(S, m, vs, vl), cf);
}


void spE_global_set_dim (sp_State *S, sp_ObjectType m, char *vs, uint vl, sp_Object **v, uint32_t len)
{
	spO_set_dim(S, spE_global_ensure(S, m, vs, vl), v, len);
}