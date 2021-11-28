/* date = November 27th 2021 7:35 pm */

#ifndef DC_RENDERER_H
#define DC_RENDERER_H

#include "dc_math.h"
#include "win32_opengl.h"
#include "dc_render_commands.h"
#include "dc_opengl.h"

typedef struct render_constraints render_constraints;
struct render_constraints
{
    u32 QuadMaxCountPerFrame;
};

opengl_renderer *RendererInit(HWND Window, s32 WindowWidth, s32 WindowHeight, render_constraints *Constraints)
{
    opengl_renderer *OpenGLRenderer;
    
    OpenGLRenderer = (opengl_renderer *)VirtualAlloc(0, sizeof(opengl_renderer),
                                                     MEM_COMMIT | MEM_RESERVE,
                                                     PAGE_READWRITE);
    
    
    HDC   GLDeviceContext = GetDC(Window);
    HGLRC GLRealContext   = win32_InitOpenGL(GLDeviceContext);
    
    OpenGLRenderer->DeviceContext = GLDeviceContext;
    OpenGLRenderer->RealContext   = GLRealContext;
    OpenGLRenderer->WindowWidth   = WindowWidth;
    OpenGLRenderer->WindowHeight  = WindowHeight;
    
    ASSERT(gladLoadGL());
    
    u32 gl_major = 0;
    u32 gl_minor = 0;
    
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
    
    render_data RenderData = { 0 };
    
    u32 VertexMaxCount = Constraints->QuadMaxCountPerFrame * 4;
    u32 IndexMaxCount  = Constraints->QuadMaxCountPerFrame * 6 * 2;
    
    u32 RenderCommandBufferSize = 65536;
    
    RenderData.TexturedVertMaxCount    = VertexMaxCount;
    RenderData.UntexturedVertMaxCount  = VertexMaxCount;
    RenderData.RenderCommandsMaxSize   = RenderCommandBufferSize;
    
    RenderData.TexturedVerts   = VirtualAlloc(0, VertexMaxCount * sizeof(textured_vertex),
                                              MEM_COMMIT | MEM_RESERVE,
                                              PAGE_READWRITE);
    
    RenderData.UntexturedVerts = VirtualAlloc(0, VertexMaxCount * sizeof(untextured_vertex),
                                              MEM_COMMIT | MEM_RESERVE,
                                              PAGE_READWRITE);
    
    
    RenderData.Indices         = VirtualAlloc(0, IndexMaxCount * sizeof(u16),
                                              MEM_COMMIT | MEM_RESERVE,
                                              PAGE_READWRITE);
    
    RenderData.RenderCommands  = VirtualAlloc(0, RenderCommandBufferSize * sizeof(u8),
                                              MEM_COMMIT | MEM_RESERVE,
                                              PAGE_READWRITE);
    
    ASSERT(RenderData.TexturedVerts   &&
           RenderData.UntexturedVerts &&
           RenderData.Indices         &&
           RenderData.RenderCommands);
    
    OpenGLRenderer->RenderData = RenderData;
    
    opengl_shader_guage *GuageShader = &OpenGLRenderer->GuageShader;
    opengl_shader_label *LabelShader = &OpenGLRenderer->LabelShader;
    
    
    OPENGL_DBG(glGenBuffers     (1, &OpenGLRenderer->UntexturedVertBufferID));
    OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    
    
    OPENGL_DBG(glGenBuffers     (1, &OpenGLRenderer->TexturedVertBufferID));
    OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    /*
    OPENGL_DBG(glEnableVertexAttribArray(0));
    OPENGL_DBG(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(textured_vertex), (void *)0x00));
    */
    
    OPENGL_DBG(glGenVertexArrays(1, &OpenGLRenderer->IndexBufferID));
    OPENGL_DBG(glBindVertexArray(0));
    
    OpenGLCreateShader(&GuageShader->ID,
                       &GuageSh
                       ader->FileInfo,
                       "../res/shaders/throttle.glsl",
                       sizeof("../res/shaders/throttle.glsl"));
    
    OpenGLCreateShader(&LabelShader->ID,
                       &LabelShader->FileInfo,
                       "../res/shaders/text.glsl",
                       sizeof("../res/shaders/text.glsl"));
    
    
    GuageShader->UIDThrottle          = glGetUniformLocation(GuageShader->ID, "ThrottleValue"  );
    
    GuageShader->UIDWindowSize        = glGetUniformLocation(GuageShader->ID, "WindowSize"  );
    GuageShader->UIDDeltaTime         = glGetUniformLocation(GuageShader->ID, "DeltaTime"  );
    GuageShader->UIDThrottleSize      = glGetUniformLocation(GuageShader->ID, "UISize"  );
    GuageShader->UIDThrottlePos       = glGetUniformLocation(GuageShader->ID, "UIPos"  );
    GuageShader->UIDThrottleTransform = glGetUniformLocation(GuageShader->ID, "Transform"  );
    
    
    LabelShader->UIDWindowSize        = glGetUniformLocation(LabelShader->ID, "WindowSize"  );
    LabelShader->UIDDeltaTime         = glGetUniformLocation(LabelShader->ID, "DeltaTime"  );
    
    
    return OpenGLRenderer;
}

