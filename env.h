#ifndef SMPL_ENV_H
#define SMPL_ENV_H

#include "obj.h"
#include "str.h"


struct sp_Variable
{
	sp_Object *o;
	sp_String *id;
};


#define spE_global_Snew(S,s) (spE_global_new((S), (s)->src, (s)->len))

/*
*
* Wrappers for getting global objects
*
*/


/*
** given an C string and a mask
*/
#define sp_smget_oglobal(S,m,vs,vl)				(spE_global_get((S), (m), (vs), (vl)))		/* get an global object given its id 'vs' of length 'vl' and type mask 'm' */

/*
** given an SMPL string and a mask
*/
#define sp_Smget_oglobal(S,m,v)					sp_smget_oglobal((S), (m), (v)->src, (v)->len)	/* get an global object given its id 'v' as an SMPL string and type mask 'm' */

/*
** given an C string
*/
#define sp_sget_oglobal(S,vs,vl)				sp_smget_oglobal((S), OT_ANY,   (vs), (vl))		/* get an global object of any type given its id 'vs' of length 'vl' */
#define sp_sget_onglobal(S,vs,vl)				sp_smget_oglobal((S), OT_NUM,   (vs), (vl))		/* get an global object of number type given its id 'vs' of length 'vl' */
#define sp_sget_osglobal(S,vs,vl)				sp_smget_oglobal((S), OT_STR,   (vs), (vl))		/* get an global object of string type given its id 'vs' of length 'vl' */
#define sp_sget_ofglobal(S,vs,vl)				sp_smget_oglobal((S), OT_FUNC,  (vs), (vl))		/* get an global object of function type given its id 'vs' of length 'vl' */
#define sp_sget_ocfglobal(S,vs,vl)				sp_smget_oglobal((S), OT_CFUNC, (vs), (vl))		/* get an global object of C function type given its id 'vs' of length 'vl' */
#define sp_sget_odglobal(S,vs,vl)				sp_smget_oglobal((S), OT_DIM,	(vs), (vl))

/*
** given an literal
*/
#define sp_lget_oglobal(S,v)					sp_sget_oglobal  ((S), (v), sp_strlen((v)))		/* get an global object of any type given its literal id 'v' */
#define sp_lget_onglobal(S,v)					sp_sget_onglobal ((S), (v), sp_strlen((v)))		/* get an global object of number type given its literal id 'v' */
#define sp_lget_osglobal(S,v)					sp_sget_osglobal ((S), (v), sp_strlen((v)))		/* get an global object of string type given its literal id 'v' */
#define sp_lget_ofglobal(S,v)					sp_sget_ofglobal ((S), (v), sp_strlen((v)))		/* get an global object of function type given its literal id 'v' */
#define sp_lget_ocfglobal(S,v)					sp_sget_ocfglobal((S), (v), sp_strlen((v)))		/* get an global object of C function type given its literal id 'v' */
#define sp_lget_odglobal(S,v)					sp_sget_odglobal ((S), (v), sp_strlen((v)))

/*
** given an SMPL string
*/
#define sp_Sget_oglobal(S,v)					sp_sget_oglobal  ((S), (v)->src, (v)->len)		/* get an global object of any type given its id 'v' as an SMPL string */
#define sp_Sget_onglobal(S,v)					sp_sget_onglobal ((S), (v)->src, (v)->len)		/* get an global object of number type given its id 'v' as an SMPL string */
#define sp_Sget_osglobal(S,v)					sp_sget_osglobal ((S), (v)->src, (v)->len)		/* get an global object of string type given its id 'v' as an SMPL string */
#define sp_Sget_ofglobal(S,v)					sp_sget_ofglobal ((S), (v)->src, (v)->len)		/* get an global object of function type given its id 'v' as an SMPL string */
#define sp_Sget_ocfglobal(S,v)					sp_sget_ocfglobal((S), (v)->src, (v)->len)		/* get an global object of C function type given its id 'v' as an SMPL string */
#define sp_Sget_odglobal(S,v)					sp_sget_odglobal ((S), (v)->src, (v)->len)

/*
*
* Wrappers for setting global objects
*
*/


