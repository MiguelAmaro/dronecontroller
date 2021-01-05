/* date = January 4th 2021 6:48 pm */

#ifndef FLIGHTCONTROL_PLATFORM_H
#define FLIGHTCONTROL_PLATFORM_H

#include "LAL.h"

// NOTE(MIGUEL): This is an Xmacro. Do some research on this
enum {
#define key(name, str) KEY_##name,
#include "Platform_Key_List.inc"
    KEY_MAX
};

typedef struct Platform
{
    void *permanent_storage;
    u32 permanent_storage_size;
    b32 quit;
    b32 key_down[KEY_MAX];
} Platform;


#endif //FLIGHTCONTROL_PLATFORM_H
