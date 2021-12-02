/* date = November 27th 2021 7:35 pm */

#ifndef DC_RENDERER_H
#define DC_RENDERER_H

#include "dc_math.h"
#include "win32_opengl.h"
#include "dc_render_commands.h"
#include "dc_opengl.h"

#include <ft2build.h>
#include FT_FREETYPE_H


// NOTE(MIGUEL): There will be 2 catagories of things for the renderer,
//               * [Specialize Items] like (Guages, Graphs, Text, or any Items
//                 that require special shaders)
//               * [Primitives] only need verts and colors

typedef struct render_constraints render_constraints;
struct render_constraints
{
    u32 QuadMaxCountPerFrame;
};

//- GLYPH STORAGE 
#define GLYPH_MAX_COUNT 256

typedef struct glyph glyph;
struct glyph
{
    u32   CharIndex;
    u32   TexID  ;
    v2s32 Dim    ;
    v2s32 Bearing;
    u32   Advance;
};

typedef struct glyph_hash glyph_hash;
struct glyph_hash
{
    u32   Count;
    u32   MaxCount;
    u32   CharIndex[GLYPH_MAX_COUNT];
    u32   TexID    [GLYPH_MAX_COUNT];
    v2s32 Dim      [GLYPH_MAX_COUNT];
    v2s32 Bearing  [GLYPH_MAX_COUNT];
    u32   Advance  [GLYPH_MAX_COUNT];
    //u32   TexelSize[256];
    //u8    BitmapStore[65536];
};

glyph_hash GlyphHash;

void
GlyphHashTableInit(glyph_hash *GlyphHash);

glyph
GlyphHashTableInsert(glyph_hash *GlyphHash,
                     u32   CharIndex,
                     u32   TextureID,
                     v2s32 Dim,
                     v2s32 Bearing,
                     u32   Advance);

glyph
GlyphHashTableLookup(glyph_hash *GlyphHash, u32   CharIndex);

void
GlyphHashTableFill(glyph_hash *GlyphHash);


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
    RenderData.IndexMaxCount           = IndexMaxCount;
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
    
    //-/ UNTEXUTRED VERTEX
    OPENGL_DBG(glGenVertexArrays(1, &OpenGLRenderer->UntexturedVertAttribID));
    OPENGL_DBG(glGenBuffers     (1, &OpenGLRenderer->UntexturedVertBufferID));
    OPENGL_DBG(glBindVertexArray(    OpenGLRenderer->UntexturedVertAttribID));
    OPENGL_DBG(glGenBuffers(1, &OpenGLRenderer->IndexBufferID));
    OPENGL_DBG(glEnableVertexAttribArray(0));
    OPENGL_DBG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(v2f32), (GLvoid *)0));
    
    //-/ TEXUTURED VERTEX
    OPENGL_DBG(glGenVertexArrays(1, &OpenGLRenderer->TexturedVertAttribID));
    OPENGL_DBG(glBindVertexArray(    OpenGLRenderer->TexturedVertAttribID));
    OPENGL_DBG(glGenBuffers     (1, &OpenGLRenderer->TexturedVertBufferID));
    OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, OpenGLRenderer->TexturedVertBufferID)); 
    OPENGL_DBG(glBufferData(GL_ARRAY_BUFFER,
                            RenderData.TexturedVertMaxCount * sizeof(textured_vertex),
                            0,
                            GL_DYNAMIC_DRAW));
    
    /// POS & TEXCOORD ATTRIB
    OPENGL_DBG(glEnableVertexAttribArray(0));
    OPENGL_DBG(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(v2f32), (GLvoid *)(sizeof(v2f32))));
    OPENGL_DBG(glBindVertexArray(0));
    
#if 0
    /// POSITION ATTRIB
    OPENGL_DBG(glEnableVertexAttribArray(0));
    OPENGL_DBG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(v2f32), (GLvoid *)0));
    /// TEXCOORD ATTRIB
    OPENGL_DBG(glEnableVertexAttribArray(1));
    OPENGL_DBG(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v2f32), (GLvoid *)(sizeof(v2f32))));
