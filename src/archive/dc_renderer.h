/* date = November 27th 2021 7:35 pm */

#ifndef DC_RENDERER_H
#define DC_RENDERER_H

#include "dc_math.h"
#include "win32_opengl.h"
#include "win32_d3d11.h"
#include "dc_render_commands.h"
#include "dc_opengl.h"

#include <ft2build.h>
#include FT_FREETYPE_H


extern D3D11_INPUT_ELEMENT_DESC gVertexLayout[3] = { 0 };
extern u32 gVertexLayoutCount = 3;


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
  v2s Dim    ;
  v2s Bearing;
  u32   Advance;
};

typedef struct glyph_hash glyph_hash;
struct glyph_hash
{
  u32   Count;
  u32   MaxCount;
  u32   CharIndex[GLYPH_MAX_COUNT];
  u32   TexID    [GLYPH_MAX_COUNT];
  v2s Dim      [GLYPH_MAX_COUNT];
  v2s Bearing  [GLYPH_MAX_COUNT];
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
                     v2s Dim,
                     v2s Bearing,
                     u32   Advance);

glyph
GlyphHashTableLookup(glyph_hash *GlyphHash, u32   CharIndex);

void
GlyphHashTableFill(glyph_hash *GlyphHash);

#if RENDERER_OPENGL
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
  
  Assert(gladLoadGL());
  
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
  
  Assert(RenderData.TexturedVerts   &&
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
  OPENGL_DBG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(v2f), (GLvoid *)0));
  
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
  OPENGL_DBG(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(v2f), (GLvoid *)(sizeof(v2f))));
  OPENGL_DBG(glBindVertexArray(0));
  
#if 0
  /// POSITION ATTRIB
  OPENGL_DBG(glEnableVertexAttribArray(0));
  OPENGL_DBG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(v2f), (GLvoid *)0));
  /// TEXCOORD ATTRIB
  OPENGL_DBG(glEnableVertexAttribArray(1));
  OPENGL_DBG(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v2f), (GLvoid *)(sizeof(v2f))));
