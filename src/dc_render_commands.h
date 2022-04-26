/* date = November 28th 2021 3:33 am */

#ifndef DC_RENDER_COMMANDS_H
#define DC_RENDER_COMMANDS_H

#include "dc_types.h"
#include "dc_math.h"
#include "dc_strings.h"


//- PRIMITIVES 

typedef struct textured_vertex textured_vertex;
struct textured_vertex
{
  v2f Pos;
  v2f TexCoords;
};

typedef struct untextured_vertex untextured_vertex;
struct untextured_vertex
{
  v2f Pos;
};

//- COMMANDS 
typedef enum render_command_type render_command_type;
enum render_command_type
{
  RenderCommand_Clear,
  RenderCommand_Guage,
  RenderCommand_Label,
  RenderCommand_Quad,
  RenderCommand_Dbg,
  
  // TODO(MIGUEL): Call this primitive and have paramerized shader draw primitives. Ui funcions will pass in the parameters necessary to for the shader draw the objects need. As far as verts go every thing will be quad except for very special cases.
};

typedef struct render_command_header render_command_header;
struct render_command_header
{
  u32 Type;
};

typedef struct render_command_data_quad render_command_data_quad;
struct render_command_data_quad
{
  u32 QuadCount;
  
  u32 UntexturedVertArrayOffset;
  u32 IndexArrayOffset ;
  
  v2f Pos;
  v2f Dim;
  
  v2f ViewPos;
  v2f ViewDim;
  
  v4f Color;
};

typedef struct render_command_data_guage render_command_data_guage;
struct render_command_data_guage
{
  u32 QuadCount;
  
  u32 UntexturedVertArrayOffset;
  u32 IndexArrayOffset ;
  
  v2f Pos;
  v2f Dim;
  
  v2f ViewPos;
  v2f ViewDim;
  
  
  f32 NormThrottlePos;
};

typedef struct render_command_data_label render_command_data_label;
struct render_command_data_label
{
  u32 QuadCount;
  
  u32 TexturedVertArrayOffset;
  u32 IndexArrayOffset ;
  
  v2f Pos;
  v4f Color;
  
  f32 Scale;
  v2f ViewPos;
  v2f ViewDim;
  
  str8 String;
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
  v4f Color;
};

//- COMMAND & DATA STORAGE 
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


void PushGuage(render_data *RenderData, v2f Pos, v2f Dim, f32 NormThrottlePos,
               v2f ViewPos, v2f ViewDim);
void PushLabel(render_data *RenderData, str8 String, v2f Pos, f32 Scale, v3f Color,
               v2f ViewPos, v2f ViewDim);
void PushLabel2(render_data *RenderData, str8 String, v2f Pos, f32 Scale, v3f Color,
                v2f ViewPos, v2f ViewDim);

void PushRect(render_data *RenderData, v2f Pos, v2f Dim, v4f Color,
              v2f ViewPos, v2f ViewDim);
#endif //DC_RENDER_COMMANDS_H