#endif
    u16 QuadIndices[] = { 0, 1, 3, 0, 2, 3 };
    
    OPENGL_DBG(glGenBuffers(1, &OpenGLRenderer->TexturedIndexBufferID));
    OPENGL_DBG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGLRenderer->TexturedIndexBufferID)); 
    OPENGL_DBG(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                            ARRAY_SIZE(QuadIndices)* sizeof(u16),
                            QuadIndices,
                            GL_STATIC_DRAW));
    
    opengl_shader_guage *GuageShader = &OpenGLRenderer->GuageShader;
    opengl_shader_label *LabelShader = &OpenGLRenderer->LabelShader;
    opengl_shader_quad  *PrimitiveShader = &OpenGLRenderer->PrimitiveShader;
    
    ASSERT(OpenGLCreateShader(&GuageShader->ID,
                              &GuageShader->FileInfo,
                              "../res/shaders/throttle.glsl",
                              sizeof("../res/shaders/throttle.glsl")));
    
    ASSERT(OpenGLCreateShader(&LabelShader->ID,
                              &LabelShader->FileInfo,
                              "../res/shaders/text.glsl",
                              sizeof("../res/shaders/text.glsl")));
    
    ASSERT(OpenGLCreateShader(&PrimitiveShader->ID,
                              &PrimitiveShader->FileInfo,
                              "../res/shaders/primitive.glsl",
                              sizeof("../res/shaders/primitive.glsl")));
    
    GuageShader->UIDThrottle   = glGetUniformLocation(GuageShader->ID, "ThrottleValue"  );
    GuageShader->UIDWindowSize = glGetUniformLocation(GuageShader->ID, "WindowSize"  );
    GuageShader->UIDDeltaTime  = glGetUniformLocation(GuageShader->ID, "DeltaTime"  );
    GuageShader->UIDSize       = glGetUniformLocation(GuageShader->ID, "UISize"  );
    GuageShader->UIDPos        = glGetUniformLocation(GuageShader->ID, "UIPos"  );
    GuageShader->UIDTransform  = glGetUniformLocation(GuageShader->ID, "Transform"  );
    
    
    LabelShader->UIDWindowSize = glGetUniformLocation(LabelShader->ID, "WindowSize"  );
    LabelShader->UIDDeltaTime  = glGetUniformLocation(LabelShader->ID, "DeltaTime"  );
    LabelShader->UIDTransform  = glGetUniformLocation(LabelShader->ID, "Transform"  );
    
    PrimitiveShader->UIDColor      = glGetUniformLocation(PrimitiveShader->ID, "ColorIn"  );
    PrimitiveShader->UIDTransform  = glGetUniformLocation(PrimitiveShader->ID, "Transform"  );
    
    GlyphHashTableInit(&GlyphHash);
    GlyphHashTableFill(&GlyphHash);
    
    return OpenGLRenderer;
}