void RendererBeginFrame(opengl_renderer *OpenGL, s32 WindowWidth, s32 WindowHeight)
{
    render_data *RenderData = &OpenGL->RenderData;
    
    OpenGLHotSwapShader(&OpenGL->GuageShader.ID, &OpenGL->GuageShader.FileInfo);
    OpenGLHotSwapShader(&OpenGL->LabelShader.ID, &OpenGL->LabelShader.FileInfo);
    
    OpenGL->WindowWidth  = WindowWidth;
    OpenGL->WindowHeight = WindowHeight;
    
    glViewport(0, 0, OpenGL->WindowWidth, OpenGL->WindowHeight);
    
    RenderData->RenderCommandsFilledPos = RenderData->RenderCommands;
    
    RenderData->TexturedVertCount   = 0;
    RenderData->UntexturedVertCount = 0;
    RenderData->IndexCount          = 0;
    
    
};

void RendererEndFrame(opengl_renderer *OpenGL)
{
    render_data *RenderData = &OpenGL->RenderData;
    
    OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, OpenGL->TexturedVertBufferID)); 
    OPENGL_DBG(glBufferData(GL_ARRAY_BUFFER,
                            RenderData->UntexturedVertCount * sizeof(untextured_vertex),
                            RenderData->UntexturedVerts,
                            GL_STREAM_DRAW));
    
    OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, OpenGL->UntexturedVertBufferID)); 
    OPENGL_DBG(glBufferData(GL_ARRAY_BUFFER,
                            RenderData->TexturedVertCount * sizeof(textured_vertex),
                            RenderData->TexturedVerts,
                            GL_STREAM_DRAW));
    
    
    OPENGL_DBG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGL->IndexBufferID)); 
    OPENGL_DBG(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                            RenderData->IndexCount * sizeof(u16),
                            RenderData->Indices,
                            GL_STREAM_DRAW));
    
#if 0
    OPENGL_DBG(glBindVertexArray(    Glyph->VertexAttribID));
    
    OPENGL_DBG(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)0x00));
    
    // UNBIND BUFFER
    OPENGL_DBG(glEnableVertexAttribArray(0));
    OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    OPENGL_DBG(glBindVertexArray(0));
    
    
    OPENGL_DBG(glGenVertexArrays(1, &Sprite->VertexAttribID));
    OPENGL_DBG(glBindVertexArray(    Sprite->VertexAttribID));
    
    OPENGL_DBG(glEnableVertexAttribArray(0));
    OPENGL_DBG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void *)0x00));
