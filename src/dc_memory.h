/* date = November 25th 2021 1:33 pm */

#ifndef DC_MEMORY_H
#define DC_MEMORY_H

#include "dc_types.h"

typedef struct memory_arena memory_arena;
struct memory_arena
{
    size_t  Size;
    size_t  Used;
    void   *BasePtr;
};


#define MEMORY_ARENA_PUSH_STRUCT(Arena,        Type) (Type *)MemoryArenaPushBlock(Arena, sizeof(Type))
#define MEMORY_ARENA_PUSH_ARRAY( Arena, Count, Type) (Type *)MemoryArenaPushBlock(Arena, (Count) * sizeof(Type))
#define MEMORY_ARENA_ZERO_STRUCT(Instance          )         MemoryArenaZeroBlock(sizeof(Instance), &(Instance))

#endif //DC_MEMORY_H
