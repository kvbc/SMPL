/*
*
* dim.c
* (Dimensional Array)
*
*/

#include "dim.h"
#include "parser.h"


static void free_objs (sp_Dim *dim)
{
    for(uint32_t i = 0; i < dim->len; ++i)
    {
        sp_Object *o = dim->src[i];
        if(o != NULL)
            spO_free(o);
    }
}

sp_Dim *spA_new (sp_State *S)
{
    sp_Dim *dim = spM_alloc(S, sizeof *dim);
    dim->len = 0;
    dim->src = NULL;
    dim->lens = NULL;
    return dim;
}

sp_Dim *spA_new_set (sp_State *S, sp_Object **v, uint32_t len)
{
    return spA_set(S, spA_new(S), v, len);
}

void spA_free (sp_Dim *dim)
{
    free_objs(dim);
    free(dim->src);
    free(dim->lens);
    free(dim);
}

sp_Dim *spA_set (sp_State *S, sp_Dim *dim, sp_Object **v, uint32_t len)
{
    dim->src = v;
    dim->len = len;

	return dim;
}

sp_Dim *spA_set_vsafe (sp_State *S, sp_Dim *dim, sp_Object **v, uint32_t len)
{
    free(dim->src);

    spA_set(S, dim, v, len);
    return dim;
}

sp_Dim *spA_set_vobjsafe (sp_State *S, sp_Dim *dim, sp_Object **v, uint32_t len)
{
    free_objs(dim);
    free(dim->src);

    spA_set(S, dim, v, len);
    return dim;
}

sp_Dim *spA_copy (sp_State *S, sp_Dim *dim, sp_Object **v, uint32_t len)
{
    sp_Object **src = malloc(len * sizeof(*src));
    for(uint32_t i = 0; i < len; i++)
    {
        src[i] = spO_new_copy(S, v[i]);
    }

    spA_set_vobjsafe(S, dim, src, len);
    return dim;
}