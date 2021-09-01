/* date = January 4th 2021 6:48 pm */

#ifndef DRONECONTROLLER_PLATFORM_H
#define DRONECONTROLLER_PLATFORM_H

#include "LAL.h"

// NOTE(MIGUEL): This is an Xmacro. Do some research on this
enum {
#define key(name, str) KEY_##name,
#include "Platform_Key_List.inc"
    KEY_MAX
};

typedef struct platform platform;
struct platform
{
    b32 quit;
    u32 window_height;
    u32 window_width;
    
    // STORAGE
    void *permanent_storage;
    u32   permanent_storage_size;
    
    // INPUT
    b32 key_down[KEY_MAX];
    f32 frames_per_second_target;
    
    f32 mouse_x;
    f32 mouse_y;
    b32 mouse_leftb_down;
    b32 mouse_leftb_pressed;
    b32 mouse_rightb_down;
    b32 mouse_rightb_pressed;
    
    // TODO(MIGUEL): normalize the axis inputs
    // NOTE(MIGUEL): initializatin is ASSERT guarded
    b32 stick_is_initialized;
    f32 stick_x;
    f32 stick_y;
    
    f32 throttle;
    
    b32 serialport_is_initialized;
    
    // TIME
    f32 current_time;
    f32 last_time;
};


#endif //DRONECONTROLLER_PLATFORM_H
