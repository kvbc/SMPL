#ifndef SMPL_API_H
#define SMPL_API_H


#define sp_push_String(S,s) sp_push_string((S), (s)->src, (s)->len)


/*
*
* Wrappers for getting globals
*
*/


/*
** given an SMPL string and a mask
*/
#define sp_Smget_global(S,m,s)		(sp_mget_global((S), (m), (s)->src, (s)->len))

/*
** given an C string
*/
#define sp_sget_global(S,s,l)		(sp_mget_global((S), OT_ANY,   (s), (l)))
#define sp_sget_sglobal(S,s,l)		(sp_mget_global((S), OT_STR,   (s), (l)))
#define sp_sget_nglobal(S,s,l)		(sp_mget_global((S), OT_NUM,   (s), (l)))
#define sp_sget_fglobal(S,s,l)		(sp_mget_global((S), OT_FUNC,  (s), (l)))
#define sp_sget_cfglobal(S,s,l)		(sp_mget_global((S), OT_CFUNC, (s), (l)))

/*
** given an literal
*/
#define sp_lget_global(S,s)			sp_sget_global  ((S), (s), sp_strlen((s)))
#define sp_lget_sglobal(S,s)		sp_sget_sglobal ((S), (s), sp_strlen((s)))
#define sp_lget_nglobal(S,s)		sp_sget_nglobal ((S), (s), sp_strlen((s)))
#define sp_lget_fglobal(S,s)		sp_sget_fglobal ((S), (s), sp_strlen((s)))
#define sp_lget_cfglobal(S,s)		sp_sget_cfglobal((S), (s), sp_strlen((s)))

/*
** given an SMPL string
*/
#define sp_Sget_global(S,s)			sp_sget_global  ((S), (s)->src, (s)->len)	
#define sp_Sget_sglobal(S,s)		sp_sget_sglobal ((S), (s)->src, (s)->len)	
#define sp_Sget_nglobal(S,s)		sp_sget_nglobal ((S), (s)->src, (s)->len)	
#define sp_Sget_fglobal(S,s)		sp_sget_fglobal ((S), (s)->src, (s)->len)	
#define sp_Sget_cfglobal(S,s)		sp_sget_cfglobal((S), (s)->src, (s)->len)	


/*
*
* Wrappers for setting globals
*
*/


/*
** given an literal
*/
#define sp_lset_global(S,s)			(sp_set_global((S), (s), sp_strlen((s))))

/*
** given an SMPL string
*/
#define sp_Sset_global(S,s)			(sp_set_global((S), (s)->src, (s)->len))


#endif // SMPL_API_H