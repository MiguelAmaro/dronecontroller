
#include <stdio.h>

//UNITY HEADERS
#include "types.h"
#include "memory.h"
#include "string.h"
#include "os.h"
#include "platform.h"
#include "controllers.h"
#include "telemetry.h"
#include "devices.h"
#include "entity.h"
#include "rendercmd.h"
#include "math.h"
#include "ui.h"
#include "dc.h"
//UNITY SOURCE
#include "os.c"
#include "memory.c"
#include "string.c"
#include "controllers.c"
#include "devices.c"

#define INITIAL_WINDOW_COORD_X (0)
#define INITIAL_WINDOW_COORD_Y (10)
#define INITIAL_WINDOW_WIDTH   (1600)
#define INITIAL_WINDOW_HEIGHT  (600)

#define PERMANENT_STORAGE_SIZE Megabytes(64)
#define TRANSIENT_STORAGE_SIZE Megabytes(64)
#define DEBUG_CONSOLE_BUFFER_SIZE       (4096)

#define RENDERER_OPENGL   0
#define RENDERER_SOFTWARE 0
#define RENDERER_D3D11    1

typedef struct process_state process_state;
struct process_state
{
  u8 *Memory;
  u64 MemorySize;
  arena Arena;
  arena FrameArena;
};
fn void OSGetExeFileName(arena *Arena, str8 *StrArray, u32 Count)
{
  u8 Buffer[236] = {0};
  u32 Status = GetModuleFileNameA(NULL,(LPSTR)Buffer, sizeof(Buffer));
  str8 FilePath = Str8CopyToArena(Arena, Buffer);
  Assert(Status != 0);
  u8 *Dir = FilePath.Data;
  u8 *Exe = FilePath.Data;
  for(u8 *Scan = (u8 *)Buffer; *Scan; ++Scan)
  {
    if(*Scan == '\\') { Exe = Scan + 1;}
  }
  //u32 Length = SafeTruncateu64(CStrGetLength((char *)Exe, 0));
  str8 Name = Str8CopyToArena(Arena, Exe);
  str8 Path = Str8Copy(Arena, Str8(Dir, FilePath.Length-Name.Length));
  StrArray[0] = Name;
  StrArray[1] = Path;
  StrArray[3] = FilePath;
  return;
};
// DYNAMIC APP API
typedef struct plugin plugin;
struct plugin
{
  os_module   Module;
  b32         IsValid;
  datetime LastWrite;
};
typedef struct app_plugin app_plugin;
struct app_plugin
{
  plugin          Plugin;
  api_app_update *Update;
};
fn void AppPluginHotLoad(app_plugin *AppPlugin, str8 SourceDLLName, str8 TempDLLName, str8 LockedFileName)
{
  datetime NewWriteTime = OSFileLastWriteTime(SourceDLLName);
  if(AppPlugin->Plugin.Module && !IsEqual(&NewWriteTime, &AppPlugin->Plugin.LastWrite, datetime))
  { 
    OSModuleUnload(AppPlugin->Plugin.Module);
    AppPlugin->Plugin.IsValid = FALSE;
    AppPlugin->Update = NULL;
  }
  if(OSFileExists(LockedFileName))
  {
    AppPlugin->Plugin.LastWrite = OSFileLastWriteTime(SourceDLLName);
    OSFileCopy(SourceDLLName, TempDLLName);
    AppPlugin->Plugin.Module = OSModuleLoad(TempDLLName);
  }
  if(AppPlugin->Plugin.Module)
  {
    AppPlugin->Update = (api_app_update *)OSModuleGetProc(AppPlugin->Plugin.Module, Str8("Update"));
    AppPlugin->Plugin.IsValid = (AppPlugin->Update != NULL);
  }
  return;
}
int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{
  OSConsoleCreate();
  OSWindowCreate(Instance, V2s(800, 800), V2s(0, 0));
  thread_ctx ThreadContext = {0};
  ThreadCtxInit(&ThreadContext, OSMemoryAlloc(Gigabytes(1)), Gigabytes(1));
  ThreadCtxSet(&ThreadContext);
  
  
  platform Platform = {0};
  process_state Process = {0};
#if 0
  Process.MemorySize = (PERMANENT_STORAGE_SIZE + TRANSIENT_STORAGE_SIZE);
  Process.Memory     = VirtualAlloc(0, Process.MemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  Process.Arena      = ArenaInit(NULL, PERMANENT_STORAGE_SIZE, Process.Memory);
  Process.FrameArena = ArenaInit(NULL, TRANSIENT_STORAGE_SIZE, Process.Memory + PERMANENT_STORAGE_SIZE);
#else
  Platform.PermanentStorage     = OSMemoryAlloc(PERMANENT_STORAGE_SIZE);
  Platform.PermanentStorageSize = PERMANENT_STORAGE_SIZE;
  Platform.TransientStorage     = OSMemoryAlloc(TRANSIENT_STORAGE_SIZE);
  Platform.TransientStorageSize = TRANSIENT_STORAGE_SIZE;
  Platform.Arena = ArenaInit(NULL, PERMANENT_STORAGE_SIZE, Platform.PermanentStorage);
#endif
  scratch_mem Scratch = MemoryGetScratch(NULL, 0);
  str8       *List    = ArenaPushArray(Scratch.Arena, 3, str8);
  OSGetExeFileName(&Platform.Arena, List, 3);
  str8 ExeName  = List[0];
  str8 ExePath  = List[1];
  str8 FullPath = List[3];
  MemoryReleaseScratch(Scratch);
  
  str8 ProcessDllPath     = Str8Concat(ExePath, Str8("dc.dll"), &Platform.Arena);
  str8 ProcessDllTempPath = Str8Concat(ExePath, Str8("dc_temp.dll"), &Platform.Arena);
  str8 ProcessDllLockPath = Str8Concat(ExePath, Str8("lock.dll"), &Platform.Arena);
  app_plugin AppAPI = {0};
  device     Device = {0};
  controller Controller = {0};
  DeviceInit(&Device, "\\\\.\\COM7", 115200, 8);
  u32 Running = 1;
  // Prepeaere Frame Timer
  // Prepeaere Renderer
  while(Running)
  {
    scratch_mem Scratch = MemoryGetScratch(NULL, 0);
    ConsoleLog(*Scratch.Arena, "name: %s\n", ExeName.Data);
    ConsoleLog(*Scratch.Arena, "path: %s\n", ExePath.Data);
    ConsoleLog(*Scratch.Arena, "path: %s\n", FullPath.Data);
    ConsoleLog(*Scratch.Arena, "path: %s\n", ProcessDllPath.Data);
    ConsoleLog(*Scratch.Arena, "path: %s\n", ProcessDllTempPath.Data);
    ConsoleLog(*Scratch.Arena, "path: %s\n", ProcessDllLockPath.Data);
    MemoryReleaseScratch(Scratch);
    
    // Prepeaere Renderer Buckets
    render_data RenderData = {0};
    // Get OS Input
    
    //~ CONTROLLER
    
    //~ DEVICE
    DeviceGetMessage();
    device_info DeviceInfo =
    {
      0,
    };
    
    //~ APP STUFF
    AppPluginHotLoad(&AppAPI, ProcessDllPath, ProcessDllTempPath, ProcessDllLockPath);
    if(AppAPI.Plugin.IsValid)
    {
      AppAPI.Update(&Platform, &DeviceInfo, &RenderData);
    }
    //~ FRAME TIMER
    
  }
  return 0;
}
