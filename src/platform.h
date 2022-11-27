#ifndef PLATFORM_H
#define PLATFORM_H


#if 0
#define PLATFORM_FREE_FILE_MEMORY(name) void name(ThreadContext *thread, void *memory)
typedef PLATFORM_FREE_FILE_MEMORY(platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) DebugReadFileResult name(ThreadContext *thread, u8 *file_name)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUG_PlatformReadEntireFile);

#define PLATFORM_SET_CAPTURE(name) void name(b32 Capture)
typedef PLATFORM_SET_CAPTURE(platform_set_capture);

typedef struct platform_api platform_api;
struct platform_api
{
  platform_set_capture *PlatformCaputure;
  
};
#endif

typedef enum platfrom_key platfrom_key;
enum platfrom_key
{
  Key_NULL,
  Key_a, Key_b, Key_c, Key_d,
  Key_e, Key_f, Key_g, Key_h,
  Key_i, Key_j, Key_k, Key_l,
  Key_m, Key_n, Key_o, Key_p,
  Key_q, Key_r, Key_s, Key_t,
  Key_u, Key_v, Key_w, Key_x,
  Key_y, Key_z,
  Key_MAX
};

typedef struct input_button_state input_button_state;
struct input_button_state
{
  b32 IsDown;
  b32 IsPressedNow;
  b32 IsReleasedNow;
  u32 HalfTransitionCount; // button half transition
};

typedef struct input_src input_src;
struct input_src
{
  // UI Oriented
  v2f MousePos;
  
  b32 MouseLeftButtonDown;
  b32 MouseLeftButtonPressed;
  b32 MouseRightButtonDown;
  b32 MouseRightButtonPressed;
  
  input_button_state AlphaKeys[Key_MAX];
  input_button_state   NavKeys[6];
  
  // Drone Oriented
  v2f StickPos;
  f32   NormThrottlePos;
};


typedef struct platform platform;
struct platform
{
  b32 QuitApp;
  v2s WindowDim;
  u32 WindowHeight;
  u32 WindowWidth;
  
  void *PermanentStorage;
  u32   PermanentStorageSize;
  void *TransientStorage;
  u32   TransientStorageSize;
  
  /// INPUT
  input_src Controls[1]; // Allowable controllers xbox360, trustermaster, keyboard
  
  f32 TargetSecondsPerFrame;
  
  // TODO(MIGUEL): normalize the axis inputs
  // NOTE(MIGUEL): initializatin is ASSERT guarded
  b32 StickIsInitialized; 
  
  // TIME
  f32 CurrentTime;
  f32 LastTime;
  arena Arena;
  //platform_api API;
};


#endif //PLATFORM_H
