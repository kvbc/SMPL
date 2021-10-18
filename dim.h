#ifndef SMPL_DIM_H
#define SMPL_DIM_H

#include "smpl.h"
#include <stdint.h>


struct sp_Dim
{
    uint32_t len;
    uint32_t *lens;
    sp_Object **src;
    struct sp_Dim *next;
};


#endif /* SMPL_DIM_H */