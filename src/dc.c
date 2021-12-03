
#include "dc.h"

#include "dc_ui.h"
#include "dc_math.h"
#include "dc_strings.h"
#include "dc_entity.h"
#include <stdio.h>
#include <math.h>


internaldef
u32 Entity_Create(app_state *AppState, v2f32 Pos , v2f32 Dim, entity_type Type)
{
    ASSERT(AppState->EntityCount < ENTITY_MAX_COUNT);
    
    entity *Entity = AppState->Entities + AppState->EntityCount;
    
    Entity->Index = AppState->EntityCount;
    Entity->Type = Type;
    Entity->Dim = Dim;
    Entity->Pos = Pos;
    
    return AppState->EntityCount++;
}

void
App_Init(platform *Platform_)
{
    return;
}

APP_UPDATE(Update)
{
    app_state *AppState = (app_state *)Platform->PermanentStorage;
    
    //~ MEMORY & INITIALIZATION
    memory_arena TextArena = MemoryArenaInit(0, MEGABYTES(10),
                                             Platform->TransientStorage);
    
    v2f32 MousePos = v2f32Init(Platform->Controls->MousePos.x,
                               ((f32)Platform->WindowHeight - Platform->Controls->MousePos.y));
    
    if(!AppState->IsInitialized)
    {
        AppState->IsInitialized = 1;
    }
    
    
    PushClear(RenderData, v4f32Init(0.12f, 0.12f, 0.12f, 1.0f));
    
    UIBeginFrame(Platform);
    
    AppState->DeltaTime += Platform->CurrentTime - Platform->LastTime;
    f32 MoveSpeed  = -200.0f *  AppState->DeltaTime;
    
    //~ INPUT RESPONSE
    if(Platform->Controls[0].AlphaKeys[Key_q].EndedDown)
    {
        Platform->QuitApp = 1;
    }
    
    if(Platform->Controls[0].AlphaKeys[Key_o].EndedDown)
    {
        Entity_Create(AppState, MousePos, v2f32Init(400.0f, 60.0f), Entity_output);
    }
    
    if(Platform->Controls[0].AlphaKeys[Key_c].EndedDown)
    {
        // TODO(MIGUEL): Should connect to a board
        
    }
    
    if(Platform->Controls[0].AlphaKeys[Key_w].EndedDown)
    {
        Entity_Create(AppState, MousePos, v2f32Init(400.0f, 60.0f), Entity_guage);
    }
    
    if(Platform->Controls[0].AlphaKeys[Key_w].EndedDown)
    {
        Entity_Create(AppState, MousePos, v2f32Init(400.0f, 60.0f), Entity_guage);
    }
    
    if(Platform->Controls[0].AlphaKeys[Key_s].EndedDown)
    {
        entity *Entity = AppState->Entities;
        
        Entity->Pos.x  = Platform->WindowWidth  / 2.0f;
        Entity->Pos.y  = Platform->WindowHeight / 2.0f;
    }
    
    telem_packet Packet = TelemetryDequeuePacket(PacketQueues,
                                                 Telem_QueueRecieve);
    
    str8 DroneMsg = str8InitFromArenaFormated(&TextArena,
                                              "Drone Message: %s",
                                              Packet.Payload);
    
    
    //~ UPDATE
    entity *Entity = AppState->Entities;
    for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
    {
        switch(Entity->Type)
        {
            case Entity_output:
            {
                
                ASSERT(DroneMsg.Count != 4294967295);
                
                v2f32 DeltaPos = Entity->Pos; 
                
                
                r2f32 EntityRect = r2f32Init(0, &Entity->Dim, &Entity->Pos);
                if (UIDragXY(&EntityRect, &DeltaPos, Entity))
                {
                    EntityRect.min = v2f32Add(EntityRect.min, v2f32Sub(DeltaPos, Entity->Pos));
                    EntityRect.max = v2f32Add(EntityRect.max, v2f32Sub(DeltaPos, Entity->Pos));
                }
                
                r2f32ConvertToDimPos(&EntityRect, &Entity->Dim, &Entity->Pos);
                
                PushLabel(RenderData,
                          DroneMsg,
                          Entity->Pos, 0.75f, v3f32Init(1.0f, 1.0f, 0.0f));
                
            } break;
            case Entity_guage:
            {
                UIProccessGuage(Entity,
                                AppState->DeltaTime,
                                Platform->Controls->NormThrottlePos,
                                &TextArena,
                                RenderData);
            } break;
        }
        
    }
    
    
    ui_text *UIText = AppState->UIText;
    for(u32 UITextIndex = 0; UITextIndex < AppState->UITextCount; UITextIndex++, UIText++)
    {
        
        entity *Entities = AppState->Entities + UIText->EntityID;
        
        UIText->Pos = Entity->Pos;
        
    }
    
    UIEndFrame();
    
    return;
}
