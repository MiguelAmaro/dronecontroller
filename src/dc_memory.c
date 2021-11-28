#include "dc_memory.h"

void
MemoryCopy(void *SrcBuffer, u32 SrcSize,
           void *DstBuffer, u32 DstSize)
{
    
    u8* Src = (u8 *)SrcBuffer;
    u8* Dst = (u8 *)DstBuffer;
    
    for(u32 Index = 0;
        Index < SrcSize && Index < DstSize;
        Index++, Src++, Dst++)
    {
        *Dst = *Src;
    }
    
    return;
}

void
MemorySet(void *SrcBuffer, u32 SrcSize, u32 Value)
{
    
    u8* Src = (u8 *)SrcBuffer;
    
    for(u32 Index = 0;
        Index < SrcSize;
        Index++, Src++)
    {
        *Src = Value;
    }
    
    return;
}


//- ARENAS 

void
MemoryArenaInit(memory_arena *Arena, size_t Size, void *BasePtr)
{
    Arena->BasePtr = BasePtr;
    Arena->Size    = Size;
    Arena->Used    = 0;
    
    return;
}

void
MemoryArenaDiscard(memory_arena *Arena)
{
    // NOTE(MIGUEL): Clearing large Amounts of data e.g ~4gb 
    //               results in a noticable slow down.
    MemorySet(Arena->BasePtr, Arena->Used, 0);
    
    Arena->BasePtr = 0;
    Arena->Size    = 0;
    Arena->Used    = 0;
    
    return;
}

inline void *
MemoryArenaPushBlock(memory_arena *Arena, size_t Size)
{
    ASSERT((Arena->Used + Size) <= Arena->Size);
    
    void *NewArenaPartitionAdress  = (u8 *)Arena->BasePtr + Arena->Used;
    Arena->Used  += Size;
    
    return NewArenaPartitionAdress;
}

inline void
MemoryArenaZeroBlock(size_t size, void *address)
{
    u8 *byte = (u8 *)address;
    
    while(size--)
    {
        *byte++ = 0;
    }
    
    return;
}
