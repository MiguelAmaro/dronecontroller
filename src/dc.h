/* date = September 1st 2021 6:43 pm */

#ifndef DRONECONTROLLER_H
#define DRONECONTROLLER_H

#include "dc_platform.h"
#include "dc_entity.h"
#include "dc_memory.h"

#define ENTITY_MAX_COUNT 256
#define UITEXT_MAX_COUNT 256

typedef struct ui_text ui_text;
struct ui_text
{
    u32 EntityID;
    v3f32 Color;
    
    str8 String;
    
    v2f32 Pos;
    
    f32 Scale;
};

typedef struct glyph glyph;
struct glyph
{
    u32   CharIndex;
    u32   TexID  ;
    v2s32 Dim    ;
    v2s32 Bearing;
    u32   Advance;
};

typedef struct glyph_hash glyph_hash;
struct glyph_hash
{
    u32   Count;
    u32   MaxCount;
    u32   CharIndex[256];
    u32   TexID    [256];
    v2s32 Dim      [256];
    v2s32 Bearing  [256];
    u32   Advance  [256];
};

typedef struct app_state app_state;
struct app_state
{
    b32 IsInitialized;
    u8  Throttle;
    f32 DeltaTime;
    
    entity Entities[ENTITY_MAX_COUNT];
    u32    EntityCount;
    u32    EntityMaxCount;
    
    ui_text UIText[UITEXT_MAX_COUNT];
    u32     UITextCount;
    u32     UITextMaxCount;
    
    memory_arena UITextArena;
    
    glyph_hash GlyphHash;
    
};

typedef struct app_backbuffer app_backbuffer;
struct app_backbuffer
{
    void *Data           ;
    s32   Width          ;
    s32   Height         ;
    s32   BytesPerPixel;
};

#include "dc.c"

void App_Init  (platform *Platform_);

b32  App_Update(app_backbuffer *Backbuffer, platform *Platform);


#endif //DRONECONTROLLER_H
