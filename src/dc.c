#include "dc.h"

#include "dc_math.h"
#include "dc_strings.h"
#include "dc_ui.h"
#include "dc_opengl.h"
#include "dc_entity.h"
#include <stdio.h>
#include <math.h>


internaldef
u32 Entity_Create(app_state *AppState, v2f32 Pos , v2f32 Dim, entity_type Type)
{
    ASSERT(AppState->EntityCount < ENTITY_MAX_COUNT);
    
    entity *Entity = AppState->Entities + AppState->EntityCount;
    
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

void 
App_Update(platform *Platform, app_backbuffer *Backbuffer, render_data *RenderData)
{
    app_state *AppState = (app_state *)Platform->PermanentStorage;
    
    //~ MEMORY & INITIALIZATION
    memory_arena TextArena = MemoryArenaInit(0, MEGABYTES(10),
                                             Platform->TransientStorage);
    
    v2f32 MousePos = v2f32Init(Platform->Controls->MousePos.x,
                               ((f32)Platform->WindowHeight - Platform->Controls->MousePos.y));
    
    if(!AppState->IsInitialized)
    {
        Entity_Create(AppState, v2f32Init(Platform->WindowWidth   / 2.0f,
                                          Platform->WindowHeight  / 2.0f),
                      v2f32Init(400.0f, 60.0f), Entity_guage);
        //UICreateGuage(AppState, v2f32Init(Platform->WindowWidth   / 2.0f,
        //Platform->WindowHeight  / 2.0f),
        //v2f32Init(240.0f, 40.0f),
        //&AppState->UITextArena);
        
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
    
    if(Platform->Controls[0].AlphaKeys[Key_c].EndedDown)
    {
        // TODO(MIGUEL): Should connect to a board
        
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
    
    
    //~ UPDATE
    entity *Entity = AppState->Entities;
    for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
    {
        
        rect_v2f32 EntityBounds = { 0 };
        
        rect_v2f32Init(&EntityBounds, &Entity->Dim, &Entity->Pos);
        
        if(rect_v2f32IsInRect(&EntityBounds, &MousePos) && 
           Platform->Controls->MouseLeftButtonDown)
        {
            Entity->Pos = MousePos;
        }
        
        PushGuage(RenderData,
                  Entity->Pos,
                  Entity->Dim,
                  Platform->Controls->NormThrottlePos);
        
        str8 Test = str8InitFromArenaFormated(&TextArena, " Throttle: %2.2f%", 100.0f * Platform->Controls->NormThrottlePos);
        
        //Platform->Controls->NormThrottlePos
        PushLabel(RenderData,
                  Test,
                  v2f32Addxy(&Entity->Pos, -Entity->Dim.x / 2.0f, -60.0f),
                  0.8f,
                  v3f32Init(1.0f, 0.0f, 2.0f));
        
#if 0
        UIProccessGuage(Entity, AppState->DeltaTime);
#else
#endif
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
