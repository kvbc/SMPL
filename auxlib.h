#ifndef SMPL_AUXLIB_H
#define SMPL_AUXLIB_H

#include "smpl.h"


/*
*
* Syntactic sugar for explicit struct initialization
*
*/

#define to_label(_i,_id)		((sp_Label) {		\
									.i  = (_i),		\
									.id = (_id)})

#define to_function(_i,_arg)	((sp_Function) {	\
									.i   = (_i),	\
									.arg = (_arg)})

#define to_variable(_id,_o)		((sp_Variable) {	\
									.o  = (_o),		\
									.id = (_id)})

#endif // SMPL_AUXLIB_H