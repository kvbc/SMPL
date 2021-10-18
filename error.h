#ifndef SMPL_ERROR_H
#define SMPL_ERROR_H


#include "auxlib.h"


/*
*
* Error codes are 32 bit unsigned integers.
* All error codes have their code in the lowest 6 bits
* and 2 arguments (tokens) stored in the highest 26 bits.
*
* Unsigned integer (32)
* -------------------------------------------------------------------------
* | 0xFFF80000                | 0x7FFC0                       | 0x3F      |
* | 2 2 2 2 2 2 2 2 2 2 2 2 2 | 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 | 0 0 0 0 0 |
* | arg1 (13)                 | arg2 (13)                     | code (6)  |
* -------------------------------------------------------------------------
* | 0xFFFFFFC0                                                |
* | 0 0 0 0 0 0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
* | arg (26)                                                  |
* -------------------------------------------------------------
*
*/


enum sp_ErrorCode
{
	/* none */
	EC_NONE,

	/* max */
	EC_MAX_ERRS,		/* errors */
	EC_MAX_VARS,		/* variables */
	EC_MAX_STACK,		/* stack */
	EC_MAX_LINES,		/* lines */
	EC_MAX_ID,			/* identifier length */
	EC_MAX_INS,			/* instructions */
	EC_MAX_ARGS,		/* opcode arguments */
	EC_MAX_LBLS,		/* labels */

	/* expected */
	EC_EXP_SB,			/* symbol */				/* ARG */
	EC_EXP_SBS,			/* symbols */				/* ARG1, ARG2 */
	EC_EXP_NUM,			/* number */
	EC_EXP_STR,			/* string */
	EC_EXP_CF,			/* C function */

	/* unexpected */
	EC_UNEXP_SB,		/* symbol */				/* ARG */

	/* undefined */
	EC_UND,
	
	/* type mismatch */
	EC_TP_MISMATCH,
	EC_ARG_TP_MISMATCH,

	/* unbalanced */
	EC_UNB_PAREN,		/* parenthesis */

	/* unterminated */
	EC_UNT_STR,			/* string */

	/* uncallable */
	EC_UNC_OBJ,			/* object */

	/* malformed */
	EC_MAL_NUM,			/* number */

	/* cannot */
	EC_CNT_F_RD,		/* read file */
	EC_CNT_F_OP,		/* open file */
	EC_CNT_F_SK,		/* seek in file */
	EC_CNT_F_TL,		/* tell in file */

	/* out of */
	EC_OUT_MEM,			/* memory */

	/* without */
	EC_WO				/* ARG1, ARG2 */
}; /* < 64 (2^6) */


struct sp_Error
{
	char *a;
	char *b;
	uint ln;
	sp_ErrorCode ec;
};


#define ERR(_ln,_ec,_a,_b)	(sp_Error)			\
							{					\
								.a  = (_a),		\
								.b  = (_b),		\
								.ln = (_ln),	\
								.ec = (_ec)		\
							}


#endif // SMPL_ERROR_H
