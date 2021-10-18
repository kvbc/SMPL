/*
*
* lex.c
* (Lexer)
*
*/


/* SMPL headers */
#include "lex.h"
#include "str.h"
#include "smpl.h"
#include "state.h"

/* C headers */
#include <math.h>
#include <ctype.h>
#include <string.h>


/*
*
* Common defines
*
*/


#define next(S)				spL_nextc(S)

#define is_nl(c)			((c)=='\n' || (c)=='\r')
#define is_ident_end(c)		((c)=='$' || (c)=='%')

#define lex_error(S,err)	(sp_error_throw((S), (S)->ln, (err)))


/*
**
** BASIC keywords.
** Sorted in alphabetical order to allow binary search.
**
*/
static char *keywords[] =
{
	"AND", "DEF", "DIM", "DO", "ELSE",
	"END", "FN", "FOR", "GOSUB", "GOTO",
	"IF", "INPUT", "LET", "LOOP", "NEXT",
	"NOT", "ON", "OR", "PRINT", "REM",
	"REPEAT", "RETURN", "STEP", "THEN", "TO",
	"UNTIL", "WEND", "WHILE"
};


/*
** Initialize the lexer.
*/
void spL_init (sp_State *S)
{
	S->ln = 1;
	S->nl = false;
	next(S);
	spL_next(S);
}


/*
** Advance the lexer.
*/
void spL_nextc (sp_State *S)
{
	if(S->p == S->e)
		S->c = -1;
	else
		S->c = *(S->p++);
}


/*
** Binary search for the current lexeme in the keywords array.
** If found, returns the index of it, otherwise returns -1.
*/
static int get_keyword (sp_State *S)
{
	int l = 0;
	int r = kw_count - 1;
	while(l <= r)
	{
		int m = (l + r) >> 1;
		int c = strcmp(S->s->src, keywords[m]);
		if(c < 0)
			r = m - 1;
		else if(c > 0)
			l = m + 1;
		else
			return m;
	}
	return -1;
}


#define to_hex(c) (((c) & 0xf) + ((c) >> 6) + (((c) >> 6) << 3))

/*
** Lex an stream of hexadecimal characters and convert it into an integer
*/
static sp_Number lex_hexadecimal (sp_State *S)
{
	sp_Number n = 0;
	do
	{
		n = n * 16 + to_hex(S->c);
		next(S);
	}
	while(isxdigit(S->c));
	return n;
}


#define to_dec(c) ((c)-'0')

/*
** Lex an stream of decimal digits and convert it into an integer
*/
static sp_Number lex_integer (sp_State *S)
{
	sp_Number n = 0;
	do
	{
		n = n * 10 + to_dec(S->c);
		next(S);
	}
	while(isdigit(S->c));
	return n;
}


/*
** Lex an integer and divide it by 10 to the power of number of digits
*/
static sp_Number lex_fraction (sp_State *S)
{
	sp_Number i = 0;
	sp_Number d = 1;
	do
	{
		i = i * 10 + to_dec(S->c);
		d *= 10;
		next(S);
	}
	while(isdigit(S->c));
	return i / d;
}


/*
** Lex an exponent
*/
static sp_Number lex_exponent (sp_State *S)
{
	if(S->c == 'e' || S->c == 'E')
	{
		next(S);
		int sign = 1;
		if(S->c == '-')
		{
			sign = -1;
			next(S);
		}
		else if(S->c == '+')
			next(S);
		return sign * lex_integer(S);
	}
	return 0;
}


/*
** Lex an number
*/
static sp_Token lex_number (sp_State *S)
{
	sp_Number n;

	/* 0, 0.123, hexadecimal */
	if(S->c == '0')
	{
		next(S);
		if(S->c == 'x' || S->c == 'X')
		{
			/* hexadecimal */
			S->n = n;
			return TK_NUMBER;
		}
		n = 0;
		if(S->c == '.')
		{
			next(S);
			n += lex_fraction(S);
		}
	}
	else if(S->c == '.') /* .123 */
	{
		next(S);
		if(!isdigit(S->c))
			return '.';
		n = lex_fraction(S);
	}
	else /* 123, 123.123 */
	{
		n = lex_integer(S);
		if(S->c == '.')
			n += lex_fraction(S);
	}

	/* lex an exponent */
	sp_Number e = lex_exponent(S);
	if(e < 0)
		n /= pow(10, -e);
	else if(e > 0)
		n *= pow(10, e);

	/* malformed number */
	if(isalpha(S->c))
		lex_error(S, EC_MAL_NUM);

	S->n = n;
	return TK_NUMBER;
}


/*
** Lex an string
*/
static void lex_string (sp_State *S)
{
	next(S);
	char *b = S->p - 1;
	do
	{
		next(S);
		if(S->c == TK_EOF || S->c == '\n')
			lex_error(S, EC_UNT_STR);
	}
	while(S->c != '"');
	spS_set(S->s, b, S->p - b - 1);
	next(S);
}


/*
** Lex an name
*/
static sp_Token lex_name (sp_State *S)
{
	char *b = S->p;
	do next(S);
	while(isalpha(S->c));

	uint32_t len = S->p - b + (S->c < 0);
	sp_Token t = TK_NAME;

	if(len == 1)
	{
		if(is_ident_end(S->c))
			next(S), ++len;
		t = TK_IDENT;
	}
	else if(S->c == '$')
		next(S), ++len;
	
	if(b + UINT_MAX < S->p)
		lex_error(S, EC_MAX_ID);

	spS_set(S->s, b - 1, len);
	return t;
}


/*
** Advance to the next line
*/
static void next_line (sp_State *S)
{
	if(++S->ln == UINT_MAX)
		lex_error(S, EC_MAX_LINES);
	char o = S->c;
	next(S);
	if(is_nl(S->c) && S->c != o)
		next(S);
	S->nl = true;
}


/*
** Lex the next token
*/
static sp_Token lex (sp_State *S)
{
	S->nl = false;
	for(;;)
	{
		switch(S->c)
		{
		case '\n': case '\r':
			next_line(S);
			break;
		case ' ': case '\f': case '\t': case '\v':
			next(S);
			break;
		case '<':
			next(S);
			if(S->c == '>')
				return next(S), TK_NE;
			else if(S->c == '=')
				return next(S), TK_LE;
			return TK_LT;
		case '>':
			next(S);
			if(S->c == '=')
				return next(S), TK_GE;
			return TK_GT;
		case '=':
			next(S);
			return TK_EQ;
		case '"':
			lex_string(S);
			return TK_STRING;
		case '.':
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			return lex_number(S);
		case -1:
			return TK_EOF;
		default:
			if(isalpha(S->c))
			{
				sp_Token t = lex_name(S);
				if(t == TK_NAME)
				{
					int kw = get_keyword(S);
					if(kw >= 0)
						return first_kw_tk + kw;
				}
				return t;
			}
			else
			{
				char c = S->c;
				next(S);
				return c;
			}
		}
	}
}


/*
** Set the current token in state 'S'
** to the next lexed token
*/
void spL_next(sp_State *S)
{
	S->t = lex(S);
}