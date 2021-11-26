/* date = November 25th 2021 1:33 pm */

#ifndef DC_MEMORY_H
#define DC_MEMORY_H

#include "dc_types.h"

typedef struct str8 str8;
struct str8
{
    u8 *Data;
    u32 Count;
};

typedef struct memory_arena memory_arena;
struct memory_arena
{
    size_t  Size;
    size_t  Used;
    void   *BasePtr;
};

//- UTILS 

internaldef
void MemoryCopy(void *SrcBuffer, u32 SrcSize,
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

internaldef
void MemorySet(void *SrcBuffer, u32 SrcSize, u32 Value)
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

internaldef void
MemoryArenaInit(memory_arena *Arena, size_t Size, void *BasePtr)
{
    Arena->BasePtr = BasePtr;
    Arena->Size    = Size;
    Arena->Used    = 0;
    
    return;
}

internaldef void
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

#define MEMORY_ARENA_PUSH_STRUCT(Arena,        Type) (Type *)MemoryArenaPushBlock(Arena, sizeof(Type))
#define MEMORY_ARENA_PUSH_ARRAY( Arena, Count, Type) (Type *)MemoryArenaPushBlock(Arena, (Count) * sizeof(Type))
#define MEMORY_ARENA_ZERO_STRUCT(Instance          )         MemoryArenaZeroBlock(sizeof(Instance), &(Instance))
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


//- STRING 

internaldef
str8 str8Init(u8 *String, u32 Count)
{
    str8 Result = { 0 };
    
    Result.Data  = String;
    Result.Count = (Count > 0) ? (Count - 1): 0; // Ignore Null Terminator
    
    ASSERT(Result.Count != 4294967295);
    
    return Result;
}

internaldef
u32 str8GetCStrLength(u8 *Char)
{
    u32 Result = 0;
    while(*Char++) {Result++;}
    
    return Result;
}

internaldef
void str8RemoveFromEndToChar(u8 Char, str8 *A)
{
    u8  *EndOfA = A->Data + A->Count;
    
    while((*--EndOfA != Char) && (A->Count >= 0))
    {
        *EndOfA = 0;
        A->Count--;
    }
    
    return;
}

internaldef
void str8AppendBtoA(str8 *A, u32 ASize, str8 B)
{
    u8  *EndOfA      = A->Data + A->Count - 1;
    u32  AUnusedSize = ASize - A->Count;
    
    if(AUnusedSize - B.Count > 0)
    {
        MemoryCopy(B.Data, B.Count, EndOfA ,AUnusedSize);
        
        A->Count += B.Count;
    }
    
    return;
}

#endif //DC_MEMORY_H