void RendererBeginFrame(opengl_renderer *OpenGL, s32 WindowWidth, s32 WindowHeight)
{
    render_data *RenderData = &OpenGL->RenderData;
    
    OpenGLHotSwapShader(&OpenGL->GuageShader.ID, &OpenGL->GuageShader.FileInfo);
    OpenGLHotSwapShader(&OpenGL->LabelShader.ID, &OpenGL->LabelShader.FileInfo);
    OpenGLHotSwapShader(&OpenGL->PrimitiveShader.ID, &OpenGL->PrimitiveShader.FileInfo);
    
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
    
    OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, OpenGL->UntexturedVertBufferID)); 
    OPENGL_DBG(glBufferData(GL_ARRAY_BUFFER,
                            RenderData->UntexturedVertCount * sizeof(untextured_vertex),
                            RenderData->UntexturedVerts,
                            GL_STREAM_DRAW));
    
    OPENGL_DBG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGL->IndexBufferID)); 
    OPENGL_DBG(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                            RenderData->IndexCount * sizeof(u16),
                            RenderData->Indices,
                            GL_STREAM_DRAW));
    
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
            case RenderCommand_Quad:
            {
                CurrentCommandHeaderPos += (sizeof(render_command_header) + 
                                            sizeof(render_command_data_quad));
                
                render_command_data_quad*QuadData =
                    ((render_command_data_quad *)((u8 *)CommandHeader + 
                                                  sizeof(render_command_header)));
                
                m4f32 Trans  = m4f32Translate(v3f32Init(QuadData->Pos.x, QuadData->Pos.y, 0.0f));
                m4f32 Scale  = m4f32Scale(QuadData->Dim.x / 2.0f, QuadData->Dim.y / 2.0f, 1.0f);
                m4f32 Rotate = m4f32Identity();
                
                m4f32 World          = m4f32Multiply(Scale, Trans);
                m4f32 Transform = m4f32Multiply(World, OrthoProjection);
                
                v2f32 WindowDim = v2f32Init(OpenGL->WindowWidth, OpenGL->WindowHeight);
                
                
                
                opengl_shader_quad *PrimitiveShader = &OpenGL->PrimitiveShader;
                OPENGL_DBG(glUseProgram(PrimitiveShader->ID));
                
                OPENGL_DBG(glBindVertexArray(OpenGL->UntexturedVertAttribID));
                OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER,  OpenGL->UntexturedVertBufferID));
                OPENGL_DBG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGL->IndexBufferID));
                
                
                // POSITION ATTRIB
                OPENGL_DBG(glEnableVertexAttribArray(0));
                OPENGL_DBG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(v2f32), (GLvoid *)0));
                
                
                OPENGL_DBG(glUniform4fv(PrimitiveShader->UIDColor, 1, QuadData->Color.c));
                OPENGL_DBG(glUniformMatrix4fv(PrimitiveShader->UIDTransform, 1, 0, Transform.e));
                
                OPENGL_DBG(glEnable(GL_BLEND));
                OPENGL_DBG(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                
                OPENGL_DBG(glDrawElementsBaseVertex(GL_TRIANGLES,
                                                    6 * QuadData->QuadCount,
                                                    GL_UNSIGNED_SHORT,
                                                    (GLvoid *)(QuadData->IndexArrayOffset * sizeof(u16)),
                                                    QuadData->UntexturedVertArrayOffset));
                OPENGL_DBG(glBindVertexArray(0));
                OPENGL_DBG(glUseProgram(0));
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
                m4f32 Transform = m4f32Multiply(World, OrthoProjection);
                
                v2f32 WindowDim = v2f32Init(OpenGL->WindowWidth, OpenGL->WindowHeight);
                
                
                
                opengl_shader_guage *GuageShader = &OpenGL->GuageShader;
                OPENGL_DBG(glUseProgram(GuageShader->ID));
                
                OPENGL_DBG(glBindVertexArray(OpenGL->UntexturedVertAttribID));
                OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER,  OpenGL->UntexturedVertBufferID));
                OPENGL_DBG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGL->IndexBufferID));
                
                
                // POSITION ATTRIB
                OPENGL_DBG(glEnableVertexAttribArray(0));
                OPENGL_DBG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(v2f32), (GLvoid *)0));
                
                
                OPENGL_DBG(glUniform2fv(GuageShader->UIDWindowSize  , 1, WindowDim.c));
                OPENGL_DBG(glUniform1f (GuageShader->UIDDeltaTime   ,    OpenGL->DeltaTime));
                OPENGL_DBG(glUniform1f (GuageShader->UIDThrottle    ,    GuageData->NormThrottlePos));
                OPENGL_DBG(glUniform2fv(GuageShader->UIDPos , 1, GuageData->Pos.c));
                OPENGL_DBG(glUniform2fv(GuageShader->UIDSize, 1, GuageData->Dim.c));
                OPENGL_DBG(glUniformMatrix4fv(GuageShader->UIDTransform, 1, 0, Transform.e));
                
                OPENGL_DBG(glEnable(GL_BLEND));
                OPENGL_DBG(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                
                OPENGL_DBG(glDrawElementsBaseVertex(GL_TRIANGLES,
                                                    6 * GuageData->QuadCount,
                                                    GL_UNSIGNED_SHORT,
                                                    (GLvoid *)(GuageData->IndexArrayOffset * sizeof(u16)),
                                                    GuageData->UntexturedVertArrayOffset));
                OPENGL_DBG(glBindVertexArray(0));
                OPENGL_DBG(glUseProgram(0));
            } break;
            case RenderCommand_Label:
            {
                CurrentCommandHeaderPos += (sizeof(render_command_header) + 
                                            sizeof(render_command_data_label));
                
                render_command_data_label *LabelData =
                    ((render_command_data_label *)((u8 *)CommandHeader + 
                                                   sizeof(render_command_header)));
                
                str8  String = LabelData->String;
                v2f32 Pos    = LabelData->Pos;
                f32   Scale  = LabelData->Scale;
                
                opengl_shader_label *LabelShader = &OpenGL->LabelShader;
                OPENGL_DBG(glUseProgram(LabelShader->ID));
                
                OPENGL_DBG(glUniformMatrix4fv(LabelShader->UIDTransform, 1, 0, OrthoProjection.e));
                OPENGL_DBG(glUniform3f(glGetUniformLocation(LabelShader->ID, "TextColor"),
                                       LabelData->Color.x,
                                       LabelData->Color.y,
                                       LabelData->Color.z));
                
                OPENGL_DBG(glActiveTexture(GL_TEXTURE0));
                
                for (u32 Index = 0; Index < String.Count; Index++)
                {
                    u32 Char = String.Data[Index];
                    glyph Glyph = GlyphHashTableLookup(&GlyphHash, Char);
                    
                    float xpos = Pos.x + Glyph.Bearing.x * Scale;
                    float ypos = Pos.y - (Glyph.Dim.y - Glyph.Bearing.y) * Scale;
                    
                    float w = Glyph.Dim.x * Scale;
                    float h = Glyph.Dim.y * Scale;
                    
#if 0
                    // TODO(MIGUEL): Use indexed Vertices instead. Only After
                    //               implementing a simple IMUI and rendering 
                    //               api. Not a priority.
                    textured_vertex QuadVerts[4] =
                    {
                        { xpos + w, ypos + h,   1.0f, 0.0f },
                        { xpos + w, ypos,       1.0f, 1.0f },
                        { xpos,     ypos,       0.0f, 1.0f },
                        { xpos,     ypos + h,   0.0f, 0.0f },            
                    };
                    u16 QuadIndices[6] = { 0, 1, 2, 0, 2, 3 };
                    //OPENGL_DBG(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0));
#else
                    float vertices[6][4] = {
                        { xpos,     ypos + h,   0.0f, 0.0f },
                        { xpos,     ypos,       0.0f, 1.0f },
                        { xpos + w, ypos,       1.0f, 1.0f },
                        
                        { xpos,     ypos + h,   0.0f, 0.0f },            
                        { xpos + w, ypos,       1.0f, 1.0f },
                        { xpos + w, ypos + h,   1.0f, 0.0f }           
                    };
#endif
                    
                    glBindTexture(GL_TEXTURE_2D, Glyph.TexID);
                    
                    glBindVertexArray(OpenGL->TexturedVertAttribID);
                    
                    //POSITION ATTRIB
                    OPENGL_DBG(glEnableVertexAttribArray(0));
                    OPENGL_DBG(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(v2f32), (GLvoid *)0));
                    
                    OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, OpenGL->TexturedVertBufferID));
                    OPENGL_DBG((glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices)));
                    
                    OPENGL_DBG(glDrawArrays (GL_TRIANGLES, 0, 6));
                    
                    Pos.x += (Glyph.Advance >> 6) * Scale;
                }
                
                glBindVertexArray(0);
                glBindTexture(GL_TEXTURE_2D, 0);
                OPENGL_DBG(glUseProgram(0));
            } break;
        }
    }
    
    OPENGL_DBG(SwapBuffers(OpenGL->DeviceContext));
    
    return;
}


