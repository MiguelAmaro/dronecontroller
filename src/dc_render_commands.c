#include "dc_render_commands.h"

void PushClear(render_data *RenderData, v4f Color)
{
  u8 *EndOfCommandBuffer  = (RenderData->RenderCommands + 
                             RenderData->RenderCommandsMaxSize);
  u8 *CommandPushLocation = (RenderData->RenderCommandsFilledPos +
                             sizeof(render_command_header) + 
                             sizeof(render_command_data_clear));
  
  if(CommandPushLocation < EndOfCommandBuffer)
  {
    render_command_header *CommandHeader;
    
    CommandHeader = (render_command_header *)(RenderData->RenderCommandsFilledPos);
    CommandHeader->Type = RenderCommand_Clear;
    
    render_command_data_clear *Data;
    Data = (render_command_data_clear *)((u8 *)CommandHeader + 
                                         sizeof(render_command_header));
    
    Data->Color = Color;
    
    RenderData->RenderCommandsFilledPos = CommandPushLocation;
  }
  
  return;
}


void PushRect(render_data *RenderData, v2f Pos, v2f Dim, v4f Color,
              v2f ViewPos, v2f ViewDim)
{
  untextured_vertex QuadVerts[4] =
  {
    {  1.0f,  1.0f },
    {  1.0f, -1.0f },
    { -1.0f, -1.0f },
    { -1.0f,  1.0f },
  };
  
  u16 QuadIndices[] = { 0, 1, 2, 0, 2, 3 };
  
  u8 *EndOfCommandBuffer  = (RenderData->RenderCommands + 
                             RenderData->RenderCommandsMaxSize);
  u8 *CommandPushLocation = (RenderData->RenderCommandsFilledPos +
                             sizeof(render_command_header) + 
                             sizeof(render_command_data_quad));
  
  if(CommandPushLocation < EndOfCommandBuffer)
  {
    if(((RenderData->UntexturedVertCount + 4) < RenderData->UntexturedVertMaxCount) &&
       ((RenderData->IndexCount          + 6) < RenderData->IndexMaxCount))
    {
      
      untextured_vertex *UntexturedVertex = (RenderData->UntexturedVerts + 
                                             RenderData->UntexturedVertCount);
      
      u16 *Indices = RenderData->Indices + RenderData->IndexCount;
      
      MemoryCopy(QuadVerts       , sizeof(untextured_vertex) * 4,
                 UntexturedVertex, sizeof(untextured_vertex) * 4);
      
      MemoryCopy(QuadIndices, sizeof(u16) * 6,
                 Indices    , sizeof(u16) * 6);
      
      render_command_header *CommandHeader;
      
      CommandHeader = (render_command_header *)(RenderData->RenderCommandsFilledPos);
      CommandHeader->Type = RenderCommand_Quad;
      
      render_command_data_quad *Data;
      Data = (render_command_data_quad *)((u8 *)CommandHeader +
                                          sizeof(render_command_header));
      
      Data->QuadCount = 1;
      Data->UntexturedVertArrayOffset = RenderData->UntexturedVertCount;
      Data->IndexArrayOffset          = RenderData->IndexCount;
      Data->Dim = Dim;
      Data->Pos = Pos;
      Data->ViewDim = ViewDim;
      Data->ViewPos = ViewPos;
      Data->Color= Color;
      
      RenderData->UntexturedVertCount += 4;
      RenderData->IndexCount          += 6;
      RenderData->RenderCommandsFilledPos = CommandPushLocation;
    }
  }
  
  return;
}


void PushGuage(render_data *RenderData, v2f Pos, v2f Dim, f32 NormThrottlePos,
               v2f ViewPos, v2f ViewDim)
{
  untextured_vertex QuadVerts[4] =
  {
    {  1.0f,  1.0f },
    {  1.0f, -1.0f },
    { -1.0f, -1.0f },
    { -1.0f,  1.0f },
  };
  
  u16 QuadIndices[] = { 0, 1, 2, 0, 2, 3 };
  
  u8 *EndOfCommandBuffer  = (RenderData->RenderCommands + 
                             RenderData->RenderCommandsMaxSize);
  u8 *CommandPushLocation = (RenderData->RenderCommandsFilledPos +
                             sizeof(render_command_header) + 
                             sizeof(render_command_data_guage));
  
  if(CommandPushLocation < EndOfCommandBuffer)
  {
    if(((RenderData->UntexturedVertCount + 4) < RenderData->UntexturedVertMaxCount) &&
       ((RenderData->IndexCount          + 6) < RenderData->IndexMaxCount))
    {
      
      untextured_vertex *UntexturedVertex = (RenderData->UntexturedVerts + 
                                             RenderData->UntexturedVertCount);
      
      u16 *Indices = RenderData->Indices + RenderData->IndexCount;
      
      MemoryCopy(QuadVerts       , sizeof(untextured_vertex) * 4,
                 UntexturedVertex, sizeof(untextured_vertex) * 4);
      
      MemoryCopy(QuadIndices, sizeof(u16) * 6,
                 Indices    , sizeof(u16) * 6);
      
      render_command_header *CommandHeader;
      
      CommandHeader = (render_command_header *)(RenderData->RenderCommandsFilledPos);
      CommandHeader->Type = RenderCommand_Guage;
      
      render_command_data_guage *Data;
      Data = (render_command_data_guage *)((u8 *)CommandHeader +
                                           sizeof(render_command_header));
      
      Data->QuadCount = 1;
      Data->UntexturedVertArrayOffset = RenderData->UntexturedVertCount;
      Data->IndexArrayOffset          = RenderData->IndexCount;
      Data->Dim = Dim;
      Data->Pos = Pos;
      Data->ViewDim = ViewDim;
      Data->ViewPos = ViewPos;
      Data->NormThrottlePos = NormThrottlePos;
      
      RenderData->UntexturedVertCount += 4;
      RenderData->IndexCount          += 6;
      RenderData->RenderCommandsFilledPos = CommandPushLocation;
    }
  }
  
  return;
}

