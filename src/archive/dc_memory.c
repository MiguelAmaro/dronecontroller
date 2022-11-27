#include "windows.h"
#include "dc_memory.h"
#include "dc_math.h"
void
MemoryCopy(void *SrcBuffer, u64 SrcSize,
           void *DstBuffer, u64 DstSize)
{
  u32 MaxAllowedSize = Min(SrcSize, DstSize);
  u8* Src = (u8 *)SrcBuffer;
  u8* Dst = (u8 *)DstBuffer;
  
  for(u32 Index = 0;
      Index < MaxAllowedSize;
      Index++, Src++, Dst++)
  {
    *Dst = *Src;
  }
  
  return;
}

void
MemorySet(void *SrcBuffer, u64 SrcSize, u8 Value)
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

void
MemoryZero(void *SrcBuffer, u64 SrcSize)
{
  MemorySet(SrcBuffer, SrcSize, 0);
  return;
}

u32
MemoryHexDump(u8 *Buffer, u32 BufferSize, u8 *Data, u32 DataSize, u32 Stride, u32 BreakIndex, u32 Spacing)
{
  // TODO(MIGUEL): Make sure DataSize is Divisable by the stride;
  
  u32 BytesWritten = 0;
  
  for(u32 Line = 0; Line < (DataSize / Stride); Line++)
  {
    u8 *Src = Data + (Stride * Line);
    
    for(u32 Char = 0;
        (Char < Stride) &&
        ((BufferSize - BytesWritten) > 0); 
        Char++)
    {
      u32 NibH = 0xf & (Src[Char] >> 4);
      u32 NibL = 0xf & (Src[Char] >> 0);
      
      BytesWritten += wsprintf(Buffer + BytesWritten, 
                               "%lx%lx ",
                               NibH,
                               NibL);
      
      if(((Char + 1) % BreakIndex) == 0) Buffer[BytesWritten++] = ' ';
    }
    
    for(u32 Space = 0;
        (Space < Spacing) &&
        ((BufferSize - BytesWritten) > 0);
        Space++)
    {
      Buffer[BytesWritten++] = ' ';
    }
    
    for(u32 Char = 0;
        (Char < Stride) &&
        ((BufferSize - BytesWritten) > 0);
        Char++)
    {
      BytesWritten += wsprintf(Buffer + BytesWritten, 
                               "%c",
                               Src[Char] == 0? '.':Src[Char]);
      
      if(((Char + 1) % BreakIndex) == 0) Buffer[BytesWritten++] = ' ';
    }
    
    Buffer[BytesWritten++] = '\n';
  }
  
  return BytesWritten;
}

//- ARENAS 

memory_arena
MemoryArenaInit(memory_arena *Arena, size_t Size, void *BasePtr)
{
  memory_arena Result;
  
  Result.BasePtr = BasePtr;
  Result.Size    = Size;
  Result.Used    = 0;
  
  if(Arena) { *Arena = Result; }
  
  return Result;
}

void
MemoryArenaDiscard(memory_arena *Arena)
{
  // NOTE(MIGUEL): Clearing large Amounts of data e.g ~4gb 
  //               results in a noticable slow down.
  MemorySet(Arena->BasePtr, Arena->Used, (u8)0);
  
  Arena->BasePtr = 0;
  Arena->Size    = 0;
  Arena->Used    = 0;
  
  return;
}

void *
MemoryArenaPushBlock(memory_arena *Arena, size_t Size)
{
  Assert((Arena->Used + Size) <= Arena->Size);
  
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