void
GlyphHashTableInit(glyph_hash *GlyphHash)
{
    GlyphHash->Count    = 0;
    GlyphHash->MaxCount = GLYPH_MAX_COUNT;
    
    MemorySet(GlyphHash->CharIndex);
    MemorySet(GlyphHash->TexID);
    MemorySet(GlyphHash->Dim);
    MemorySet(GlyphHash->Bearing);
    MemorySet(GlyphHash->Advance);
    
    return;
}

glyph
GlyphHashTableInsert(glyph_hash *GlyphHash,
                     u32   CharIndex,
                     u32   TextureID,
                     v2s32 Dim,
                     v2s32 Bearing,
                     u32   Advance)
{
    u32 GlyphHashIndex = CharIndex % GlyphHash->MaxCount;
    
    glyph Found = { 0 };
    
    
    for(u32 NumVisited = 0;  NumVisited < GlyphHash->MaxCount; NumVisited++)
    {
        u32 Index = (GlyphHashIndex + NumVisited) % GlyphHash->MaxCount;
        
        if(GlyphHash->CharIndex[Index] == 0)
        {
            GlyphHash->CharIndex[Index] = CharIndex;
            GlyphHash->TexID    [Index] = TextureID;
            GlyphHash->Dim      [Index] = Dim;
            GlyphHash->Bearing  [Index] = Bearing;
            GlyphHash->Advance  [Index] = Advance;
            break;
        }
    }
    
    return Found;
}

