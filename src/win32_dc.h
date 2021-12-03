/* date = April 26th 2021 3:58 am */

#ifndef WIN32_DRONECONTROLLER_H
#define WIN32_DRONECONTROLLER_H

#include "dc_platform.h"

typedef struct win32_thread_info win32_thread_info;
struct win32_thread_info
{
    u32    LogicalThreadIndex;
    DWORD  ID;
    HANDLE Handle;
};

typedef struct win32_backbuffer win32_backbuffer;
struct win32_backbuffer
{
    BITMAPINFO BitmapInfo;
    void *Data;
    u32 Width;
    u32 Height;
    u32 BytesPerPixel;
};

typedef struct win32_state win32_state;
struct win32_state
{
    HWND Window;
    
    size_t  MainMemoryBlockSize;
    void   *MainMemoryBlock;
    
    // TODO(MIGUEL): implement this
    b32 InputPlayBackIndex;
    b32 InputRecordIndex;
    
    win32_thread_info CommThreadInfo;
    
    char  ExeFileName[MAX_PATH];
    char *OnePastLastExeFileNameSlash;
};

#endif //WIN32_DRONECONTROLLER_H
