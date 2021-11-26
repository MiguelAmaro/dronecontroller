/* date = November 25th 2021 1:33 pm */

#ifndef DC_MEMORY_H
#define DC_MEMORY_H

#include "dc_types.h"

typedef struct str8 str8;
struct str8
{
    u8 *String;
    u32 Count;
};

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
str8 str8Init(u8 *String, u32 Count)
{
    str8 Result = { 0 };
    
    Result.String = String;
    Result.Count  = (Count - 1); // Ignore Null Terminator
    
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
    u8  *EndOfA = A->String + A->Count;
    
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
    u8  *EndOfA      = A->String + A->Count - 1;
    u32  AUnusedSize = ASize - A->Count;
    
    if(AUnusedSize - B.Count > 0)
    {
        MemoryCopy(B.String, B.Count, EndOfA ,AUnusedSize);
        
        A->Count += B.Count;
    }
    
    return;
}


#endif //DC_MEMORY_H
