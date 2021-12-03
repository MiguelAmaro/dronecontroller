/* date = January 5th 2021 8:02 pm */

#ifndef FLIGHTCONTROL_FILEIO_H
#define FLIGHTCONTROL_FILEIO_H

#include "dc_types.h"
#include "dc_platform.h"
#include "dc_memory.h"

#include <stdio.h>
#include <stdarg.h>

typedef struct str8 str8;
struct str8
{
    u8 *Data;
    u32 Count;
};

str8 str8Init(u8 *String, u32 Count);
str8 str8InitFromArena(memory_arena *Arena, u8 *String);
str8 str8InitFromArenaFormated(memory_arena *Arena, u8* Format, ...);
void str8RemoveFromEndToChar  (u8 Char, str8 *A);
void str8Concat(size_t SourceACount, char *SourceA,
                size_t SourceBCount, char *SourceB,
                size_t DestCount   , char *Dest    );

//- UTILS 

void str8AppendBtoA(str8 *A, u32 ASize, str8 B);
void BuildPrefixSuffixTable(u32* Table, u32 TableSize, readonly u8 *SearchTerm);
u32  StringMatchKMP(u8 *Text,  u32 BytesToRead, u8 *SearchTerm);

#endif //FLIGHTCONTROL_FILEIO_H
