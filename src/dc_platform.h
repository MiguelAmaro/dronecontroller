/* date = January 4th 2021 6:48 pm */

#ifndef DRONECONTROLLER_PLATFORM_H
#define DRONECONTROLLER_PLATFORM_H

#include <stdint.h>

#define ASSERT(Expression) if(!(Expression)){ *(u32 *)0x00 = 0; }

#define NULLPTR       ((void *)0x00)
#define readonly        const
#define internaldef     static
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

#include "dc_math.h"

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

typedef struct drone_controller_input drone_controller_input;
struct drone_controller_input
{
    v2f32 StickPos;
    
    f32 NormalizedThrottle;
};

typedef struct ui_controller_input ui_controller_input;
struct ui_controller_input
{
    v2f32 MousePos;
    
    b32 MouseLeftButtonDown;
    b32 MouseLeftButtonPressed;
    b32 MouseRightButtonDown;
    b32 MouseRightButtonPressed;
    
    // NOTE(MIGUEL): maybe some of the keys map to something that can control the drone
    b32 KeyDown[KEY_MAX];
};


typedef struct app_input app_input;
struct app_input
{
    ui_controller_input    UIControls;
    drone_controller_input DroneControls;
};


typedef struct platform platform;
struct platform
{
    b32 QuitApp;
    u32 WindowHeight;
    u32 WindowWidth;
    
    // STORAGE
    void *PermanentStorage;
    u32   PermanentStorageSize;
    // STORAGE
    void *TransientStorage;
    u32   TransientStorageSize;
    
    /// INPUT
    app_input AppInput[1]; // Allowable controllers xbox360, trustermaster, keyboard
    
    f32 TargetSecondsPerFrame;
    
    // TODO(MIGUEL): normalize the axis inputs
    // NOTE(MIGUEL): initializatin is ASSERT guarded
    b32 StickIsInitialized; 
    b32 SerialportDeviceConnected;
    
    // TIME
    f32 CurrentTime;
    f32 LastTime;
};


#endif //DRONECONTROLLER_PLATFORM_H