#endif
    
    u8 *CurrentCommandHeaderPos = RenderData->RenderCommands;
    
    m4f32 OrthoProjection = m4f32Orthographic(0.0f, OpenGL->WindowWidth,
                                              0.0f, OpenGL->WindowHeight,
                                              0.1f, 100.0f);
    
    while(CurrentCommandHeaderPos < RenderData->RenderCommandsFilledPos)
    {
        render_command_header *CommandHeader;
        
        CommandHeader = (render_command_header *)CurrentCommandHeaderPos;
        
        switch(CommandHeader->Type)
        {
            case RenderCommand_Clear:
            {
                CurrentCommandHeaderPos += (sizeof(render_command_header) + 
                                            sizeof(render_command_data_clear));
                
                render_command_data_clear *ClearData = 
                    (render_command_data_clear *)((u8 *)CommandHeader + 
                                                  sizeof(render_command_header));
                
                v4f32 Color = ClearData->Color;
                
                glClearColor(Color.r, Color.g, Color.b, Color.a);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            } break;
            case RenderCommand_Guage:
            {
                CurrentCommandHeaderPos += (sizeof(render_command_header) + 
                                            sizeof(render_command_data_guage));
                
                render_command_data_guage *GuageData =
                    ((render_command_data_guage *)((u8 *)CommandHeader + 
                                                   sizeof(render_command_header)));
                
                m4f32 Trans  = m4f32Translate(v3f32Init(GuageData->Pos.x, GuageData->Pos.y, 0.0f));
                m4f32 Scale  = m4f32Scale(GuageData->Dim.x / 2.0f, GuageData->Dim.y / 2.0f, 1.0f);
                m4f32 Rotate = m4f32Identity();
                
                m4f32 World          = m4f32Multiply(Scale, Trans);
                m4f32 GuageTransform = m4f32Multiply(World, OrthoProjection);
                
                opengl_shader_guage *GuageShader = &OpenGL->GuageShader;
                
                v2f32 WindowDim = v2f32Init(OpenGL->WindowWidth, OpenGL->WindowHeight);
                
                OPENGL_DBG(glUseProgram(GuageShader->ID));
                
                OPENGL_DBG(glEnableVertexAttribArray(0));
                OPENGL_DBG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(untextured_vertex), (void *)0x00));
                
                
                OPENGL_DBG(glUniform2fv(GuageShader->UIDWindowSize  , 1, WindowDim.c));
                OPENGL_DBG(glUniform1f (GuageShader->UIDDeltaTime   ,    OpenGL->DeltaTime));
                OPENGL_DBG(glUniform1f (GuageShader->UIDThrottle    ,    GuageData->NormThrottlePos));
                OPENGL_DBG(glUniform2fv(GuageShader->UIDThrottlePos , 1, GuageData->Pos.c));
                OPENGL_DBG(glUniform2fv(GuageShader->UIDThrottleSize, 1, GuageData->Dim.c));
                OPENGL_DBG(glUniformMatrix4fv(GuageShader->UIDThrottleTransform, 1, 0, GuageTransform.e));
                
                OPENGL_DBG(glEnable(GL_BLEND));
                OPENGL_DBG(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                
                OPENGL_DBG(glBindVertexArray(OpenGL->IndexBufferID));
                OPENGL_DBG(glDrawElementsBaseVertex(GL_TRIANGLES, 6 * GuageData->QuadCount, GL_UNSIGNED_SHORT,
                                                    (GLvoid *)(GuageData->IndexArrayOffset*sizeof(u16)),
                                                    GuageData->VertexArrayOffset));
                OPENGL_DBG(glBindVertexArray(0));
            } break;
            case RenderCommand_Label:
            {
                CurrentCommandHeaderPos += (sizeof(render_command_header) + 
                                            sizeof(render_command_data_label));
                
                render_command_data_label *LabelData =
                    ((render_command_data_label *)((u8 *)CommandHeader + 
                                                   sizeof(render_command_header)));
                
                glEnable(GL_CULL_FACE);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                m4f32 GlyphProjection = m4f32Orthographic(0.0f, OpenGL->WindowWidth,
                                                          0.0f, OpenGL->WindowHeight,
                                                          0.1f, 100.0f);
                
                m4f32 GlyphTransform = GlyphProjection;
                
                opengl_shader_label *LabelShader = &OpenGL->LabelShader;
                
                
                OPENGL_DBG(glUseProgram(LabelShader->ID));
                
                OPENGL_DBG(glUniformMatrix4fv(LabelShader->UIDThrottleTransform, 1, 0, GlyphTransform.e));
                OPENGL_DBG(glUniform3f(glGetUniformLocation(LabelShader->ID, "TextColor"),
                                       LabelData->Color.x,
                                       LabelData->Color.y,
                                       LabelData->Color.z));
                
                OPENGL_DBG(glActiveTexture(GL_TEXTURE0));
                OPENGL_DBG(glBindVertexArray(OpenGL->IndexBufferID));
                
                // render glyph texture over quad
                //OPENGL_DBG(glBindTexture(GL_TEXTURE_2D, Glyph.TexID));
                // update content of VBO memory
                OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, OpenGL->IndexBufferID));
                //glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
                OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, 0));
                // render quad
                OPENGL_DBG(glDrawArrays(GL_TRIANGLES, 0, 6));
                // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
                //x += (Glyph.Advance >> 6) * Scale; // bitshift by 6 to get value in pixels (2^6 = 64)
                
                OPENGL_DBG(glBindVertexArray(0));
                OPENGL_DBG(glBindTexture(GL_TEXTURE_2D, 0));
            } break;
        }
        
        OPENGL_DBG(SwapBuffers(OpenGL->DeviceContext));
    }
    
    return;
}

#endif //DC_RENDERER_H
