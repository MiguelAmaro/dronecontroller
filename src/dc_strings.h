/* date = January 5th 2021 8:02 pm */

#ifndef FLIGHTCONTROL_FILEIO_H
#define FLIGHTCONTROL_FILEIO_H

#include "dc_types.h"
#include "dc_platform.h"

typedef struct str8 str8;
struct str8
{
    u8 *Data;
    u32 Count;
};

//- STRING FUNCTIONS 

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


//- UTILS 

internaldef void 
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

internaldef u32 
StringMatchKMP(readonly u8 *Text, readonly u32 BytesToRead, readonly u8 *SearchTerm)
{
    readonly u32 SearchTermLength = strlen(SearchTerm);
    
    u32 result = 0;
    u32 i = 0;
    u32 j = 0;
    
    u32 *table = calloc(SearchTermLength, sizeof(u32)); 
    u32 *tableCopy = table;
    
    ASSERT(Text && table);
    
    BuildPrefixSuffixTable(table , SearchTermLength, SearchTerm);
    
    while(i < BytesToRead)
    {
        // CASE: Match
        if(*(SearchTerm + j) == *(Text + i))
        {
            i++; j++;
        }
        
        if(j == SearchTermLength)
        {
            result = (i - j);
            break;
        }
        
        // CASE: Mis-match
        
        else if( (i < BytesToRead) && (*(SearchTerm + j) != *(Text + i)))
        {
            if(j != 0) { j = *(table + j - 1); }
            else       { i++;                  }
        }
    }
    
    ASSERT(table == tableCopy);
    
    free(table);
    
    return (i - j);
}


#endif //FLIGHTCONTROL_FILEIO_H
