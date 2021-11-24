#include "dc.h"

#include "dc_math.h"
#include "dc_opengl.h"
#include "dc_shader.h"
#include "dc_helpers.h"
#include "dc_entity.h"
//#include "dc_renderer.h"
#include <stdio.h>
#include <math.h>
#include "LAL.h"
// hell
// TODO(MIGUEL): App cannnot Crash when stick is not connected
// TODO(MIGUEL): App cannnot Crash when MCU is not connected
// TODO(MIGUEL): App should give use an oppertunity to connect a device(stick, mcu) thoughout app life time
// TODO(MIGUEL): Integrate FreeType library
// TODO(MIGUEL): Implement a simple gui
// TODO(MIGUEL): GUI: No cursor, Buttons with key
// TODO(MIGUEL): Integrate Opencv once app has access to camera feed


global entity g_Sprite    = {0};

internaldef void RenderWeirdGradient(app_backbuffer *Buffer, int offset_x, int offset_y)
{
    int width  = Buffer->Width ;
    int Height = Buffer->Height;
    
    int Pitch = width * Buffer->BytesPerPixel;
    u8 *row   = (u8*)Buffer->Data;
    
    for(int y = 0; y < Height; y++)
    {
        u8 *pixel = (u8 *)row;
        
        for(int x = 0; x < width; x++)
        {
            // Blue
            * pixel = (u8)(x + offset_x);
            ++pixel;
            
            // Green
            * pixel = (u8)(y + offset_y);
            ++pixel;
            
            // Red
            * pixel = 0;
            ++pixel;
            
            // Padding
            * pixel = 0;
            ++pixel;
        }
        row += Pitch;
    }
    
    return;
}


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
    
    local_persist IsInitialized = 0;
    if(!IsInitialized)
    {
        Entity_Create(AppState,
                      v2f32Init(Platform->WindowWidth   / 2.0f,
                                Platform->WindowHeight  / 2.0f),
                      v2f32Init(240.0f, 40.0f),
                      Entity_guage);
        
        IsInitialized = 1;
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
        // NOTE(MIGUEL): Input only for SRITE AKE GEO(player)
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
        {
            AppState->Throttle = 255.0f * (Platform->AppInput[0].DroneControls.NormalizedThrottle);
            printf("%d\n", AppState->Throttle);
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
        
        // NOTE(MIGUEL): this will get clear if when reloading function dynmical from dll
        local_persist XOffset = 0;
        local_persist YOffset = 0;
        
        RenderWeirdGradient(Backbuffer, XOffset,
                            YOffset * AppState->Throttle);
        
        
        ++YOffset;
        ++XOffset;
    }
    
    return app_should_quit;
}
