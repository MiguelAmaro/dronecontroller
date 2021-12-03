/* date = January 8th 2021 4:16 pm */

#ifndef FLIGHTCONTROL_ENTITY_H
#define FLIGHTCONTROL_ENTITY_H

#include "dc_math.h"

typedef enum entity_type entity_type;
enum entity_type
{
    Entity_guage,
};

typedef struct entity entity;
struct entity
{
    u32 Index;
    
    entity_type Type;
    
    v2f32 Dim;
    v2f32 Pos;
    
    u8 HotAlpha;
    v4f32 Color;
    
    u32 RenderInfoID;
};


#endif //FLIGHTCONTROL_ENTITY_H
