#ifndef SMPL_LEX_H
#define SMPL_LEX_H

#include <stdbool.h>
#include <limits.h>


#define FIRST_TK UCHAR_MAX + 1


enum sp_Token
{
	/* miscellaneous */
	TK_IDENT = FIRST_TK,
	TK_STRING,
	TK_NUMBER,
	TK_NAME,
	TK_EOF,

	/* relational operators */
	TK_LT, TK_LE,
	TK_GT, TK_GE,
	TK_NE, TK_EQ,

	/* keywords */
	KW_AND, // done [TESTED]
	KW_DEF, // done [TESTED]
	KW_DIM, // done [TESTED]
	KW_DO,
	KW_ELSE, // done
	KW_END,
	KW_FN,
	KW_FOR,
	KW_GOSUB,
	KW_GOTO,
	KW_IF, // done
	KW_INPUT, // WIP
	KW_LET, // done
	KW_LOOP,
	KW_NEXT,
	KW_NOT, // done [TESTED]
	KW_ON,
	KW_OR, // done [TESTED]
	KW_PRINT, // done [TESTED]
	KW_REM, // done [TESTED]
	KW_REPEAT,
	KW_RETURN,
	KW_STEP,
	KW_THEN, // done
	KW_TO,
	KW_UNTIL,
	KW_WEND,
	KW_WHILE

	/*
	// Standard numerical functions
	FN_ABS,
	FN_ACS, FN_ACSH,
	FN_ASN, FN_ASNH,
	FN_ATN, FN_ATNH,
	FN_CBRT,
	FN_CEIL,
	FN_COS, FN_COSH,
	FN_EXP,
	FN_INT,
	FN_LOG,
	FN_SIN, FN_SINH,
	FN_SQRT,
	FN_TAN, FN_TANH,

	// Custom numerical functions
	FN_PI,								// Returns the ratio of the circumference of a circle to its diameter
	FN_RND,								// Returns a pseudo-random number in the range of 0 (inclusive) to N (exclusive)
	FN_MIN,								// Returns the lowest-valued number of two passed parameters
	FN_MAX,								// Returns the highest-valued number of two passed parameters
	FN_POW,								// Returns the base X to the exponent Y that is X^Y
	FN_ASC,								// Returns the first character of string S mapped to the numeric index of the ASCII-Table
	FN_SGN,								// Returns either a positive or negative indicating the sign of number X. Exclusively returns 0 if the number X is equal to 0
	FN_VAL,								// Finds a numerical value in string S and converts it into a number
	FN_LEN,								// Returns the length of string S

	// String functions
	FN_AT,
	FN_MID,								// Returns part of the string S starting inclusively from 1-based index X with the length of Y
	FN_REP,								// Returns N copies of string S concatenated together
	FN_CHR,
	FN_STR,
	FN_LEFT,
	FN_RIGHT,
	FN_UCASE,
	FN_LCASE
	*/
};


#define first_kw_tk		KW_AND							/* first keyword token */
#define last_kw_tk		KW_WHILE						/* last keyword token */
#define kw_count		(last_kw_tk - first_kw_tk + 1) 	/* keyword count */


#endif // SMPL_LEX_H