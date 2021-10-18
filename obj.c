/*
*
* obj.c
* (Object)
*
*/


/* SMPL headers */
#include "obj.h"
#include "mem.h"
#include "auxlib.h"

/* C headers */
#include <string.h>
#include <stdlib.h>


/*
** Allocate an new object and initialize it
** return an pointer to it
*/
sp_Object* spO_new (sp_State *S)
{
	sp_Object *o = spM_alloc_sizeof(S, *o);
	obj_settype(o, OT_ANY);
	return o;
}


sp_Object *spO_new_copy (sp_State *S, sp_Object *o)
{
	return spO_copy(S, spO_new(S), o);
}


sp_Object *spO_new_number (sp_State *S, sp_Number n)
{
	sp_Object *o = spO_new(S);
	spO_set_number(o, n);
	return o;
}


sp_Object *spO_new_string (sp_State *S, char *src, uint len)
{
	sp_Object *o = spO_new(S);
	spO_set_string(S, o, src, len);
	return o;
}


sp_Object *spO_new_function (sp_State *S, uint i, sp_Object *arg)
{
	sp_Object *o = spO_new(S);
	spO_set_function(o, i, arg);
	return o;
}


sp_Object *spO_new_CFunction (sp_State *S, sp_CFunction cf)
{
	sp_Object *o = spO_new(S);
	spO_set_CFunction(o, cf);
	return o;
}


sp_Object *spO_new_dim (sp_State *S, sp_Object **v, uint32_t len)
{
	sp_Object *o = spO_new(S);
	spO_set_dim(S, o, v, len);
	return o;
}


/*
** Free the object
*/
void spO_free (sp_Object *o)
{
	spO_free_value(o);
	free(o);
}


/*
** Free object's value
*/
void spO_free_value (sp_Object *o)
{
	if(obj_isstr(o))
	{
		spS_free(obj_str(o));
	}
	else if(obj_isdim(o))
	{
		spA_free(obj_dim(o));
	}
}


/*
** Set object's value to the given number
*/
void spO_set_number (sp_Object *o, sp_Number n)
{
	// printf("1 %p\n", o);
	spO_free_value(o);
	obj_settype(o, OT_NUM);
	obj_setnum(o, n);
}


/*
** Set object's value to the given buffer
*/
void spO_set_string (sp_State *S, sp_Object *o, char *buff, uint len)
{
	if(!obj_isstr(o))
	{
		spO_free_value(o);
		obj_setstr(o, spS_new(S));
	}
	obj_settype(o, OT_STR);
	spS_set(obj_str(o), buff, len);
}


/*
** Set object's value to the given function
*/
void spO_set_function (sp_Object *o, uint i, sp_Object *arg)
{
	spO_free_value(o);
	obj_settype(o, OT_FUNC);
	obj_setfunc(o, to_function(i, arg));
}


/*
** Set object's value to the given C function
*/
void spO_set_CFunction (sp_Object *o, sp_CFunction cf)
{
	spO_free_value(o);
	obj_settype(o, OT_CFUNC);
	obj_setcfunc(o, cf);
}

static void predim (sp_State *S, sp_Object *o, sp_Object **v, uint32_t len)
{
	if(!obj_isdim(o))
	{
		spO_free_value(o);
		obj_setdim(o, spA_new(S));
	}
	obj_settype(o, OT_DIM);
}

void spO_set_dim (sp_State *S, sp_Object *o, sp_Object **v, uint32_t len)
{
	predim(S, o, v, len);
	spA_set_vsafe(S, obj_dim(o), v, len);
}


void spO_copy_dim (sp_State *S, sp_Object *o, sp_Object **v, uint32_t len)
{
	predim(S, o, v, len);
	spA_copy(S, obj_dim(o), v, len);
}


/*
** Copy the value of object 'o2' to the object 'o1'
*/
sp_Object *spO_copy (sp_State *S, sp_Object *o1, sp_Object *o2)
{
	switch(obj_type(o2))
	{
	case OT_NUM:
		spO_set_number(o1, obj_num(o2));
		break;
	case OT_STR:
		spO_set_String(S, o1, obj_str(o2));
		break;
	case OT_FUNC:
		spO_set_function(o1, obj_fi(o2), obj_foarg(o2));
		break;
	case OT_CFUNC:
		spO_set_CFunction(o1, obj_cfunc(o2));
		break;
	case OT_DIM:
		spO_copy_dim(S, o1, obj_dimsrc(o2), obj_dimlen(o2));
		break;
	}
	return o1;
}