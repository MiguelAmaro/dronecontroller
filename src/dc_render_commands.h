/* date = November 28th 2021 3:33 am */

#ifndef DC_RENDER_COMMANDS_H
#define DC_RENDER_COMMANDS_H

#include "dc_math.h"

typedef struct textured_vertex textured_vertex;
struct textured_vertex
{
    v2f32 Pos;
    v2f32 TexCoords;
};

typedef struct untextured_vertex untextured_vertex;
struct untextured_vertex
{
    v2f32 Pos;
};

typedef struct render_command_textured_quads render_command_textured_quads;
struct render_command_textured_quads
{
    u32 QuadCount;
    
    // NOTE(MIGUEL): For accessing render_data buffers.
    u32 VertexArrayOffset; // NOTE(casey): Uses 4 vertices per quad
    u32 IndexArrayOffset ; // NOTE(casey): Uses 4 vertices per quad
};


typedef struct render_command_data_clear render_command_data_clear;
struct render_command_data_clear
{
    v4f32 Color;
};


typedef struct render_command_data_guage render_command_data_guage;
struct render_command_data_guage
{
    u32 QuadCount;
    
    u32 VertexArrayOffset;
    u32 IndexArrayOffset ;
    
    v2f32 Pos;
    v2f32 Dim;
    
    f32 NormThrottlePos;
};

typedef struct render_command_data_label render_command_data_label;
struct render_command_data_label
{
    u32 QuadCount;
    
    u32 VertexArrayOffset;
    u32 IndexArrayOffset ;
    
    v2f32 Pos;
    v4f32 Color;
    
    u8 *String;
};

typedef enum render_command_type render_command_type;
enum render_command_type
{
    RenderCommand_Clear,
    RenderCommand_Guage,
    RenderCommand_Label,
};


typedef struct render_command_header render_command_header;
struct render_command_header
{
    render_command_type Type;
};

typedef struct render_data render_data;
struct render_data
{
    u32 RenderCommandsMaxSize;
    u8* RenderCommandsFilledPos;
    u8* RenderCommands;
    
    textured_vertex   *TexturedVerts;
    u32 TexturedVertCount;
    u32 TexturedVertMaxCount;
    
    untextured_vertex *UntexturedVerts;
    u32 UntexturedVertCount;
    u32 UntexturedVertMaxCount;
    
    u16 *Indices;
    u32 IndexMaxCount;
    u32 IndexCount;
};

#endif //DC_RENDER_COMMANDS_H
