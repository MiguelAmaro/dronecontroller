#include "dc.h"

#include "dc_openGL.h"
#include "dc_shader.h"
#include "dc_helpers.h"
#include "dc_entity.h"
#include "dc_math.h"
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

internal void RenderWeirdGradient(app_backbuffer *Buffer, int offset_x, int offset_y)
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


void Entity_Create(app_state *AppState, v2 Pos ,entity_type Type)
{
    ASSERT(AppState->EntityCount < ENTITY_MAX_COUNT);
    
    entity *Entity = AppState->Entities + AppState->EntityCount++;
    
    Entity->Type = Type;
    Entity->Dim[0] = 200.0f;
    Entity->Dim[1] = 200.0f;
    
    Entity->Pos[0]  = Pos.X;
    Entity->Pos[1]  = Pos.Y;
    
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
    
    app_state *AppState = (app_state *)Platform->permanent_storage;
    {
        f32 MoveSpeed  = -200.0f *  AppState->DeltaTime;
        AppState->DeltaTime = Platform->current_time - Platform->last_time;
        
        v2 MousePos = { Platform->mouse_x, Platform->mouse_y };
        // ************************************************
        // INPUT RESPONSE
        //*************************************************
        if(Platform->AppInput->KeyDown[KEY_q])
        {
            app_should_quit = 1;
        }
        if(Platform->AppInput[0].KeyDown[KEY_c])
        {
            // TODO(MIGUEL): Should connect to a board
        }
        // NOTE(MIGUEL): Input only for SRITE AKE GEO(player)
        if(Platform->AppInput[0].KeyDown[KEY_w])
        {
            //glm_translate(translation, (vec3){0.0f, move_speed , 0.0f} );
            //printf("w\n");
            Entity_Create(AppState, MousePos, Entity_guage);
            
        }
        {
            AppState->ThrottleValue = 255.0f * (Platform->throttle);
            printf("%d\n", AppState->ThrottleValue);
        }
        
        
        entity *Entity = AppState->Entities;
        for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
        {
            rect_v2 EntityBounds = { 0 };
            
            v2 EntityPos = (v2){Entity->Pos[0], Entity->Pos[1]};
            v2 EntityDim = (v2){Entity->Dim[0], Entity->Dim[1]};
            
            rect_v2_Init(&EntityBounds, &EntityDim, &EntityPos);
            
            if(rect_v2_IsInRect(&EntityBounds, &MousePos))
            {
                
            }
        }
        
        // NOTE(MIGUEL): this will get clear if when reloading function dynmical from dll
        local_persist XOffset = 0;
        local_persist YOffset = 0;
        
        RenderWeirdGradient(Backbuffer, XOffset,
                            YOffset * AppState->ThrottleValue);
        
        
        ++YOffset;
        ++XOffset;
    }
    
    return app_should_quit;
}
