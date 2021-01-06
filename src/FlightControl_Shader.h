/* date = January 5th 2021 7:33 pm */

#ifndef FLIGHTCONTROL_SHADER_H
#define FLIGHTCONTROL_SHADER_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LAL.h"



#define VERTSEC ("//~VERT SHADER")
#define FRAGSEC ("//~FRAG SHADER")


u32 
GetFileSize(readonly u8 *path) 
{
    FILE *File;
    File = fopen(path, "r");
    ASSERT(File);
    
    fseek(File, 0L, SEEK_END);
    u32 size_bytes = ftell(File);
    
    return size_bytes;
}



void 
ReadAShaderFile(u32 *ShaderProgram, readonly u8 *path)
{
    //printf("\n\n************************\n  %s  \n ******************** \n", path);
    FILE *File = (void *)0x00;
    File = fopen(path, "r");
    ASSERT(File);
    
    //u32 j = 0;
    //u32 i = 1;
    
    u32 BytesToRead = GetFileSize(path);
    u8 *Shader = calloc( ( BytesToRead + 10 ),  sizeof(u8));
    
    ASSERT(Shader);
    
    // ZERO INITIALIZATION
    for(u32 i = 0; i < ( BytesToRead + 10 ); i++)
    {
        *(Shader + i) = 0x00;
    }
    
    // WRITE FILE INTO BUFFER
    for(u32 i = 0; i < BytesToRead || *(Shader + i) == EOF; i++)
    {
        *(Shader + i) = fgetc(File);
    }
    
    //printf("\n\n************************\n  %s \nsize: %d \n ******************** \n", path, BytesToRead);
    //printf("Shader\n %s \n\n", Shader);
    
    u32 VSpos = StringMatchKMP(Shader, BytesToRead, VERTSEC) + sizeof(VERTSEC);
    u32 FSpos = StringMatchKMP(Shader, BytesToRead, FRAGSEC) + sizeof(FRAGSEC);
    
    *(Shader + FSpos - sizeof(FRAGSEC) - 1) = '\0';
    //*(Shader + BytesToRead) = '\0';
    
    *ShaderProgram = CreateShaderProgram(Shader + VSpos, Shader + FSpos);
    
    //printf("========== Vertex Shader\n%s \nDONE\n\n", Shader + VSpos);
    //printf("========== Fragment Shader\n%s \nDONE\n\n", Shader + FSpos);
    
    //printf("String Match: %d \n", StringMatchKMP(Shader, BytesToRead, ENDSEC));
    //printf("String Match: %d \n", VSpos);
    //printf("String Match: %d \n", FSpos);
    
    fclose(File);
    free(Shader);
    
    return;
}


#endif //FLIGHTCONTROL_SHADER_H
