#include "dc.h"

#include "dc_math.h"
#include "dc_opengl.h"
#include "dc_entity.h"
#include <stdio.h>
#include <math.h>

void Entity_Create(app_state *AppState, v2f32 Pos , v2f32 Dim, entity_type Type)
{
    ASSERT(AppState->EntityCount < ENTITY_MAX_COUNT);
    
    entity *Entity = AppState->Entities + AppState->EntityCount++;
    
    Entity->Type = Type;
    Entity->Dim = Dim;
    Entity->Pos = Pos;
    
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
    
    if(!AppState->IsInitialized)
    {
        Entity_Create(AppState,
                      v2f32Init(Platform->WindowWidth   / 2.0f,
                                Platform->WindowHeight  / 2.0f),
                      v2f32Init(240.0f, 40.0f),
                      Entity_guage);
        
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
        
        AppState->Throttle = 255.0f * (Platform->AppInput[0].DroneControls.NormalizedThrottle);
        
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
    }
    
    return app_should_quit;
}