#endif
  u16 QuadIndices[] = { 0, 1, 3, 0, 2, 3 };
  
  OPENGL_DBG(glGenBuffers(1, &OpenGLRenderer->TexturedIndexBufferID));
  OPENGL_DBG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGLRenderer->TexturedIndexBufferID)); 
  OPENGL_DBG(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                          ArrayCount(QuadIndices)* sizeof(u16),
                          QuadIndices,
                          GL_STATIC_DRAW));
  
  opengl_shader_guage *GuageShader     = &OpenGLRenderer->GuageShader;
  opengl_shader_label *LabelShader     = &OpenGLRenderer->LabelShader;
  opengl_shader_quad  *PrimitiveShader = &OpenGLRenderer->PrimitiveShader;
  
  Assert(OpenGLCreateShader(&GuageShader->ID,
                            &GuageShader->FileInfo,
                            "../res/shaders/throttle.glsl",
                            sizeof("../res/shaders/throttle.glsl")));
  
  Assert(OpenGLCreateShader(&LabelShader->ID,
                            &LabelShader->FileInfo,
                            "../res/shaders/text.glsl",
                            sizeof("../res/shaders/text.glsl")));
  
  Assert(OpenGLCreateShader(&PrimitiveShader->ID,
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
  GlyphHashTableFill(&GlyphHash, 0);
  
  return OpenGLRenderer;
}
#endif
#if RENDERER_D3D11
d3d11_renderer *RendererInit(HWND Window, s32 WindowWidth, s32 WindowHeight, render_constraints *Constraints)
{
  d3d11_renderer *Renderer = (d3d11_renderer *)VirtualAlloc(0, sizeof(d3d11_renderer),
                                                            MEM_COMMIT | MEM_RESERVE,
                                                            PAGE_READWRITE);
  size_t ScratchMemorySize = KILOBYTES(8);
  void *ScratchMemory = VirtualAlloc(0, ScratchMemorySize,
                                     MEM_COMMIT | MEM_RESERVE,
                                     PAGE_READWRITE);
  MemoryArenaInit(&Renderer->Scratch, ScratchMemorySize, ScratchMemory);
  
  
  win32_InitD3D11(Window,
                  &Renderer->FeatureLevel,
                  &Renderer->Device,
                  &Renderer->Context,
                  &Renderer->SwapChain,
                  &Renderer->TargetView);
  
  
  Renderer->WindowWidth   = WindowWidth;
  Renderer->WindowHeight  = WindowHeight;
  
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
  
  Assert(RenderData.TexturedVerts   &&
         RenderData.UntexturedVerts &&
         RenderData.Indices         &&
         RenderData.RenderCommands);
  
  Renderer->RenderData = RenderData;
  
  
  HRESULT Result;
  
  ///VERTEX BUFFERS
  //- TEXUTRED VERTEX
  u32 TexturedVertexSize = (sizeof(v3f)+
                            sizeof(v2f)+
                            sizeof(v2f));
  
  D3D11_BUFFER_DESC TexturedVDesc = { 0 };
  TexturedVDesc.Usage = D3D11_USAGE_DYNAMIC;
  TexturedVDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  TexturedVDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  TexturedVDesc.ByteWidth = TexturedVertexSize* 4096;
  
  Result = ID3D11Device_CreateBuffer(Renderer->Device,
                                     &TexturedVDesc,
                                     NULL, 
                                     &Renderer->TexturedVBuffer);
  //- UNTEXUTRED VERTEX
  u32 UntexturedVertexSize = sizeof(v2f);
  
  D3D11_BUFFER_DESC UntexturedVDesc = { 0 };
  UntexturedVDesc.Usage = D3D11_USAGE_DYNAMIC;
  UntexturedVDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  UntexturedVDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  UntexturedVDesc.ByteWidth = UntexturedVertexSize * 4096;
  
  Result = ID3D11Device_CreateBuffer(Renderer->Device,
                                     &UntexturedVDesc,
                                     NULL,
                                     &Renderer->UntexturedVBuffer);
  Assert(!FAILED(Result));
  
  //- UNTEXUTURED VERTEX
  // (NONE)
  
  ///INDEX BUFFERS
  u16 QuadIndices[] = { 0, 1, 3, 0, 2, 3 };
  
  D3D11_BUFFER_DESC UntexturedIDesc  = { 0 };
  UntexturedIDesc.Usage          = D3D11_USAGE_DEFAULT;
  UntexturedIDesc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
  //UntexturedIDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  UntexturedIDesc.ByteWidth = sizeof(u16) * ArrayCount(QuadIndices);
  
  D3D11_SUBRESOURCE_DATA UntexturedIData = { 0 };
  UntexturedIData.pSysMem = QuadIndices;
  Result = ID3D11Device_CreateBuffer(Renderer->Device,
                                     &UntexturedIDesc,
                                     &UntexturedIData,
                                     &Renderer->UntexturedIBuffer );
  Assert(!FAILED(Result));
  
  /// SET INPUT LAYOUT
  
  gVertexLayout[0].SemanticName         = "POSITION";
  gVertexLayout[0].SemanticIndex        = 0;
  gVertexLayout[0].Format               = DXGI_FORMAT_R32G32_FLOAT;
  gVertexLayout[0].InputSlot            = 0;
  gVertexLayout[0].AlignedByteOffset    = 0;
  gVertexLayout[0].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
  gVertexLayout[0].InstanceDataStepRate = 0;
  
  gVertexLayout[1].SemanticName         = "TEXCOORD";
  gVertexLayout[1].SemanticIndex        = 0;
  gVertexLayout[1].Format               = DXGI_FORMAT_R32G32_FLOAT;
  gVertexLayout[1].InputSlot            = 0;
  gVertexLayout[1].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
  gVertexLayout[1].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
  gVertexLayout[1].InstanceDataStepRate = 0;
  
  
  D3D11InitShaderFileInfo(&Renderer->Guage.Shader.FileInfo,
                          "../res/shaders/throttle.glsl",
                          sizeof("../res/shaders/throttle.glsl"));
  D3D11InitShaderFileInfo(&Renderer->Guage.Shader.FileInfo,
                          "../res/shaders/throttle.glsl",
                          sizeof("../res/shaders/text.glsl"));
  D3D11InitShaderFileInfo(&Renderer->Guage.Shader.FileInfo,
                          "../res/shaders/throttle.glsl",
                          sizeof("../res/shaders/primitive.glsl"));
  
  D3D11LoadShader(Renderer,
                  &Renderer->Guage.Shader.VShader,
                  &Renderer->Guage.Shader.PShader,
                  Renderer->Guage.Shader.FileInfo.FileA,
                  Renderer->Guage.Shader.FileInfo.FileSize,
                  gVertexLayout,
                  gVertexLayoutCount,
                  &Renderer->Scratch);
  D3D11LoadShader(Renderer,
                  &Renderer->Label.Shader.VShader,
                  &Renderer->Label.Shader.PShader,
                  Renderer->Label.Shader.FileInfo.FileA,
                  Renderer->Label.Shader.FileInfo.FileSize,
                  gVertexLayout,
                  gVertexLayoutCount,
                  &Renderer->Scratch);
  D3D11LoadShader(Renderer,
                  &Renderer->Primitive.Shader.VShader,
                  &Renderer->Primitive.Shader.PShader,
                  Renderer->Primitive.Shader.FileInfo.FileA,
                  Renderer->Primitive.Shader.FileInfo.FileSize,
                  gVertexLayout,
                  gVertexLayoutCount,
                  &Renderer->Scratch);
  
  //-/ CONSTAN BUFFERES
  D3D11_BUFFER_DESC      GPUConstDesc     = { 0 };
  D3D11_SUBRESOURCE_DATA GPUConstResource = { 0 };
  // NOTE(MIGUEL): Disabled for easy d3d11 porting
#if 0
  GPUConstDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
  GPUConstDesc.Usage          = D3D11_USAGE_DYNAMIC;
  GPUConstDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  GPUConstDesc.ByteWidth      = sizeof(Renderer->Guage.CBuffer);
  GPUConstResource.pSysMem = &Renderer->Guage.CBuffer;
  
  Result = ID3D11Device_CreateBuffer(Renderer->Device,
                                     &GPUConstDesc,
                                     &GPUConstResource,
                                     &Renderer->GuageConsts);
  Assert(!FAILED(Result));
  
  GPUConstDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
  GPUConstDesc.Usage          = D3D11_USAGE_DYNAMIC;
  GPUConstDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  GPUConstDesc.ByteWidth      = sizeof(Renderer->Label.CBuffer);
  GPUConstResource.pSysMem = &Renderer->Label.CBuffer;
  
  Result = ID3D11Device_CreateBuffer(Renderer->Device,
                                     &GPUConstDesc,
                                     &GPUConstResource,
                                     &Renderer->LabelConsts);
  Assert(!FAILED(Result));
#endif
  GPUConstDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
  GPUConstDesc.Usage          = D3D11_USAGE_DYNAMIC;
  GPUConstDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  GPUConstDesc.ByteWidth      = sizeof(Renderer->Primitive.CBuffer);
  GPUConstResource.pSysMem = &Renderer->Primitive.CBuffer;
  
  Result = ID3D11Device_CreateBuffer(Renderer->Device,
                                     &GPUConstDesc,
                                     &GPUConstResource,
                                     &Renderer->PrimitiveConsts);
  Assert(!FAILED(Result));
  
  
#if 0
  GlyphHashTableInit(&GlyphHash);
  GlyphHashTableFill(&GlyphHash, 0);
#endif
  
  return Renderer;
}
#endif

#if RENDERER_OPENGL
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
#endif
#if RENDERER_D3D11
void RendererBeginFrame(d3d11_renderer *Renderer, s32 WindowWidth, s32 WindowHeight)
{
  render_data *RenderData = &Renderer->RenderData;
  
  //D3D11HotLoadShader(&Renderer->Guage.Shader, 0);
  //D3D11HotLoadShader(&Renderer->Label.Shader, 0);
  //D3D11HotLoadShader(&Renderer->Primitive.Shader, 0);
  
  Renderer->WindowWidth  = WindowWidth;
  Renderer->WindowHeight = WindowHeight;
  
  D3D11_VIEWPORT Viewport;
  Viewport.TopLeftY = 0;
  Viewport.TopLeftX = 0;
  Viewport.MinDepth  = 0;
  Viewport.MaxDepth  = 0;
  Viewport.Width  = (f32)WindowWidth ;
  Viewport.Height = (f32)WindowHeight;
  
  ID3D11DeviceContext_RSSetViewports(Renderer->Context, 1, &Viewport );
  
  RenderData->RenderCommandsFilledPos = RenderData->RenderCommands;
  
  RenderData->TexturedVertCount   = 0;
  RenderData->UntexturedVertCount = 0;
  RenderData->IndexCount          = 0;
  
  
};
#endif

void
RendererDrawView()
{
  
  
  return;
}


#if RENDERER_OPENGL
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
  
  m4f OrthoProjection = m4fOrtho(0.0f, OpenGL->WindowWidth,
                                 0.0f, OpenGL->WindowHeight,
                                 0.1f, 100.0f);
  
  
  
  while(CurrentCommandHeaderPos < RenderData->RenderCommandsFilledPos)
  {
    render_command_header *CommandHeader;
    
    CommandHeader = (render_command_header *)CurrentCommandHeaderPos;
    // NOTE(MIGUEL): These paths should be collapsed since they all
    //               involve rendering quads. Query inforemation And add another path in
    //               the case where im passing more complated geometry.
    switch(CommandHeader->Type)
    {
      case RenderCommand_Clear:
      {
        CurrentCommandHeaderPos += (sizeof(render_command_header) + 
                                    sizeof(render_command_data_clear));
        
        render_command_data_clear *ClearData = 
          (render_command_data_clear *)((u8 *)CommandHeader + 
                                        sizeof(render_command_header));
        
        v4f Color = ClearData->Color;
        
        OPENGL_DBG(glClearColor(Color.r, Color.g, Color.b, Color.a));
        OPENGL_DBG(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
      } break;
      case RenderCommand_Quad:
      {
        CurrentCommandHeaderPos += (sizeof(render_command_header) + 
                                    sizeof(render_command_data_quad));
        
        render_command_data_quad*QuadData =
          ((render_command_data_quad *)((u8 *)CommandHeader + 
                                        sizeof(render_command_header)));
        
        m4f Trans  = m4fTranslate(v3fInit(QuadData->Pos.x, QuadData->Pos.y, 0.0f));
        m4f Scale  = m4fScale(QuadData->Dim.x / 2.0f, QuadData->Dim.y / 2.0f, 1.0f);
        m4f Rotate = m4fIdentity();
        v2f ViewPos = QuadData->ViewPos;
        v2f ViewDim = QuadData->ViewDim;
        
        m4f World          = m4fMultiply(Scale, Trans);
        m4f Transform = m4fMultiply(World, OrthoProjection);
        
        v2f WindowDim = v2fInit(OpenGL->WindowWidth, OpenGL->WindowHeight);
        
        
        
        opengl_shader_quad *PrimitiveShader = &OpenGL->PrimitiveShader;
        OPENGL_DBG(glUseProgram(PrimitiveShader->ID));
        
        OPENGL_DBG(glBindVertexArray(OpenGL->UntexturedVertAttribID));
        OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER,  OpenGL->UntexturedVertBufferID));
        OPENGL_DBG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGL->IndexBufferID));
        
        
        // POSITION ATTRIB
        OPENGL_DBG(glEnableVertexAttribArray(0));
        OPENGL_DBG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(v2f), (GLvoid *)0));
        
        
        OPENGL_DBG(glUniform4fv(PrimitiveShader->UIDColor, 1, QuadData->Color.c));
        OPENGL_DBG(glUniformMatrix4fv(PrimitiveShader->UIDTransform, 1, 0, Transform.e));
        
        //OPENGL_DBG(glEnable(GL_SCISSOR_TEST));
        //OPENGL_DBG(glScissor(ViewPos.x, ViewPos.y, ViewDim.x, ViewDim.y));
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
        
        m4f Trans  = m4fTranslate(v3fInit(GuageData->Pos.x, GuageData->Pos.y, 0.0f));
        m4f Scale  = m4fScale(GuageData->Dim.x / 2.0f, GuageData->Dim.y / 2.0f, 1.0f);
        m4f Rotate = m4fIdentity();
        v2f ViewPos = GuageData->ViewPos;
        v2f ViewDim = GuageData->ViewDim;
        
        m4f World     = m4fMultiply(Scale, Trans);
        m4f Transform = m4fMultiply(World, OrthoProjection);
        
        v2f WindowDim = v2fInit(OpenGL->WindowWidth, OpenGL->WindowHeight);
        
        
        
        opengl_shader_guage *GuageShader = &OpenGL->GuageShader;
        OPENGL_DBG(glUseProgram(GuageShader->ID));
        
        OPENGL_DBG(glBindVertexArray(OpenGL->UntexturedVertAttribID));
        OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER,  OpenGL->UntexturedVertBufferID));
        OPENGL_DBG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGL->IndexBufferID));
        
        
        // POSITION ATTRIB
        OPENGL_DBG(glEnableVertexAttribArray(0));
        OPENGL_DBG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(v2f), (GLvoid *)0));
        
        
        OPENGL_DBG(glUniform2fv(GuageShader->UIDWindowSize  , 1, WindowDim.c));
        OPENGL_DBG(glUniform1f (GuageShader->UIDDeltaTime   ,    OpenGL->DeltaTime));
        OPENGL_DBG(glUniform1f (GuageShader->UIDThrottle    ,    GuageData->NormThrottlePos));
        OPENGL_DBG(glUniform2fv(GuageShader->UIDPos , 1, GuageData->Pos.c));
        OPENGL_DBG(glUniform2fv(GuageShader->UIDSize, 1, GuageData->Dim.c));
        OPENGL_DBG(glUniformMatrix4fv(GuageShader->UIDTransform, 1, 0, Transform.e));
        
        
        //OPENGL_DBG(glEnable(GL_SCISSOR_TEST));
        //OPENGL_DBG(glScissor(ViewPos.x, ViewPos.y, ViewDim.x, ViewDim.y));
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
        v2f Pos    = LabelData->Pos;
        f32   Scale  = LabelData->Scale;
        v2f ViewPos = LabelData->ViewPos;
        v2f ViewDim = LabelData->ViewDim;
        
        opengl_shader_label *LabelShader = &OpenGL->LabelShader;
        OPENGL_DBG(glUseProgram(LabelShader->ID));
        
        OPENGL_DBG(glUniformMatrix4fv(LabelShader->UIDTransform, 1, 0, OrthoProjection.e));
        OPENGL_DBG(glUniform3f(glGetUniformLocation(LabelShader->ID, "TextColor"),
                               LabelData->Color.x,
                               LabelData->Color.y,
                               LabelData->Color.z));
        
        OPENGL_DBG(glActiveTexture(GL_TEXTURE0));
        
        u32 Line = 0;
        
        
        float AdvX = Pos.x;
        for (u32 Index = 0; Index < String.Count; Index++)
        {
          u32 Char = String.Data[Index];
          
          switch((u8)Char)
          {
            case '\n':
            {
              Line++;
              AdvX = Pos.x;
            } break;
            default:
            {
              
              glyph Glyph = GlyphHashTableLookup(&GlyphHash, Char);
              
              f32 GlyphOffsetY = (Glyph.Dim.y - Glyph.Bearing.y) * Scale;
              
              f32 w = Glyph.Dim.x * Scale;
              f32 h = Glyph.Dim.y * Scale;
              u32 LineSpace = 20.0;
              
              // NOTE(MIGUEL): Origin
              f32 OrgX = AdvX + Glyph.Bearing.x * Scale;
              f32 OrgY = Pos.y - (LineSpace * Line) - GlyphOffsetY;
#if 0
              // TODO(MIGUEL): Use indeOrgXed Vertices instead. OnlOrgY After
              //               implementing a simple IMUI and rendering 
              //               api. Not a prioritOrgY.
              teOrgXtured_verteOrgX QuadVerts[4] =
              {
                { OrgX + w, OrgY + h,   1.0f, 0.0f },
                { OrgX + w, OrgY,       1.0f, 1.0f },
                { OrgX,     OrgY,       0.0f, 1.0f },
                { OrgX,     OrgY + h,   0.0f, 0.0f },            
              };
              u16 QuadIndices[6] = { 0, 1, 2, 0, 2, 3 };
              //OPENGL_DBG(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_ShORT, 0));
#else
              f32 vertices[6][4] =
              {
                { OrgX,     OrgY + h,   0.0f, 0.0f },
                { OrgX,     OrgY,       0.0f, 1.0f },
                { OrgX + w, OrgY,       1.0f, 1.0f },
                
                { OrgX,     OrgY + h,   0.0f, 0.0f },            
                { OrgX + w, OrgY,       1.0f, 1.0f },
                { OrgX + w, OrgY + h,   1.0f, 0.0f }           
              };
#endif
              glBindTexture(GL_TEXTURE_2D, Glyph.TexID);
              
              glBindVertexArray(OpenGL->TexturedVertAttribID);
              
              //POSITION ATTRIB
              OPENGL_DBG(glEnableVertexAttribArray(0));
              OPENGL_DBG(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(v2f), (GLvoid *)0));
              
              OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, OpenGL->TexturedVertBufferID));
              OPENGL_DBG((glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices)));
              
              //OPENGL_DBG(glEnable(GL_SCISSOR_TEST));
              //OPENGL_DBG(glScissor(ViewPos.x, ViewPos.y, ViewDim.x, ViewDim.y));
              OPENGL_DBG(glDrawArrays (GL_TRIANGLES, 0, 6));
              
              AdvX += (Glyph.Advance >> 6) * Scale;
            } break;
          }
        }
        
        OPENGL_DBG(glBindVertexArray(0));
        OPENGL_DBG(glBindTexture(GL_TEXTURE_2D, 0));
        OPENGL_DBG(glUseProgram(0));
      } break;
      
      case RenderCommand_Dbg:
      {
        CurrentCommandHeaderPos += (sizeof(render_command_header) + 
                                    sizeof(render_command_data_label));
        
        render_command_data_label *LabelData =
          ((render_command_data_label *)((u8 *)CommandHeader + 
                                         sizeof(render_command_header)));
        
        str8  String = LabelData->String;
        v2f Pos    = LabelData->Pos;
        f32   Scale  = LabelData->Scale;
        v2f ViewPos = LabelData->ViewPos;
        v2f ViewDim = LabelData->ViewDim;
        
        
        OPENGL_DBG(glEnable(GL_SCISSOR_TEST));
        OPENGL_DBG(glClearColor(0.25f, 0.1f, 0.1f, 0.8f));
        OPENGL_DBG(glScissor(ViewPos.x, ViewPos.y, ViewDim.x, ViewDim.y));
        OPENGL_DBG(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
        opengl_shader_label *LabelShader = &OpenGL->LabelShader;
        OPENGL_DBG(glUseProgram(LabelShader->ID));
        
        OPENGL_DBG(glUniformMatrix4fv(LabelShader->UIDTransform, 1, 0, OrthoProjection.e));
        OPENGL_DBG(glUniform3f(glGetUniformLocation(LabelShader->ID, "TextColor"),
                               LabelData->Color.x,
                               LabelData->Color.y,
                               LabelData->Color.z));
        
        OPENGL_DBG(glActiveTexture(GL_TEXTURE0));
        
        u32 Line = 0;
        
        
        float AdvX = Pos.x;
        for (u32 Index = 0; Index < String.Count; Index++)
        {
          u32 Char = String.Data[Index];
          
          switch((u8)Char)
          {
            case '\n':
            {
              Line++;
              AdvX = Pos.x;
            } break;
            default:
            {
              
              glyph Glyph = GlyphHashTableLookup(&GlyphHash, Char);
              
              f32 GlyphOffsetY = (Glyph.Dim.y - Glyph.Bearing.y) * Scale;
              
              f32 w = Glyph.Dim.x * Scale;
              f32 h = Glyph.Dim.y * Scale;
              u32 LineSpace = 20.0;
              
              // NOTE(MIGUEL): Origin
              f32 OrgX = AdvX + Glyph.Bearing.x * Scale;
              f32 OrgY = Pos.y - (LineSpace * Line) - GlyphOffsetY;
#if 0
              // TODO(MIGUEL): Use indeOrgXed Vertices instead. OnlOrgY After
              //               implementing a simple IMUI and rendering 
              //               api. Not a prioritOrgY.
              teOrgXtured_verteOrgX QuadVerts[4] =
              {
                { OrgX + w, OrgY + h,   1.0f, 0.0f },
                { OrgX + w, OrgY,       1.0f, 1.0f },
                { OrgX,     OrgY,       0.0f, 1.0f },
                { OrgX,     OrgY + h,   0.0f, 0.0f },            
              };
              u16 QuadIndices[6] = { 0, 1, 2, 0, 2, 3 };
              //OPENGL_DBG(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_ShORT, 0));
#else
              f32 vertices[6][4] =
              {
                { OrgX,     OrgY + h,   0.0f, 0.0f },
                { OrgX,     OrgY,       0.0f, 1.0f },
                { OrgX + w, OrgY,       1.0f, 1.0f },
                
                { OrgX,     OrgY + h,   0.0f, 0.0f },            
                { OrgX + w, OrgY,       1.0f, 1.0f },
                { OrgX + w, OrgY + h,   1.0f, 0.0f }           
              };
#endif
              OPENGL_DBG(glBindTexture(GL_TEXTURE_2D, Glyph.TexID));
              
              OPENGL_DBG(glBindVertexArray(OpenGL->TexturedVertAttribID));
              
              //POSITION ATTRIB
              OPENGL_DBG(glEnableVertexAttribArray(0));
              OPENGL_DBG(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(v2f), (GLvoid *)0));
              
              OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, OpenGL->TexturedVertBufferID));
              OPENGL_DBG(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
              
              OPENGL_DBG(glDrawArrays (GL_TRIANGLES, 0, 6));
              
              AdvX += (Glyph.Advance >> 6) * Scale;
            } break;
          }
        }
        
        OPENGL_DBG(glBindVertexArray(0));
        OPENGL_DBG(glBindTexture(GL_TEXTURE_2D, 0));
        OPENGL_DBG(glDisable(GL_SCISSOR_TEST));
        OPENGL_DBG(glUseProgram(0));
      } break;
    }
  }
  
  OPENGL_DBG(SwapBuffers(OpenGL->DeviceContext));
  
  return;
}
#endif
#if RENDERER_D3D11
void RendererEndFrame(d3d11_renderer *Renderer)
{
  render_data *RenderData = &Renderer->RenderData;
  u8 *CurrentCommandHeaderPos = RenderData->RenderCommands;
  
  m4f OrthoProjection = m4fOrtho(0.0f, Renderer->WindowWidth,
                                 0.0f, Renderer->WindowHeight,
                                 0.1f, 100.0f);
  
  
  
  while(CurrentCommandHeaderPos < RenderData->RenderCommandsFilledPos)
  {
    render_command_header *CommandHeader;
    
    CommandHeader = (render_command_header *)CurrentCommandHeaderPos;
    // NOTE(MIGUEL): These paths should be collapsed since they all
    //               involve rendering quads. Query inforemation And add another path in
    //               the case where im passing more complated geometry.
    switch(CommandHeader->Type)
    {
      case RenderCommand_Clear:
      {
        // TODO(MIGUEL): should be a function
        CurrentCommandHeaderPos += (sizeof(render_command_header) + 
                                    sizeof(render_command_data_clear));
        
        render_command_data_clear *ClearData = 
          (render_command_data_clear *)((u8 *)CommandHeader + 
                                        sizeof(render_command_header));
        
        v4f Color = ClearData->Color;
        
        ID3D11DeviceContext_ClearRenderTargetView(Renderer->Context, Renderer->TargetView, Color.c);
      } break;
      case RenderCommand_Quad:
      {
        CurrentCommandHeaderPos += (sizeof(render_command_header) + 
                                    sizeof(render_command_data_quad));
#if 1
        render_command_data_quad*QuadData =
          ((render_command_data_quad *)((u8 *)CommandHeader + 
                                        sizeof(render_command_header)));
        
        m4f Trans  = m4fTranslate(v3fInit(QuadData->Pos.x, QuadData->Pos.y, 0.0f));
        m4f Scale  = m4fScale(QuadData->Dim.x / 2.0f, QuadData->Dim.y / 2.0f, 1.0f);
        m4f Rotate = m4fIdentity();
        v2f ViewPos = QuadData->ViewPos;
        v2f ViewDim = QuadData->ViewDim;
        
        m4f World          = m4fMultiply(Scale, Trans);
        m4f Transform = m4fMultiply(World, OrthoProjection);
        
        v2f WindowDim = v2fInit(Renderer->WindowWidth, Renderer->WindowHeight);
        
        // NOTE(MIGUEL): primitive or quad
        d3d11_quad *Primitive = &Renderer->Primitive;
        
        
#if 0
        OPENGL_DBG(glEnable(GL_BLEND));
        OPENGL_DBG(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        
        OPENGL_DBG(glDrawElementsBaseVertex(GL_TRIANGLES,
                                            6 * QuadData->QuadCount,
                                            GL_UNSIGNED_SHORT,
                                            (GLvoid *)(QuadData->IndexArrayOffset * sizeof(u16)),
                                            QuadData->UntexturedVertArrayOffset));
        OPENGL_DBG(glBindVertexArray(0));
        OPENGL_DBG(glUseProgram(0));
#endif
        // NOTE(MIGUEL): Sets the Model and How to Shade it
        u32 Stride[] = {sizeof(v3f)+sizeof(v2f)+sizeof(v2f)};
        u32 Offset[] = { 0 };
        
        ID3D11DeviceContext_IASetVertexBuffers(Renderer->Context, 0, 1, &Renderer->UntexturedVBuffer, Stride, Offset);
        ID3D11DeviceContext_IASetInputLayout(Renderer->Context, Renderer->InputLayout);
        ID3D11DeviceContext_IASetIndexBuffer(Renderer->Context,
                                             Renderer->TexturedIBuffer, DXGI_FORMAT_R16_UINT, 0 );
        ID3D11DeviceContext_IASetPrimitiveTopology(Renderer->Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        // NOTE(MIGUEL): This is only because this constant buffer isnt set to
        //               dynamic.(UpdateSubresource() call)
        // TODO(MIGUEL): Create a compiled path for a dynamic constant buffer
        
        
        Primitive->Color = QuadData->Color;
        Primitive->Transform = Transform;
        
        
        D3D11_MAPPED_SUBRESOURCE PrimitiveMap = {0};
        
        //CONST BUFFER
        ID3D11DeviceContext_Map(Renderer->Context, Renderer->PrimitiveConsts, 0,
                                D3D11_MAP_WRITE_DISCARD, 0,
                                &PrimitiveMap);
        MemoryCopy(Primitive->CBuffer, sizeof(Primitive->CBuffer),
                   PrimitiveMap.pData, sizeof(Primitive->CBuffer));
        ID3D11DeviceContext_Unmap(Renderer->Context, Renderer->PrimitiveConsts, 0);
        //VERTEX BUFFER
        ID3D11DeviceContext_Map(Renderer->Context, Renderer->UntexturedVBuffer, 0,
                                D3D11_MAP_WRITE_DISCARD, 0,
                                &PrimitiveMap);
        MemoryCopy(Primitive->CBuffer, sizeof(Primitive->CBuffer),
                   PrimitiveMap.pData, sizeof(Primitive->CBuffer));
        ID3D11DeviceContext_Unmap(Renderer->Context, Renderer->UntexturedVBuffer, 0);
        //INDEX BUFFER
        ID3D11DeviceContext_Map(Renderer->Context, Renderer->PrimitiveConsts, 0,
                                D3D11_MAP_WRITE_DISCARD, 0,
                                &PrimitiveMap);
        MemoryCopy(Primitive->CBuffer, sizeof(Primitive->CBuffer),
                   PrimitiveMap.pData, sizeof(Primitive->CBuffer));
        ID3D11DeviceContext_Unmap(Renderer->Context, Renderer->PrimitiveConsts, 0);
        ID3D11DeviceContext_VSSetShader(Renderer->Context, Renderer->Primitive.Shader.VShader, 0, 0);
        ID3D11DeviceContext_VSSetConstantBuffers(Renderer->Context, 0, 1, &Renderer->PrimitiveConsts);
        
        
        ID3D11DeviceContext_PSSetShader(Renderer->Context, Renderer->Primitive.Shader.PShader , 0, 0);
        
        
        ID3D11DeviceContext_DrawIndexed(Renderer->Context, 6, 0, 0);
#endif
      } break;
      // NOTE(MIGUEL): Sections below are excluded for testin code above
      
      case RenderCommand_Guage:
      {
        CurrentCommandHeaderPos += (sizeof(render_command_header) + 
                                    sizeof(render_command_data_guage));
        
#if 0
        render_command_data_guage *GuageData =
          ((render_command_data_guage *)((u8 *)CommandHeader + 
                                         sizeof(render_command_header)));
        
        m4f Trans  = m4fTranslate(v3fInit(GuageData->Pos.x, GuageData->Pos.y, 0.0f));
        m4f Scale  = m4fScale(GuageData->Dim.x / 2.0f, GuageData->Dim.y / 2.0f, 1.0f);
        m4f Rotate = m4fIdentity();
        v2f ViewPos = GuageData->ViewPos;
        v2f ViewDim = GuageData->ViewDim;
        
        m4f World     = m4fMultiply(Scale, Trans);
        m4f Transform = m4f32Multiply(World, OrthoProjection);
        
        v2f WindowDim = v2fInit(Renderer->WindowWidth, Renderer->WindowHeight);
        
        //TODO(MIGUEL):Update const buffers
        
        ID3D11DeviceContext_VSSetShader(Renderer->Context, Renderer->Guage.Shader.VShader, 0, 0);
        ID3D11DeviceContext_PSSetShader(Renderer->Context, Renderer->Primitive.Shader.PShader , 0, 0);
        
        
        u32 Stride = 0;
        u32 Offset = 0;
        ID3D11DeviceContext_IASetVertexBuffers(Renderer->Context, 0, 1,
                                               &Renderer->UntexturedVBuffer, &Stride, &Offset);
        ID3D11DeviceContext_IASetInputLayout(Renderer->Context, Renderer->InputLayout);
        ID3D11DeviceContext_IASetIndexBuffer(Renderer->Context,
                                             Renderer->TexturedIBuffer, DXGI_FORMAT_R16_UINT, 0 );
        ID3D11Context_IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        
        ID3D11DeviceContext_VSSetConstantBuffers(Renderer->Context, 0, 1, &Renderer->Guage.CBuffer);
        
        // TODO(MIGUEL): figere this out
        //OPENGL_DBG(glEnable(GL_SCISSOR_TEST));
        //OPENGL_DBG(glScissor(ViewPos.x, ViewPos.y, ViewDim.x, ViewDim.y));
        //OPENGL_DBG(glEnable(GL_BLEND));
        //OPENGL_DBG(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        
        ID3D11DeviceContext_DrawElements();
        OPENGL_DBG(glDrawElementsBaseVertex(GL_TRIANGLES,
                                            6 * GuageData->QuadCount,
                                            GL_UNSIGNED_SHORT,
                                            (GLvoid *)(GuageData->IndexArrayOffset * sizeof(u16)),
                                            GuageData->UntexturedVertArrayOffset));
#endif
      } break;
      case RenderCommand_Label:
      {
        CurrentCommandHeaderPos += (sizeof(render_command_header) + 
                                    sizeof(render_command_data_label));
#if 0
        render_command_data_label *LabelData =
          ((render_command_data_label *)((u8 *)CommandHeader + 
                                         sizeof(render_command_header)));
        
        str8  String = LabelData->String;
        v2f Pos    = LabelData->Pos;
        f32   Scale  = LabelData->Scale;
        v2f ViewPos = LabelData->ViewPos;
        v2f ViewDim = LabelData->ViewDim;
        
        
        //TODO(MIGUEL):Update const buffers
        
        u32 Stride = 0;
        u32 Offset = 0;
        d3d11_label *Label = &Renderer->Label;
        ID3D11DeviceContext_IASetVertexBuffers(Renderer->Context, 0, 1,
                                               &Renderer->TexturedVBuffer, &Stride, &Offset);
        ID3D11DeviceContext_IASetInputLayout(Renderer->Context, Renderer->InputLayout);
        ID3D11DeviceContext_IASetIndexBuffer(Renderer->Context,
                                             Renderer->TexturedIBuffer, DXGI_FORMAT_R16_UINT, 0 );
        ID3D11Context_IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        ID3D11DeviceContext_VSSetShader(Renderer->Context, Label->Shader.VShader, 0, 0);
        ID3D11DeviceContext_PSSetShader(Renderer->Context, Label->Shader.PShader, 0, 0);
        
        /*
        OPENGL_DBG(glUniformMatrix4fv(LabelShader->UIDTransform, 1, 0, OrthoProjection.e));
        OPENGL_DBG(glUniform3f(glGetUniformLocation(LabelShader->ID, "TextColor"),
                               LabelData->Color.x,
                               LabelData->Color.y,
                               LabelData->Color.z));
        
        OPENGL_DBG(glActiveTexture(GL_TEXTURE0));
        */
        u32 Line = 0;
        
        
        float AdvX = Pos.x;
        for (u32 Index = 0; Index < String.Count; Index++)
        {
          u32 Char = String.Data[Index];
          
          switch((u8)Char)
          {
            case '\n':
            {
              Line++;
              AdvX = Pos.x;
            } break;
            default:
            {
              
              glyph Glyph = GlyphHashTableLookup(&GlyphHash, Char);
              
              f32 GlyphOffsetY = (Glyph.Dim.y - Glyph.Bearing.y) * Scale;
              
              f32 w = Glyph.Dim.x * Scale;
              f32 h = Glyph.Dim.y * Scale;
              u32 LineSpace = 20.0;
              
              // NOTE(MIGUEL): Origin
              f32 OrgX = AdvX + Glyph.Bearing.x * Scale;
              f32 OrgY = Pos.y - (LineSpace * Line) - GlyphOffsetY;
#if 0
              // TODO(MIGUEL): Use indeOrgXed Vertices instead. OnlOrgY After
              //               implementing a simple IMUI and rendering 
              //               api. Not a prioritOrgY.
              teOrgXtured_verteOrgX QuadVerts[4] =
              {
                { OrgX + w, OrgY + h,   1.0f, 0.0f },
                { OrgX + w, OrgY,       1.0f, 1.0f },
                { OrgX,     OrgY,       0.0f, 1.0f },
                { OrgX,     OrgY + h,   0.0f, 0.0f },            
              };
              u16 QuadIndices[6] = { 0, 1, 2, 0, 2, 3 };
              //OPENGL_DBG(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_ShORT, 0));
#else
              f32 vertices[6][4] =
              {
                { OrgX,     OrgY + h,   0.0f, 0.0f },
                { OrgX,     OrgY,       0.0f, 1.0f },
                { OrgX + w, OrgY,       1.0f, 1.0f },
                
                { OrgX,     OrgY + h,   0.0f, 0.0f },            
                { OrgX + w, OrgY,       1.0f, 1.0f },
                { OrgX + w, OrgY + h,   1.0f, 0.0f }           
              };
#endif
              glBindTexture(GL_TEXTURE_2D, Glyph.TexID);
              
              glBindVertexArray(OpenGL->TexturedVertAttribID);
              
              //POSITION ATTRIB
              OPENGL_DBG(glEnableVertexAttribArray(0));
              OPENGL_DBG(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(v2f), (GLvoid *)0));
              
              OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, OpenGL->TexturedVertBufferID));
              OPENGL_DBG((glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices)));
              
              //OPENGL_DBG(glEnable(GL_SCISSOR_TEST));
              //OPENGL_DBG(glScissor(ViewPos.x, ViewPos.y, ViewDim.x, ViewDim.y));
              OPENGL_DBG(glDrawArrays (GL_TRIANGLES, 0, 6));
              
              AdvX += (Glyph.Advance >> 6) * Scale;
            } break;
          }
        }
        
        OPENGL_DBG(glBindVertexArray(0));
        OPENGL_DBG(glBindTexture(GL_TEXTURE_2D, 0));
        OPENGL_DBG(glUseProgram(0));
#endif
      } break;
      case RenderCommand_Dbg:
      {
        CurrentCommandHeaderPos += (sizeof(render_command_header) + 
                                    sizeof(render_command_data_label));
#if 0
        render_command_data_label *LabelData =
          ((render_command_data_label *)((u8 *)CommandHeader + 
                                         sizeof(render_command_header)));
        
        str8  String = LabelData->String;
        v2f Pos    = LabelData->Pos;
        f32   Scale  = LabelData->Scale;
        v2f ViewPos = LabelData->ViewPos;
        v2f ViewDim = LabelData->ViewDim;
        
        
        OPENGL_DBG(glEnable(GL_SCISSOR_TEST));
        OPENGL_DBG(glClearColor(0.25f, 0.1f, 0.1f, 0.8f));
        OPENGL_DBG(glScissor(ViewPos.x, ViewPos.y, ViewDim.x, ViewDim.y));
        OPENGL_DBG(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
        opengl_shader_label *LabelShader = &OpenGL->LabelShader;
        OPENGL_DBG(glUseProgram(LabelShader->ID));
        
        OPENGL_DBG(glUniformMatrix4fv(LabelShader->UIDTransform, 1, 0, OrthoProjection.e));
        OPENGL_DBG(glUniform3f(glGetUniformLocation(LabelShader->ID, "TextColor"),
                               LabelData->Color.x,
                               LabelData->Color.y,
                               LabelData->Color.z));
        
        OPENGL_DBG(glActiveTexture(GL_TEXTURE0));
        
        u32 Line = 0;
        
        
        float AdvX = Pos.x;
        for (u32 Index = 0; Index < String.Count; Index++)
        {
          u32 Char = String.Data[Index];
          
          switch((u8)Char)
          {
            case '\n':
            {
              Line++;
              AdvX = Pos.x;
            } break;
            default:
            {
              
              glyph Glyph = GlyphHashTableLookup(&GlyphHash, Char);
              
              f32 GlyphOffsetY = (Glyph.Dim.y - Glyph.Bearing.y) * Scale;
              
              f32 w = Glyph.Dim.x * Scale;
              f32 h = Glyph.Dim.y * Scale;
              u32 LineSpace = 20.0;
              
              // NOTE(MIGUEL): Origin
              f32 OrgX = AdvX + Glyph.Bearing.x * Scale;
              f32 OrgY = Pos.y - (LineSpace * Line) - GlyphOffsetY;
#if 0
              // TODO(MIGUEL): Use indeOrgXed Vertices instead. OnlOrgY After
              //               implementing a simple IMUI and rendering 
              //               api. Not a prioritOrgY.
              teOrgXtured_verteOrgX QuadVerts[4] =
              {
                { OrgX + w, OrgY + h,   1.0f, 0.0f },
                { OrgX + w, OrgY,       1.0f, 1.0f },
                { OrgX,     OrgY,       0.0f, 1.0f },
                { OrgX,     OrgY + h,   0.0f, 0.0f },            
              };
              u16 QuadIndices[6] = { 0, 1, 2, 0, 2, 3 };
              //OPENGL_DBG(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_ShORT, 0));
#else
              f32 vertices[6][4] =
              {
                { OrgX,     OrgY + h,   0.0f, 0.0f },
                { OrgX,     OrgY,       0.0f, 1.0f },
                { OrgX + w, OrgY,       1.0f, 1.0f },
                
                { OrgX,     OrgY + h,   0.0f, 0.0f },            
                { OrgX + w, OrgY,       1.0f, 1.0f },
                { OrgX + w, OrgY + h,   1.0f, 0.0f }           
              };
#endif
              OPENGL_DBG(glBindTexture(GL_TEXTURE_2D, Glyph.TexID));
              
              OPENGL_DBG(glBindVertexArray(OpenGL->TexturedVertAttribID));
              
              //POSITION ATTRIB
              OPENGL_DBG(glEnableVertexAttribArray(0));
              OPENGL_DBG(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(v2f), (GLvoid *)0));
              
              OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, OpenGL->TexturedVertBufferID));
              OPENGL_DBG(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
              
              OPENGL_DBG(glDrawArrays (GL_TRIANGLES, 0, 6));
              
              AdvX += (Glyph.Advance >> 6) * Scale;
            } break;
          }
        }
        
        OPENGL_DBG(glBindVertexArray(0));
        OPENGL_DBG(glBindTexture(GL_TEXTURE_2D, 0));
        OPENGL_DBG(glDisable(GL_SCISSOR_TEST));
        OPENGL_DBG(glUseProgram(0));
#endif
      } break;
    }
  }
  
  IDXGISwapChain_Present(Renderer->SwapChain, 0, 0);
  
  return;
}
#endif