/*
** given an C string and a mask
*/
#define sp_smset_oglobal(S,m,vs,vl,o)			(spE_global_set				((S), (m), (vs), (vl), (o)))				/* set an global with the given id 'vs' of length 'vl' and type mask matching to mask 'm' to the given object 'o' */
#define sp_smset_onglobal(S,m,vs,vl,n)			(spE_global_set_number		((S), (m), (vs), (vl), (n)))				/* set an global with the given id 'vs' of length 'vl' and type mask matching to mask 'm' to the given number 'n' */
#define sp_smset_osglobal(S,m,vs,vl,ss,sl)		(spE_global_set_string		((S), (m), (vs), (vl), (ss), (sl)))			/* set an global with the given id 'vs' of length 'vl' and type mask matching to mask 'm' to the given string 'ss' of length 'sl' */
#define sp_smset_ofglobal(S,m,vs,vl,i,arg)		(spE_global_set_function	((S), (m), (vs), (vl), (i), (arg)))			/* set an global with the given id 'vs' of length 'vl' and type mask matching to mask 'm' to a function with source 'i' and argument 'arg' */
#define sp_smset_ocfglobal(S,m,vs,vl,cf)		(spE_global_set_CFunction	((S), (m), (vs), (vl), (cf)))				/* set an global with the given id 'vs' of length 'vl' and type mask matching to mask 'm' to the given C function 'cf' */
#define sp_smset_odglobal(S,m,vs,vl,v,l)		(spE_global_set_dim			((S), (m), (vs), (vl), (v), (l)))

/*
** given an SMPL string and a mask
*/
#define sp_Smset_oglobal(S,m,id,o)				sp_smset_oglobal  ((S), (m), (id)->src, (id)->len, (o))					/* set an global with the given id 'v' being an SMPL string and type mask matching to mask 'm' to the given object 'o' */
#define sp_Smset_onglobal(S,m,id,n)				sp_smset_onglobal ((S), (m), (id)->src, (id)->len, (n))					/* set an global with the given id 'v' being an SMPL string and type mask matching to mask 'm' to the given number 'n' */
#define sp_Smset_osglobal(S,m,id,ss,sl)			sp_smset_osglobal ((S), (m), (id)->src, (id)->len, (ss), (sl))			/* set an global with the given id 'v' being an SMPL string and type mask matching to mask 'm' to the given string 'ss' of length 'sl' */
#define sp_Smset_ofglobal(S,m,id,i,arg)			sp_smset_ofglobal ((S), (m), (id)->src, (id)->len, (i), (arg))			/* set an global with the given id 'v' being an SMPL string and type mask matching to mask 'm' to a function with source 'i' and argument 'arg' */
#define sp_Smset_ocfglobal(S,m,id,cf)			sp_smset_ocfglobal((S), (m), (id)->src, (id)->len, (cf))				/* set an global with the given id 'v' being an SMPL string and type mask matching to mask 'm' to the given C function 'cf' */
#define sp_Smset_odglobal(S,m,id,v,l)			sp_smset_odglobal ((S), (m), (id)->src, (id)->len, (v), (l))

#define sp_Smset_olglobal(S,m,v,s)				sp_Smset_osglobal((S), (m), (v), (s), sp_strlen((s)))					/* set an global with the given id 'v' being an SMPL string and type mask matching to mask 'm' to the given literal 's' */

/*
** given an literal and a mask
*/
#define sp_lmset_oglobal(S,m,id,o)				sp_smset_oglobal  ((S), (m), (id), sp_strlen((id)), (o))					/* set an global with the given literal id 'v' and type mask matching to mask 'm' to the given object 'o' */
#define sp_lmset_onglobal(S,m,id,n)				sp_smset_onglobal ((S), (m), (id), sp_strlen((id)), (n))					/* set an global with the given literal id 'v' and type mask matching to mask 'm' to the given number 'n' */
#define sp_lmset_osglobal(S,m,id,ss,sl)			sp_smset_osglobal ((S), (m), (id), sp_strlen((id)), (ss), (sl))			/* set an global with the given literal id 'v' and type mask matching to mask 'm' to the given string 'ss' of length 'sl' */
#define sp_lmset_ofglobal(S,m,id,i,arg)			sp_smset_ofglobal ((S), (m), (id), sp_strlen((id)), (i), (arg))			/* set an global with the given literal id 'v' and type mask matching to mask 'm' to a function with source 'i' and argument 'arg' */
#define sp_lmset_ocfglobal(S,m,id,cf)			sp_smset_ocfglobal((S), (m), (id), sp_strlen((id)), (cf))					/* set an global with the given literal id 'v' and type mask matching to mask 'm' to the given C function 'cf' */
#define sp_lmset_odglobal(S,m,id,v,l)			sp_smset_odglobal ((S), (m), (id), sp_strlen((id)), (v), (l))

#define sp_lmset_olglobal(S,m,v,s)				sp_smset_osglobal ((S), (m), (v), sp_strlen((v)), (s), sp_strlen((s)))	/* set an global with the given literal id 'v' and type mask matching to mask 'm' to the given literal 's' */

