#include "dc.h"

#include "dc_math.h"
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

void UITextPush(app_state *AppState, memory_arena *Arena,
                str8 String, u32 EntityIndex,
                v2f32 Pos, f32 Scale, v3f32 Color)
{
    ui_text *UIText = AppState->UIText + AppState->UITextCount++;
    
    UIText->Pos      = Pos;
    UIText->Color    = Color;
    UIText->Scale    = Scale;
    UIText->EntityID = EntityIndex;
    
    UIText->String.Data  = MEMORY_ARENA_PUSH_ARRAY(Arena, String.Count, u8);
    UIText->String.Count = String.Count;
    
    MemoryCopy(String.Data, String.Count,
               UIText->String.Data, UIText->String.Count);
    
    return;
}

void UICreateGuage(app_state *AppState, v2f32 Pos, v2f32 Dim, memory_arena *Arena)
{
    u8 *StartupMsg = "Initializing...";
    
    str8 String = str8Init(StartupMsg, sizeof(StartupMsg));
    
    u32 EntityIndex = Entity_Create(AppState,
                                    Pos,
                                    Dim,
                                    Entity_guage);
    
    UITextPush(AppState, Arena, String,
               EntityIndex, Pos, 0.5f,
               v3f32Init(1.0f, 0.0f, 1.0f));
    
    MemoryCopy(StartupMsg, sizeof(StartupMsg) - 1,
               String.Data ,String.Count);
    
    return;
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
        
        UICreateGuage(AppState, v2f32Init(Platform->WindowWidth   / 2.0f,
                                          Platform->WindowHeight  / 2.0f),
                      v2f32Init(240.0f, 40.0f),
                      &AppState->UITextArena);
        
        AppState->IsInitialized = 1;
    }
    
    {
        
        
        AppState->DeltaTime += Platform->CurrentTime - Platform->LastTime;
        f32 MoveSpeed  = -200.0f *  AppState->DeltaTime;
        
        v2f32 MousePos = { Platform->AppInput->UIControls.MousePos.x , Platform->AppInput->UIControls.MousePos.y };
        // ************************************************
        // INPUT RESPONSE
        //*************************************************
        if(Platform->AppInput[0].UIControls.KeyDown[KEY_q])
        {
            app_should_quit = 1;
        }
        
        if(Platform->AppInput[0].UIControls.KeyDown[KEY_c])
        {
            // TODO(MIGUEL): Should connect to a board
        }
        
        if(Platform->AppInput[0].UIControls.KeyDown[KEY_w])
        {
            Entity_Create(AppState, MousePos, v2f32Init(200.0f, 200.0f), Entity_guage);
        }
        
        if(Platform->AppInput[0].UIControls.KeyDown[KEY_s])
        {
            entity *Entity = AppState->Entities;
            
            Entity->Pos.x  = Platform->WindowWidth  / 2.0f;
            Entity->Pos.y  = Platform->WindowHeight / 2.0f;
        }
        
        entity *Entity = AppState->Entities;
        for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
        {
            rect_v2f32 EntityBounds = { 0 };
            
            rect_v2f32_Init(&EntityBounds, &Entity->Dim, &Entity->Pos);
            
            v2f32 NewMousePos = v2f32Init(MousePos.x, (f32)Platform->WindowHeight - MousePos.y);
            
            if(rect_v2f32_IsInRect(&EntityBounds, &NewMousePos) && Platform->AppInput[0].UIControls.MouseLeftButtonDown)
            {
                Entity->Pos = NewMousePos;
            }
        }
        
        
        ui_text *UIText = AppState->UIText;
        for(u32 UITextIndex = 0; UITextIndex < AppState->UITextCount; UITextIndex++, UIText++)
        {
            
            entity *Entities = AppState->Entities + UIText->EntityID;
            
            UIText->Pos = Entity->Pos;
            
        }
        
    }
    return app_should_quit;
}
