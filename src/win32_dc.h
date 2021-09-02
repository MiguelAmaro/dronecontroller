/* date = April 26th 2021 3:58 am */

#ifndef WIN32_DRONECONTROLLER_H
#define WIN32_DRONECONTROLLER_H

#include "dc_platform.h"

typedef struct win32_State win32_State;
struct win32_State
{
    size_t MainMemoryBlockSize;
    u8     MainMemoryBlock;
    
    // TODO(MIGUEL): implement this
    b32 InputPlayBackIndex;
    b32 InputRecordIndex;
};

typedef struct win32_Backbuffer win32_Backbuffer;
struct win32_Backbuffer
{
    BITMAPINFO BitmapInfo;
    void *Data;
    u32 Width;
    u32 Height;
    u32 BytesPerPixel;
};

#endif //WIN32_DRONECONTROLLER_H
