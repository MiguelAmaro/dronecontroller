/* date = January 4th 2021 6:48 pm */

#ifndef DRONECONTROLLER_PLATFORM_H
#define DRONECONTROLLER_PLATFORM_H

#include <stdint.h>

#define ASSERT(Expression) if(!(Expression)){ *(u32 *)0x00 = 0; }

#define NULLPTR       ((void *)0x00)
#define readonly        const
#define internal        static
#define local_persist   static
#define global          static

#define KILOBYTES(size) (size * 1024)
#define MEGABYTES(size) (KILOBYTES(size) * 1024)
#define GIGABYTES(size) (MEGABYTES(size) * 1024)

typedef uint8_t  u8 ;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef  int8_t  s8 ;
typedef  int16_t s16;
typedef  int32_t s32;
typedef  int64_t s64;

typedef  int8_t  b8 ;
typedef  int16_t b16;
typedef  int32_t b32;
typedef  int64_t b64;

typedef float f32;
typedef float f64;

// NOTE(MIGUEL): This is an Xmacro. Do some research on this
enum {
#define key(name, str) KEY_##name,
#include "Platform_Key_List.inc"
    KEY_MAX
};



typedef struct app_button_state app_button_state;
struct app_button_state
{
    u32 HalfTransitionCount; // button half transition
    b32 EndedDown;
};


typedef struct app_input app_input;
struct app_input
{
    /// Drone control input
    f32 MovementX;
    f32 MovementY;
    
    /// UI control input
    b32 KeyDown[KEY_MAX];
    
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
    
    /// INPUT
    app_input AppInput[2]; // Allowable controllers xbox360, trustermaster, keyboard
    
    //b32 key_down[KEY_MAX];
    f32 frames_per_second_target;
    
    f32 mouse_x; // NOTE(MIGUEL): app_input deprecated soon!
    f32 mouse_y;
    b32 mouse_leftb_down;
    b32 mouse_leftb_pressed;
    b32 mouse_rightb_down;
    b32 mouse_rightb_pressed;
    
    // TODO(MIGUEL): normalize the axis inputs
    // NOTE(MIGUEL): initializatin is ASSERT guarded
    b32 stick_is_initialized;
    f32 stick_x; // NOTE(MIGUEL): this will be stored in app_input
    f32 stick_y;
    
    f32 throttle;
    
    b32 serialport_is_initialized;
    
    // TIME
    f32 current_time;
    f32 last_time;
};


#endif //DRONECONTROLLER_PLATFORM_H
