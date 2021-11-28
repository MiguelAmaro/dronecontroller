#include "dc.h"

#include "dc_math.h"
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

b32 App_Update(app_backbuffer *Backbuffer, platform *Platform)
{
    b32 app_should_quit = 0;
    
    app_state *AppState = (app_state *)Platform->PermanentStorage;
    
    // MEMORY
    MemoryArenaInit(&AppState->UITextArena,
                    MEGABYTES(10),
                    Platform->TransientStorage);
    
    if(!AppState->IsInitialized)
    {
        
        //UICreateGuage(AppState, v2f32Init(Platform->WindowWidth   / 2.0f,
        //Platform->WindowHeight  / 2.0f),
        //v2f32Init(240.0f, 40.0f),
        //&AppState->UITextArena);
        
        AppState->IsInitialized = 1;
    }
    
    UIBeginFrame(Platform);
    
    AppState->DeltaTime += Platform->CurrentTime - Platform->LastTime;
    f32 MoveSpeed  = -200.0f *  AppState->DeltaTime;
    
    v2f32 MousePos = v2f32Init(Platform->Controls->MousePos.x,
                               Platform->Controls->MousePos.y);
    // ************************************************
    // INPUT RESPONSE
    //*************************************************
    if(Platform->Controls[0].AlphaKeys[Key_q].EndedDown)
    {
        app_should_quit = 1;
    }
    
    if(Platform->Controls[0].AlphaKeys[Key_c].EndedDown)
    {
        // TODO(MIGUEL): Should connect to a board
    }
    
    if(Platform->Controls[0].AlphaKeys[Key_w].EndedDown)
    {
        Entity_Create(AppState, MousePos, v2f32Init(200.0f, 200.0f), Entity_guage);
    }
    
    if(Platform->Controls[0].AlphaKeys[Key_s].EndedDown)
    {
        entity *Entity = AppState->Entities;
        
        Entity->Pos.x  = Platform->WindowWidth  / 2.0f;
        Entity->Pos.y  = Platform->WindowHeight / 2.0f;
    }
    
    // ************************************************
    // UPDATE
    //*************************************************
    entity *Entity = AppState->Entities;
    for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
    {
#if 0
        rect_v2f32 EntityBounds = { 0 };
        
        rect_v2f32_Init(&EntityBounds, &Entity->Dim, &Entity->Pos);
        
        v2f32 NewMousePos = v2f32Init(MousePos.x, (f32)Platform->WindowHeight - MousePos.y);
        
        if(rect_v2f32_IsInRect(&EntityBounds, &NewMousePos) && Platform->Controls->MouseLeftButtonDown)
        {
            Entity->Pos = NewMousePos;
        }
#else
        UIProccessGuage(Entity, AppState->DeltaTime);
#endif
    }
    
    
    ui_text *UIText = AppState->UIText;
    for(u32 UITextIndex = 0; UITextIndex < AppState->UITextCount; UITextIndex++, UIText++)
    {
        
        entity *Entities = AppState->Entities + UIText->EntityID;
        
        UIText->Pos = Entity->Pos;
        
    }
    
    UIEndFrame();
    
    return app_should_quit;
}
