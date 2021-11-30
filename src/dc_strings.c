#include "dc_strings.h"

str8
str8Init(u8 *String, u32 Count)
{
    str8 Result = { 0 };
    
    Result.Data  = String;
    Result.Count = (Count > 0) ? (Count - 1): 0; // Ignore Null Terminator
    
    ASSERT(Result.Count != 4294967295);
    
    return Result;
}

u32
str8GetCStrLength(u8 *Char)
{
    u32 Result = 0; // NOTE(MIGUEL): counting the null terminator
    while(*Char++) {Result++;}
    
    return Result;
}

str8
str8InitFromArena(memory_arena *Arena, u8 *String)
{
    str8 Result = { 0 };
    
    size_t Length = str8GetCStrLength(String) + 1;
    
    Result = str8Init(MEMORY_ARENA_PUSH_ARRAY(Arena, Length, u8),
                      Length);
    MemoryCopy(String     , Result.Count,
               Result.Data, Result.Count);
    
    return Result;
}

str8
str8InitFromArenaFormated(memory_arena *Arena, u8* Format, ...)
{
    // NOTE(MIGUEL): UNSAFE FUNCTION!!! GOOD LUCK! ;)
    va_list Args;
    va_start(Args, Format);
    
    str8 Result;
    
    // NOTE(MIGUEL): this can possibly write outside the arena.
    Result.Data  = ((u8 *)Arena->BasePtr + Arena->Used);
    Result.Count = vsprintf(Result.Data, Format, Args);
    
    va_end(Args);
    
    if(Arena->Used + Result.Count < Arena->Size)
    {
        Arena->Used += Result.Count;
    }
    else
    {
        // NOTE(MIGUEL): If we write outsize, suicide for us.
        //               but in a grandios way.
        ASSERT(!"We fucked up. This is your fault"
               " for using this function! not mine!");
    }
    
    return Result;
}

void
str8RemoveFromEndToChar(u8 Char, str8 *A)
{
    u8  *EndOfA = A->Data + A->Count;
    
    while((*--EndOfA != Char) && (A->Count >= 0))
    {
        *EndOfA = 0;
        A->Count--;
    }
    
    return;
}

void
str8AppendBtoA(str8 *A, u32 ASize, str8 B)
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


//- UTILS 

void 
BuildPrefixSuffixTable(u32* Table, u32 TableSize, readonly u8 *SearchTerm)
{
    u32* OriginalTable = Table;
    u32 i = 1;
    u32 j = 0;
    
    for(u32 i = 0; i < TableSize; i++)
    {
        *(Table + i) = 0;
    }
    
    
    while(i < (TableSize))
    {
        // LAST ELEMENT
        if(i == (TableSize - 1))
        {
            if(*(SearchTerm + i) == *(SearchTerm + j) || j == 0)
            {
                break;
            }
            while(*(SearchTerm + i) != *(SearchTerm + j) && j != 0)
            {
                j = *(Table + j - 1);
            }
            *(Table + i) = ++j;
            i++;
        }
        
        // MISMATCH
        else if(*(SearchTerm + i) != *(SearchTerm + j))
        {
            *(Table + i) = j = 0;
            i++;
        }
        else
        {
            *(Table + i) = j;
            i++;
            
        }
    }
    
    ASSERT(Table == OriginalTable);
    
    return;
}

u32 
StringMatchKMP(u8 *Text,  u32 BytesToRead, u8 *SearchTerm)
{
    u32 SearchTermLength = str8GetCStrLength(SearchTerm);
    
    u32 Result = 0;
    u32 i = 0;
    u32 j = 0;
    
    
    u32  Table[512]; 
    u32 *TableCopy = Table;
    
    ASSERT(SearchTermLength < ARRAY_SIZE(Table));
    ASSERT(Text && Table);
    
    BuildPrefixSuffixTable(Table , SearchTermLength, SearchTerm);
    
    while(i < BytesToRead)
    {
        // CASE: Match
        if(*(SearchTerm + j) == *(Text + i))
        {
            i++; j++;
        }
        
        if(j == SearchTermLength)
        {
            Result = (i - j);
            break;
        }
        
        // CASE: Mis-match
        
        else if( (i < BytesToRead) && (*(SearchTerm + j) != *(Text + i)))
        {
            if(j != 0) { j = *(Table + j - 1); }
            else       { i++;                  }
        }
    }
    
    ASSERT(Table == TableCopy);
    
    return (i - j);
}

