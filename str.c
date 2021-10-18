/*
*
* str.c
* (String)
*
*/


/* SMPL headers */
#include "str.h"

/* C headers */
#include <ctype.h>
#include <string.h>
#include <stdlib.h>


/*
** Allocate an new string and initialize it
** return an pointer to it
*/
sp_String* spS_new (sp_State *S)
{
	sp_String *s = spM_alloc(S, sizeof *s);
	s->src = NULL;
	s->len = 0;
	return s;
}


sp_String *spS_new_set (sp_State *S, char *buff, uint len)
{
	return spS_set(spS_new(S), buff, len);
}


/*
** Free the string
*/
void spS_free (sp_String *s)
{
	free(s->src);
}


/*
** Set an string to the given buffer
*/
sp_String* spS_set (sp_String *s, char *buff, uint len)
{
	if(len > s->len)
	{
		free(s->src);
		s->src = malloc(len + 1);
	}

	s->len = len;
	memcpy(s->src, buff, len);
	s->src[len] = '\0';

	return s;
}


/*
** Check if an string is equal to the given buffer
*/
bool spS_equal (sp_String *s, char *buff, uint len)
{
	if(s->len != len)
		return false;
	return strncmp(s->src, buff, len) == 0;
}


void spS_toupper (sp_String *s)
{
	for(uint i = 0; i < s->len; ++i)
	{
		s->src[i] = toupper(s->src[i]);
	}
}


void spS_tolower (sp_String *s)
{
	for(uint i = 0; i < s->len; ++i)
	{
		s->src[i] = tolower(s->src[i]);
	}
}