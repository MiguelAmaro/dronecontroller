#include "dc_platform.h"
#include "dc_openGL.h"
#include "dc_shader.h"
#include "dc_helpers.h"
#include "dc_entity.h"
#include "dc_renderer.h"
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


typedef struct app_state app_state;
struct app_state
{
    u8 throttle_value;
    f32 delta_time;
};

typedef struct app_backbuffer app_backbuffer;
struct app_backbuffer
{
    void *data           ;
    s32   width          ;
    s32   height         ;
    s32   bytes_per_pixel;
};

internal void RenderWeirdGradient(app_backbuffer *buffer, int offset_x, int offset_y)
{
    int width  = buffer->width ;
    int height = buffer->height;
    
    int pitch = width * buffer->bytes_per_pixel;
    u8 *row   = (u8*)buffer->data;
    
    for(int y = 0; y < height; y++)
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
        row += pitch;
    }
    
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
        f32 MoveSpeed  = -200.0f *  AppState->delta_time;
        AppState->delta_time = Platform->current_time - Platform->last_time;
        
        // ************************************************
        // INPUT RESPONSE
        //*************************************************
        if(Platform->key_down[KEY_q])
        {
            app_should_quit = 1;
        }
        if(Platform->key_down[KEY_c])
        {
            // TODO(MIGUEL): Should connect to a board
        }
        // NOTE(MIGUEL): Input only for SRITE AKE GEO(player)
        if(Platform->key_down[KEY_w])
        {
            //glm_translate(translation, (vec3){0.0f, move_speed , 0.0f} );
            //printf("w\n");
        }
        {
            AppState->throttle_value = 255.0f * (Platform->throttle);
            printf("%d\n", AppState->throttle_value);
        }
        
        // NOTE(MIGUEL): this will get clear if when reloading function dynmical from dll
        local_persist XOffset = 0;
        local_persist YOffset = 0;
        
        RenderWeirdGradient(Backbuffer, XOffset,
                            YOffset * AppState->throttle_value);
        
        
        ++YOffset;
        ++XOffset;
    }
    
    return app_should_quit;
}
