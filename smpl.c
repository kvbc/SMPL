/*
*
* smpl.c
*
*/


/*
 *
 * To fix:
 * 
 * [DONE] check correct variable type
 *        new variable type (integer)
 *        read number for INPUT
 *        check variable type for INPUT
 * [DONE] instructions should be evaluated by line numbers (increasingly)
 * [DONE] line numbers not working for EC_TP_MISMATCH?? check err_type_mismatch.bas and compile.c
 */

// avoid hash collisions


/* SMPL headers */
#include "smpl.h"
#include "state.h"
#include "parser.h"

/* C headers */
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>


/*
*
* SMPL standard library
*
*/


#define M_PI 3.14159265358979323846

#define NstdfN0(id, exp)								\
			static void (id) (sp_State *S, uint argn)	\
			{											\
				sp_push_number(S, (exp));				\
			}


#define NstdfN1(id, exp)								\
			static void (id) (sp_State *S, uint argn)	\
			{											\
				sp_Number x = sp_checknum(S);			\
				sp_push_number(S, (exp));				\
			}

#define NstdfN2(id, exp)								\
			static void (id) (sp_State *S, uint argn)	\
			{											\
				sp_Number y = sp_checknum(S);			\
				sp_Number x = sp_checknum(S);			\
				sp_push_number(S, (exp));				\
			}

#define SstdfN1(id, exp)								\
			static void (id) (sp_State *S, uint argn)	\
			{											\
				sp_String *x = sp_checkstr(S);			\
				sp_push_number(S, (exp));				\
			}

#define SNstdfO(id, exp)								\
			static void (id) (sp_State *S, uint argn)	\
			{											\
				sp_Number y = sp_checknum(S);			\
				sp_String *x = sp_checkstr(S);			\
				sp_push(S, (exp));						\
			}

#define stdf(id, exp)									\
			static void (id) (sp_State *S, uint argn)	\
			{											\
				(exp);									\
			}


NstdfN0(PI, M_PI)
NstdfN1(ABS, fabs(x))
NstdfN1(ACS, acos(x))
NstdfN1(ACSH, acosh(x))
NstdfN1(ASN, asin(x))
NstdfN1(ASNH, asinh(x))
NstdfN1(ATN, atan(x))
NstdfN1(ATNH, atanh(x))
NstdfN1(CBRT, cbrt(x))
NstdfN1(CEIL, ceil(x))
NstdfN1(COS, cos(x))
NstdfN1(COSH, cosh(x))
NstdfN1(EXP, exp(x))
NstdfN1(INT, floor(x))
NstdfN1(LOG, log(x))
NstdfN1(SIN, sin(x))
NstdfN1(SINH, sinh(x))
NstdfN1(SQRT, sqrt(x))
NstdfN1(TAN, tan(x))
NstdfN1(TANH, tanh(x))
NstdfN1(RND, rand() % (uint)(x))
NstdfN1(POW, x * x)
SstdfN1(ASC, x->src[0])
NstdfN1(SGN, x < 0 ? -1.0 : 0.0)
SstdfN1(LEN, x->len)
NstdfN2(MIN, x < y ? x : y)
NstdfN2(MAX, x > y ? x : y)
SNstdfO(AT, spO_new_string(S, x->src + (uint)y, 1))
SNstdfO(LEFT, spO_new_string(S, x->src, (uint)y))
SNstdfO(RIGHT, spO_new_string(S, x->src + x->len - (uint)y, (uint)y))
stdf(UCASE, spS_toupper(sp_topstr(S)))
stdf(LCASE, spS_tolower(sp_topstr(S)))

static void VAL (sp_State *S, uint argn)
{
	char *end;
	sp_push_number(S, strtod(sp_checkstr(S)->src, &end));
}

static void MID (sp_State *S, uint argn)
{
	sp_Number z = sp_checknum(S);
	sp_Number y = sp_checknum(S);
	sp_String *x = sp_checkstr(S);
	sp_push_string(S, x->src + (uint)y - 1, (uint)z);
}

static void CHR (sp_State *S, uint argn)
{
	char c = sp_checknum(S);
	sp_push_string(S, &c, 1);
}

static void STR (sp_State *S, uint argn)
{
	char src[50];
	snprintf(src, 50, "%lf", sp_checknum(S));
	sp_push_literal(S, src);
}

static void SPC (sp_State *S, uint argn)
{
	uint n = (uint)sp_checknum(S);
	char *src = malloc(n);
	for(uint i=0;i<n;++i) src[i]=' ';
	sp_push_string(S, src, n);
}

#define loadf(S,fs,fl,f) sp_sset_ocfglobal((S), (fs), (fl), (f))

static void load_stdlib (sp_State *S)
{
	loadf(S, "ABS",  3, ABS);
	loadf(S, "ACS",  3, ACS);
	loadf(S, "ACSH", 4, ACSH);
	loadf(S, "ASN",  3, ASN);
	loadf(S, "ASNH", 4, ASNH);
	loadf(S, "ATN",  3, ATN);
	loadf(S, "ATNH", 4, ATNH);
	loadf(S, "CBRT", 4, CBRT);
	loadf(S, "CEIL", 4, CEIL);
	loadf(S, "COS",  3, COS);
	loadf(S, "COSH", 4, COSH);
	loadf(S, "EXP",  3, EXP);
	loadf(S, "INT",  3, INT);
	loadf(S, "LOG",  3, LOG);
	loadf(S, "SIN",  3, SIN);
	loadf(S, "SINH", 4, SINH);
	loadf(S, "SQRT", 4, SQRT);
	loadf(S, "TAN",  3, TAN);
	loadf(S, "TANH", 4, TANH);
	loadf(S, "PI",   2, PI);
	loadf(S, "RND",  3, RND);
	loadf(S, "MIN",  3, MIN);
	loadf(S, "MAX",  3, MAX);
	loadf(S, "POW",  3, POW);
	loadf(S, "ASC",  3, ASC);
	loadf(S, "SGN",  3, SGN);
	loadf(S, "VAL",  3, VAL);
	loadf(S, "LEN",  3, LEN);
	loadf(S, "AT" ,  2, AT);
	loadf(S, "MID$", 4, MID);
	loadf(S, "CHR$", 4, CHR);
	loadf(S, "STR$", 4, STR);
	loadf(S, "LEFT$",  5, LEFT);
	loadf(S, "RIGHT$", 6, RIGHT);
	loadf(S, "UCASE$", 6, UCASE);
	loadf(S, "LCASE$", 6, LCASE);

	loadf(S, "SPC", 3, SPC);
}


static void print_usage (char *progn)
{
	printf("usage: %s [script]\n", progn);
}


/*
*
* Main program entry
*
*/


int main (int argc, char **argv)
{
	if(argc != 2)
	{
		print_usage(*argv);
		return EXIT_SUCCESS;
	}

	sp_State *S = sp_new_state();
	sp_on_error(S)
		goto cleanup;

	srand(time(NULL));

	load_stdlib(S);
	sp_ErrorCode ec = sp_dofile(S, argv[1]);
	if(ec)
	{
		printf("error: %s\n", spD_errc2str(ec));
		S->status = EXIT_FAILURE;
		goto free;
	}

cleanup:
	spP_free(S);
free:
	sp_free_state(S);

	return S->status;
}
