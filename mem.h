#ifndef SMPL_MEM_H
#define SMPL_MEM_H


/*
*
* Vector operations
*
*/


#define spM_vector_push(v,t,i)	((v)[(t)++] = (i))	/* push an item 'i' into the vector 'v' with it's top index being 't' */ 
#define spM_vector_pop(v,t)		((v)[--(t)])		/* pop the top item from vector 'v' with it's top index being 't' */

/*
** Check if an vector 'v' with top 't' and capacity 'c' is admissible.
** If it is then push an item 'i' into it,
** otherwise PUSH an error on line 'ln' with error code 'ec'
*/
#define spM_check_push(S,v,t,c,ln,ec,i) \
			if(spM_check_admissible((S), (t), (c), (ln), (ec))) \
				spM_vector_push((v), (t), (i))

/*
** Check if an vector 't' with top 't' and capacity 'c' is admissible.
** If it is then push an item 'i' into it,
** otherwise THROW an error on line 'ln' with error code 'ec'
*/
#define spM_assert_push(S,v,t,c,ln,ec,i) \
			(spM_check_admissible((S), (t), (c), (ln), (ec)), \
			 spM_vector_push((v), (t), (i)))


/*
*
* Allocation helpers
*
*/


#define spM_alloc_sizeof(S,e)	(spM_alloc((S), sizeof (e))) /* allocate memory for the given element 'e' */


#endif // SMPL_MEM_H