glyph
GlyphHashTableLookup(glyph_hash *GlyphHash, u32   CharIndex)
{
    u32 GlyphHashIndex = CharIndex % GlyphHash->MaxCount;
    
    glyph Found = { 0 };
    
    for(u32 NumVisited = 0;  NumVisited < GlyphHash->MaxCount; NumVisited++)
    {
        u32 Index = (GlyphHashIndex + NumVisited) % GlyphHash->MaxCount;
        
        if(GlyphHash->CharIndex[Index] == CharIndex)
        {
            Found.CharIndex = GlyphHash->CharIndex[Index];
            Found.TexID     = GlyphHash->TexID  [Index];
            Found.Dim       = GlyphHash->Dim    [Index];
            Found.Bearing   = GlyphHash->Bearing[Index];
            Found.Advance   = GlyphHash->Advance[Index];
            
            break;
        }
    }
    
    
    return Found;
}

void
GlyphHashTableFill(glyph_hash *GlyphHash)
{
    // NOTE(MIGUEL): Move this to app state maybe?
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        OutputDebugString("FreeType Error: Could not init FreeType Library");
        ASSERT(0);
    }
    
    FT_Face Face;
    if (FT_New_Face(ft, "..\\res\\fonts\\cour.ttf", 0, &Face))
    {
        OutputDebugString("FreeType Error: Could not load Font");
        ASSERT(0);
    }
    
    FT_Set_Pixel_Sizes(Face, 0, 48);
    
    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    for(u32 CharIndex = 0; CharIndex < GLYPH_MAX_COUNT; CharIndex++)
    {
        if (FT_Load_Char(Face, CharIndex, FT_LOAD_RENDER))
        {
            OutputDebugString("FreeType Error: Could not load Glyph");
            ASSERT(0);
            continue;
        }
        
        u32 TextureID;
        glGenTextures(1, &TextureID);
        glBindTexture(GL_TEXTURE_2D, TextureID);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RED,
                     Face->glyph->bitmap.width,
                     Face->glyph->bitmap.rows,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     Face->glyph->bitmap.buffer);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        GlyphHashTableInsert(GlyphHash,
                             CharIndex,
                             TextureID,
                             v2s32Init(Face->glyph->bitmap.width, Face->glyph->bitmap.rows),
                             v2s32Init(Face->glyph->bitmap_left , Face->glyph->bitmap_top),
                             Face->glyph->advance.x);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    FT_Done_Face(Face);
    FT_Done_FreeType(ft);
    
    return;
}


#endif //DC_RENDERER_H