/*
** given an C string
*/
#define sp_sset_oglobal(S,vs,vl,o)				sp_smset_oglobal  ((S), OT_ANY, (vs), (vl), (o))						/* set an global with the given id 'vs' of length 'vl' and any object type to the given object 'o' */
#define sp_sset_onglobal(S,vs,vl,n)				sp_smset_onglobal ((S), OT_ANY, (vs), (vl), (n))						/* set an global with the given id 'vs' of length 'vl' and number object type to the given number 'n' */
#define sp_sset_osglobal(S,vs,vl,ss,sl)			sp_smset_osglobal ((S), OT_ANY, (vs), (vl), (ss), (sl))					/* set an global with the given id 'vs' of length 'vl' and string object type to the given string 'ss' of length 'vl' */
#define sp_sset_ofglobal(S,vs,vl,i,arg)			sp_smset_ofglobal ((S), OT_ANY, (vs), (vl), (i), (arg))					/* set an global with the given id 'vs' of length 'vl' and function object type to a function with source 'i' and argument 'arg' */
#define sp_sset_ocfglobal(S,vs,vl,cf)			sp_smset_ocfglobal((S), OT_ANY, (vs), (vl), (cf))						/* set an global with the given id 'vs' of length 'vl' and C function object type to the given C function 'cf' */
#define sp_sset_odglobal(S,vs,vl,v,l)			sp_smset_odglobal ((S), OT_ANY, (vs), (vl), (v), (l))

/*
** given an SMPL string
*/
#define sp_Sset_oglobal(S,id,o)					sp_sset_oglobal  ((S), (id)->src, (id)->len, (o))							/* set an global with the given id 'v' being an SMPL string, to the given object 'o' */
#define sp_Sset_onglobal(S,id,n)				sp_sset_onglobal ((S), (id)->src, (id)->len, (n))							/* set an global with the given id 'v' being an SMPL string, to the given number 'n' */
#define sp_Sset_osglobal(S,id,ss,sl)			sp_sset_osglobal ((S), (id)->src, (id)->len, (ss), (sl))					/* set an global with the given id 'v' being an SMPL string, to the given string 'ss' of length 'sl' */
#define sp_Sset_ofglobal(S,id,i,arg)			sp_sset_ofglobal ((S), (id)->src, (id)->len, (i), (arg))					/* set an global with the given id 'v' being an SMPL string, to a function with source 'i' and argument 'arg' */
#define sp_Sset_ocfglobal(S,id,cf)				sp_sset_ocfglobal((S), (id)->src, (id)->len, (cf))						/* set an global with the given id 'v' being an SMPL string, to the given C function 'cf' */
#define sp_Sset_odglobal(S,id,v,l)				sp_sset_odglobal ((S), (id)->src, (id)->len, (v), (l))

#define sp_Sset_olglobal(S,v,s)					sp_Sset_osglobal((S), (v), (s), sp_strlen((s)))							/* set an global with the given id 'v' being an SMPL string, to the given literal 's' */

/*
** given an literal
*/
#define sp_lset_oglobal(S,id,o)					sp_sset_oglobal  ((S), (id), sp_strlen((id)), (o))						/* set an global with the given literal id 'v' and any object type to the given object 'o' */
#define sp_lset_onglobal(S,id,n)				sp_sset_onglobal ((S), (id), sp_strlen((id)), (n))						/* set an global with the given literal id 'v' and number object type to the given number 'n' */
#define sp_lset_osglobal(S,id,ss,sl)			sp_sset_osglobal ((S), (id), sp_strlen((id)), (ss), (sl))					/* set an global with the given literal id 'v' and string object type to the given string 'ss' of length 'vl' */
#define sp_lset_ofglobal(S,id,i,arg)			sp_sset_ofglobal ((S), (id), sp_strlen((id)), (i), (arg))					/* set an global with the given literal id 'v' and function object type to a function with source 'i' and argument 'arg' */
#define sp_lset_ocfglobal(S,id,cf)				sp_sset_ocfglobal((S), (id), sp_strlen((id)), (cf))						/* set an global with the given literal id 'v' and C function object type to the given C function 'cf' */
#define sp_lset_odglobal(S,id,v,l)				sp_sset_odglobal ((S), (id), sp_strlen((id)), (v), (l))

#define sp_lset_olglobal(S,v,s)					sp_lset_osglobal((S), (v), (s), sp_strlen((s)))							/* set an global with the given literal id 'v' and string object type to the given literal 's' */


#endif // SMPL_ENV_H