void PushLabel(render_data *RenderData,
               str8 String, v2f Pos, f32 Scale, v3f Color, v2f ViewPos, v2f ViewDim)
{
  // NOTE(MIGUEL): For now this function simply reserves memory for the 
  //               the vertices, dispatches a string and leaves it up to 
  //               the renderer to initialize the values.
  
  
  u16 QuadIndices[] = { 0, 1, 2, 0, 2, 3 };
  
  u8 *EndOfCommandBuffer  = (RenderData->RenderCommands + 
                             RenderData->RenderCommandsMaxSize);
  u8 *CommandPushLocation = (RenderData->RenderCommandsFilledPos +
                             sizeof(render_command_header) + 
                             sizeof(render_command_data_label));
  
  if(CommandPushLocation < EndOfCommandBuffer)
  {
    u32 VertexCountAfterPush = RenderData->TexturedVertCount + (4 * String.Count);
    u32 IndexCountAfterPush  = RenderData->IndexCount        + (6 * String.Count);
    
    if((VertexCountAfterPush < RenderData->TexturedVertMaxCount) &&
       (IndexCountAfterPush  < RenderData->IndexMaxCount))
    {
      
      render_command_header *CommandHeader;
      
      CommandHeader = (render_command_header *)(RenderData->RenderCommandsFilledPos);
      CommandHeader->Type = RenderCommand_Label;
      
      render_command_data_label *Data;
      Data = (render_command_data_label *)((u8 *)CommandHeader +
                                           sizeof(render_command_header));
      
      // TODO(MIGUEL): Get rid of this string shit here. have the apropriate quad 
      //               and bitmaps pushed in to a renderbuffer. 
      Data->QuadCount = String.Count;
      Data->TexturedVertArrayOffset = RenderData->TexturedVertCount;
      Data->IndexArrayOffset  = RenderData->IndexCount;
      Data->Pos    = Pos;
      Data->Scale  = Scale;
      Data->String = String;
      Data->ViewDim   = ViewDim;
      Data->ViewPos   = ViewPos;
      Data->Color  = v4fInit(Color.x, Color.y, Color.x, 1.0f);
      
      RenderData->TexturedVertCount += (4 * String.Count);
      RenderData->IndexCount        += (6 * String.Count);
      RenderData->RenderCommandsFilledPos = CommandPushLocation;
    }
  }
  
  return;
}

void PushLabel2(render_data *RenderData,
                str8 String, v2f Pos, f32 Scale, v3f Color, v2f ViewPos, v2f ViewDim)
{
  // NOTE(MIGUEL): For now this function simply reserves memory for the 
  //               the vertices, dispatches a string and leaves it up to 
  //               the renderer to initialize the values.
  
  
  u16 QuadIndices[] = { 0, 1, 2, 0, 2, 3 };
  
  u8 *EndOfCommandBuffer  = (RenderData->RenderCommands + 
                             RenderData->RenderCommandsMaxSize);
  u8 *CommandPushLocation = (RenderData->RenderCommandsFilledPos +
                             sizeof(render_command_header) + 
                             sizeof(render_command_data_label));
  
  if(CommandPushLocation < EndOfCommandBuffer)
  {
    u32 VertexCountAfterPush = RenderData->TexturedVertCount + (4 * String.Count);
    u32 IndexCountAfterPush  = RenderData->IndexCount        + (6 * String.Count);
    
    if((VertexCountAfterPush < RenderData->TexturedVertMaxCount) &&
       (IndexCountAfterPush  < RenderData->IndexMaxCount))
    {
      
      render_command_header *CommandHeader;
      
      CommandHeader = (render_command_header *)(RenderData->RenderCommandsFilledPos);
      CommandHeader->Type = RenderCommand_Dbg;
      
      render_command_data_label *Data;
      Data = (render_command_data_label *)((u8 *)CommandHeader +
                                           sizeof(render_command_header));
      
      Data->QuadCount = String.Count;
      Data->TexturedVertArrayOffset = RenderData->TexturedVertCount;
      Data->IndexArrayOffset  = RenderData->IndexCount;
      Data->Pos    = Pos;
      Data->Scale  = Scale;
      Data->String = String;
      Data->ViewDim   = ViewDim;
      Data->ViewPos   = ViewPos;
      Data->Color  = v4fInit(Color.x, Color.y, Color.x, 1.0f);
      
      RenderData->TexturedVertCount += (4 * String.Count);
      RenderData->IndexCount        += (6 * String.Count);
      RenderData->RenderCommandsFilledPos = CommandPushLocation;
    }
  }
  
  return;
}
