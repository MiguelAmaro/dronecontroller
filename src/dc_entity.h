/* date = January 8th 2021 4:16 pm */

#ifndef FLIGHTCONTROL_ENTITY_H
#define FLIGHTCONTROL_ENTITY_H

#include "dc.h"
#include "dc_math.h"

typedef enum entity_type entity_type;
enum entity_type
{
    Entity_guage,
};

typedef struct entity entity;
struct entity
{
    entity_type Type;
    
    v2 Dim;
    v2 Pos;
};


#endif //FLIGHTCONTROL_ENTITY_H