void
GlyphHashTableInit(glyph_hash *GlyphHash)
{
  GlyphHash->Count    = 0;
  GlyphHash->MaxCount = GLYPH_MAX_COUNT;
  
  MemoryZero(&GlyphHash->CharIndex, sizeof(GlyphHash->CharIndex));
  MemoryZero(&GlyphHash->TexID, sizeof(GlyphHash->TexID));
  MemoryZero(&GlyphHash->Dim, sizeof(GlyphHash->Dim));
  MemoryZero(&GlyphHash->Bearing, sizeof(GlyphHash->Bearing));
  MemoryZero(&GlyphHash->Advance, sizeof(GlyphHash->Advance));
  
  return;
}

glyph
GlyphHashTableInsert(glyph_hash *GlyphHash,
                     u32   CharIndex,
                     u32   TextureID,
                     v2s Dim,
                     v2s Bearing,
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
GlyphHashTableFill(glyph_hash *GlyphHash, d3d11_renderer *Renderer)
{
  // TODO(MIGUEL): Finish the d3d11 port!!!!!
  // NOTE(MIGUEL): Move this to app state maybe?
  FT_Library ft;
  if (FT_Init_FreeType(&ft))
  {
    OutputDebugString("FreeType Error: Could not init FreeType Library");
    Assert(0);
  }
  
  FT_Face Face;
  if (FT_New_Face(ft, "..\\res\\fonts\\cour.ttf", 0, &Face))
  {
    OutputDebugString("FreeType Error: Could not load Font");
    Assert(0);
  }
  
  FT_Set_Pixel_Sizes(Face, 0, 48);
#if RENDERER_OPENGL
  //sets byte-alignment to 1
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  
  for(u32 CharIndex = 0; CharIndex < GLYPH_MAX_COUNT; CharIndex++)
  {
    if (FT_Load_Char(Face, CharIndex, FT_LOAD_RENDER))
    {
      OutputDebugString("FreeType Error: Could not load Glyph");
      Assert(0);
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
#endif
#if RENDERER_D3D11
  
  // disable byte-alignment restriction
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  
  for(u32 CharIndex = 0; CharIndex < GLYPH_MAX_COUNT; CharIndex++)
  {
    if (FT_Load_Char(Face, CharIndex, FT_LOAD_RENDER))
    {
      OutputDebugString("FreeType Error: Could not load Glyph");
      Assert(0);
      continue;
    }
    
    DXGI_SAMPLE_DESC GlyphSampleDesc = 
    {
      .Count = 0,
      .Quality = 0,
    };
    
    D3D11_TEXTURE2D_DESC GlyphTexDesc = 
    {
      .Width = Face->glyph->bitmap.width,
      .Height= Face->glyph->bitmap.rows,
      //MipLevels, // TODO(MIGUEL): dunno
      .ArraySize = 1,
      .Format = DXGI_FORMAT_R8_UINT,
      .SampleDesc = 0,
      .Usage = 0,
      .BindFlags =  0,
      .CPUAccessFlags = 0,
      .MiscFlags = 0,
    };
    
    D3D11_SUBRESOURCE_DATA GlyphTexData =
    {
      .pSysMem = Face->glyph->bitmap.buffer,
      .SysMemPitch = 1,
      .SysMemSlicePitch = 0,
    };
    
    ID3D11Texture2D GlyphTexureID;
    ID3D11Device_CreateTexture2D(Renderer->Device,
                                 &GlyphTexDesc,
                                 &GlyphTexData,
                                 &GlyphTexureID);
    
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
#endif
  
  FT_Done_Face(Face);
  FT_Done_FreeType(ft);
  
  return;
}


#endif //DC_RENDERER_H
