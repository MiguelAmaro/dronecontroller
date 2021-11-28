#include "dc_render_commands.h"

void PushClear(render_data *RenderData, v4f32 Color)
{
    u8 *EndOfCommandBuffer  = (RenderData->RenderCommands + RenderData->RenderCommandsMaxSize);
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

void PushGuage(render_data *RenderData, v2f32 Pos, v2f32 Dim, f32 NormThrottlePos)
{
    untextured_vertex QuadVerts[4] =
    {
        {  1.0f,  1.0f },
        {  1.0f, -1.0f },
        { -1.0f, -1.0f },
        { -1.0f,  1.0f },
    };
    
    u16 QuadIndices[] = { 0, 1, 3, 0, 2, 3 };
    
    u8 *EndOfCommandBuffer  = (RenderData->RenderCommands + RenderData->RenderCommandsMaxSize);
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
            
            RenderData->UntexturedVertCount += 4;
            RenderData->IndexCount          += 6;
            
            render_command_header *CommandHeader;
            
            CommandHeader = (render_command_header *)(RenderData->RenderCommandsFilledPos);
            CommandHeader->Type = RenderCommand_Guage;
            
            render_command_data_guage *Data;
            Data = (render_command_data_guage *)(CommandHeader + sizeof(render_command_header));
            
            Data->QuadCount = 1;
            Data->VertexArrayOffset = RenderData->UntexturedVertCount;
            Data->IndexArrayOffset  = RenderData->IndexCount;
            Data->Dim = Dim;
            Data->Pos = Pos;
            Data->NormThrottlePos = NormThrottlePos;
            
            RenderData->RenderCommandsFilledPos = CommandPushLocation;
        }
    }
    
    return;
}

#if 0
void RenderLabel(render_data *RenderData, glyph_hash *GlyphHash,
                 str8 String, v2f32 Pos, f32 Scale, v3f32 Color)
{
    u8 *EndOfCommandBuffer  = (RenderData->RenderCommands + RenderData->RenderCommandsMaxSize);
    u8 *CommandPushLocation = (RenderData->RenderCommandsFilledPos +
                               sizeof(render_command_header) + 
                               sizeof(render_command_data_label));
    
    if(CommandPushLocation < EndOfCommandBuffer)
    {
        u32 VertexCountAfterPush = (RenderData->TexturedVertCount + 4) * String.Count;
        u32 IndexCountAfterPush  = (RenderData->IndexCount        + 6) * String.Count;
        
        if((VertexCountAfterPush < RenderData->TexturedVertMaxCount) &&
           (IndexCountAfterPush  < RenderData->IndexMaxCount))
        {
            for (u32 Index = 0; Index < String.Count; Index++)
            {
                u32 Char = String.Data[Index];
                glyph Glyph;// = GlyphHashTableLookup(GlyphHash, Char);
                
                float xpos = Pos.x + Glyph.Bearing.x * Scale;
                float ypos = Pos.y - (Glyph.Dim.y - Glyph.Bearing.y) * Scale;
                
                float w = Glyph.Dim.x * Scale;
                float h = Glyph.Dim.y * Scale;
                
                float QuadVerts[6][4] =
                {
                    { xpos,     ypos + h,   0.0f, 0.0f },            
                    { xpos,     ypos,       0.0f, 1.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },
                    
                    { xpos,     ypos + h,   0.0f, 0.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },
                    { xpos + w, ypos + h,   1.0f, 0.0f }           
                };
                
                u16 QuadIndices[] = { 0, 1, 3, 0, 2, 3 };
                
                textured_vertex *TexturedVertex = (RenderData->TexturedVerts + 
                                                   RenderData->TexturedVertCount);
                
                u16 *Indices = RenderData->Indices + RenderData->IndexCount;
                
                MemoryCopy(QuadVerts     , sizeof(textured_vertex) * 4,
                           TexturedVertex, sizeof(textured_vertex) * 4);
                
                MemoryCopy(QuadIndices, sizeof(u16) * 6,
                           Indices    , sizeof(u16) * 6);
                
                RenderData->UntexturedVertCount += 4;
                RenderData->IndexCount          += 6;
            }
            
            render_command_header *CommandHeader;
            
            CommandHeader = (render_command_header *)(RenderData->RenderCommandsFilledPos);
            CommandHeader->Type = RenderCommand_Guage;
            
            render_command_data_label *Data;
            Data = (render_command_data_label *)(CommandHeader + sizeof(render_command_header));
            
            Data->QuadCount = String.Count;
            Data->VertexArrayOffset = RenderData->UntexturedVertCount;
            Data->IndexArrayOffset  = RenderData->IndexCount;
            Data->Pos    = Pos;
            Data->String = String.Data;
            
            RenderData->RenderCommandsFilledPos = CommandPushLocation;
        }
    }
    
    return;
}
#endif
