/* date = September 1st 2021 6:43 pm */

#ifndef DRONECONTROLLER_H
#define DRONECONTROLLER_H

#include "dc_platform.h"
#include "dc_entity.h"
#include "dc_telemetry.h"
#include "dc_render_commands.h"
#include "dc_memory.h"
#include "dc_ui.h"

#define ENTITY_MAX_COUNT 256

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
};

typedef struct app_backbuffer app_backbuffer;
struct app_backbuffer
{
    void *Data           ;
    s32   Width          ;
    s32   Height         ;
    s32   BytesPerPixel;
};



#define APP_UPDATE( name) void name(platform *Platform, telem_packet_queues *PacketQueues, render_data *RenderData)
typedef APP_UPDATE(app_update);
APP_UPDATE(app_update_stub)
{ return; }


#endif //DRONECONTROLLER_H
