#include "fc_platform.h"
#include "fc_openGL.h"
#include "fc_shader.h"
#include "fc_helpers.h"
#include "fc_entity.h"
#include "fc_renderer.h"
#include <stdio.h>
#include <math.h>
#include "LAL.h"

// TODO(MIGUEL): App cannnot Crash when stick is not connected
// TODO(MIGUEL): App cannnot Crash when MCU is not connected
// TODO(MIGUEL): App should give use an oppertunity to connect a device(stick, mcu) thoughout app life time
// TODO(MIGUEL): Integrate FreeType library
// TODO(MIGUEL): Implement a simple gui
// TODO(MIGUEL): GUI: No cursor, Buttons with key
// TODO(MIGUEL): Integrate Opencv once app has access to camera feed


typedef struct
{
    u8 throttle_value;
    f32 delta_time;
} App;

typedef struct
{
    void *data           ;
    s32   width          ;
    s32   height         ;
    s32   bytes_per_pixel;
} App_back_buffer;

global App *app = 0;
global Platform *platform = 0;

void
App_Init(Platform *platform_)
{
    return;
}

b32 App_Update(Platform *platform_)
{
    b32 app_should_quit = 0;
    platform = platform_;
    
    app = platform->permanent_storage;
    {
        f32 move_speed  = -200.0f *  app->delta_time;
        app->delta_time = platform->current_time - platform->last_time;
        
        // ************************************************
        // INPUT RESPONSE
        //*************************************************
        if(platform->key_down[KEY_q])
        {
            app_should_quit = 1;
        }
        if(platform->key_down[KEY_c])
        {
            // TODO(MIGUEL): Should connect to a board
        }
        // NOTE(MIGUEL): Input only for SRITE AKE GEO(player)
        if(platform->key_down[KEY_w])
        {
            //glm_translate(translation, (vec3){0.0f, move_speed , 0.0f} );
            //printf("w\n");
        }
        {
            app->throttle_value = 255.0f * (platform->throttle);
            printf("%d\n", app->throttle_value);
        }
    }
    
    return app_should_quit;
}

internal void RenderWeirdGradient(App_back_buffer *buffer, int offset_x, int offset_y)
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

