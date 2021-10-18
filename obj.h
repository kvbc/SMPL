#ifndef SMPL_OBJ_H
#define SMPL_OBJ_H

#include "str.h"
#include "smpl.h"
#include "dim.h"

#include <stdint.h>


struct sp_Function
{
	uint i;
	sp_Object *arg;
};


/*
** contains 4 basic object types
** and every possible combination of them
*/

/*
enum sp_ObjectType
{
	OT_NUM = 1,	// 0001 > number
	OT_STR,		// 0010 > string
	OT_NS,		// 0011 > string | number
	OT_FUNC,	// 0100 > function
	OT_FN,		// 0101 > function | number
	OT_FS,		// 0110 > function | string
	OT_FNS,		// 0111 > function | string | number
	OT_CFUNC,	// 1000 > C function
	OT_CFN,		// 1001 > C function | number
	OT_CFS,		// 1010 > C function | string
	OT_CFNS,	// 1011 > C function | string | number
	OT_CFF,		// 1100 > C function | function
	OT_CFFN,	// 1101 > C function | function | number
	OT_CFFS,	// 1110 > C function | function | string
	OT_ANY		// 1111 > C function | function | string | number
};
*/

#define OT_NUM   0b00001
#define OT_STR   0b00010
#define OT_FUNC  0b00100
#define OT_CFUNC 0b01000
#define OT_DIM   0b10000
#define OT_ANY   0b11111

/*
** stores object's type and value
*/
struct sp_Object
{
	sp_ObjectType t;
	union
	{
		sp_Number n;
		sp_String *s;
		sp_CFunction cf;
		sp_Function f;
		sp_Dim *dim;
	} u;
};


/* required in debug.c */
#define last_objt OT_ANY


/*
*
* Wrappers for accessing struct members
*
*/


#define obj_type(o)			((o)->t)						/* type of object 'o' */
#define obj_num(o)			((o)->u.n)						/* number of object 'o' */
#define obj_str(o)			((o)->u.s)						/* string of object 'o' */
#define obj_func(o)			((o)->u.f)						/* function of object 'o' */
#define obj_cfunc(o)		((o)->u.cf)						/* C function of object 'o' */
#define obj_dim(o)			((o)->u.dim)					/* array of object 'o' */

#define obj_fi(o)			(obj_func((o)).i)				/* source of function of object 'o' */
#define obj_foarg(o)		(obj_func((o)).arg)				/* argument of function of object 'o' */
#define obj_fsarg(o)		(obj_str(obj_foarg((o))))

#define obj_dimsrc(o)		(obj_dim((o))->src)
#define obj_dimlen(o)		(obj_dim((o))->len)


/*
*
* Wrappers for basic object type checking
*
*/


#define obj_isnum(o)		(obj_type((o)) == OT_NUM)		/* is object an number */
#define obj_isstr(o)		(obj_type((o)) == OT_STR)		/* is object an string */
#define obj_isfunc(o)		(obj_type((o)) == OT_FUNC)		/* is object an function */
#define obj_iscfunc(o)		(obj_type((o)) == OT_CFUNC)		/* is object an C function */
#define obj_isdim(o)		(obj_type((o)) == OT_DIM)		/* is object an array */


/*
*
* Wrappers for setting struct members
*
*/


#define obj_settype(o,t)	(obj_type ((o)) = (t))			/* set object's type */
#define obj_setnum(o,n)		(obj_num  ((o)) = (n))			/* set object's numeric value */
#define obj_setstr(o,s)		(obj_str  ((o)) = (s))			/* set object's string value */
#define obj_setfunc(o,f)	(obj_func ((o)) = (f))			/* set object's function value */
#define obj_setcfunc(o,cf)	(obj_cfunc((o)) = (cf))			/* set object's C function value */
#define obj_setdim(o,dim)	(obj_dim  ((o)) = (dim))


/*
*
* Wrappers for function calls
*
*/


#define spO_set_String(S,o,s) spO_set_string((S), (o), (s)->src, (s)->len)


#endif // SMPL_OBJ_H