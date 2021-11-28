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
BuildPrefixSuffixTable(u32* table, u32 table_size,readonly u8 *SearchTerm)
{
    //printf("Before: %p \n", table);
    //printf("Before: %p \n", table);
    u32* tableOG = table;
    u32 i = 1;
    u32 j = 0;
    
    for(u32 i = 0; i < table_size; i++)
    {
        *(table + i) = 0;
    }
    
    
    while(i < (table_size))
    {
        // LAST ELEMENT
        if(i == (table_size - 1))
        {
            if(*(SearchTerm + i) == *(SearchTerm + j) || j == 0)
            {
                break;
            }
            while(*(SearchTerm + i) != *(SearchTerm + j) && j != 0)
            {
                j = *(table + j - 1);
            }
            *(table + i) = ++j;
            i++;
        }
        // MISMATCH
        else if(*(SearchTerm + i) != *(SearchTerm + j))
        {
            *(table + i) = j = 0;
            i++;
            //printf("Searchi : %c  <- i : %d| Searchj : %c <- j : %d | PSST %d \n", *(SearchTerm + i), i, *(SearchTerm + j), j, *(table + i));
        }
        // MATCH
        else
        {
            //printf("Searchi : %c  <- i : %d| Searchj : %c <- j : %d | PSST %d \n", *(SearchTerm + i), i, *(SearchTerm + j), j, *(table + i));
            j++;
            *(table + i) = j;
            i++;
            
        }
    }
    
    ASSERT(table == tableOG);
    
    return;
}


// TODO(MIGUEL): Take out file and use u8 * instead
internaldef u32 
StringMatchKMP(readonly u8 *Text, readonly u32 BytesToRead, readonly u8 *SearchTerm)
{
    // NOTE(MIGUEL): This implemenation only returns the first match!!
    readonly u32 SearchTermLength = strlen(SearchTerm);
    u32 result = 0;
    u32 i = 0;
    u32 j = 0;
    
    u32 *table = calloc(SearchTermLength, sizeof(u32)); 
    u32 *tableCopy = table;
    
    ASSERT(Text && table);
    
    
    BuildPrefixSuffixTable(table , SearchTermLength, SearchTerm);
    //for(u32 i = 0; i < SearchTermLength ; i++) { printf("%d ", *(table + i));}
    
    //printf("Before: %p \n", table);
    //printf(SearchTerm);
    
    while(i < BytesToRead)
    {
        //printf("i : %d | j : %d | Text : %c | | Pattern : %c \n",i, j, (u8)*(Text + i), (u8)*(SearchTerm + j));
        
        // CASE: Match
        
        if(*(SearchTerm + j) == *(Text + i))
        {
            i++;
            j++;
        }
        
        if(j == SearchTermLength)
        {
            result = (i - j);
            //printf("MATCH: i = %d - %d = j = %d \n", i , j, (i - j));
            //j = *(table + j - 1);
            break;
        }
        
        // CASE: Mis-match
        
        else if( (i < BytesToRead) && (*(SearchTerm + j) != *(Text + i)))
        {
            if(j != 0)
            {
                j = *(table + j - 1);
                
            }
            else
            {
                i++;
            }
        }
    }
    //printf("MATCH: i = %d - %d = j = %d \n", i , j, (i -j));
    //printf("After : %p \n", table);
    
    ASSERT(table == tableCopy);
    
    free(table);
    
    
    return (i - j);
}


#endif //FLIGHTCONTROL_FILEIO_H
