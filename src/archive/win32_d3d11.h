/* date = February 22nd 2022 3:16 pm */

#ifndef WIN32_D3D11_H
#define WIN32_D3D11_H


#include "dc_common.h"
#include "dc_types.h"
#include "dc_render_commands.h"


D3D11_INPUT_ELEMENT_DESC gVertexLayout[3];
u32 gVertexLayoutCount;

typedef struct d3d11_shader_file_info d3d11_shader_file_info;
struct d3d11_shader_file_info
{
  HANDLE FileA;
  HANDLE FileB;
  WIN32_FIND_DATAA DataOfActive;
  u32 FileSize;
  u8 Path[256];
};


typedef struct d3d11_shader_data d3d11_shader_data;
struct d3d11_shader_data
{
  ID3D11VertexShader *VShader;
  ID3D11PixelShader  *PShader;
  d3d11_shader_file_info FileInfo;
};

typedef struct d3d11_quad d3d11_quad;
struct d3d11_quad
{
  d3d11_shader_data Shader;
  
  union
  {
    struct
    {
      v4f Color;
      m4f Transform;
    };
    u32 CBuffer[20];
  };
};

typedef struct d3d11_guage d3d11_guage;
struct d3d11_guage
{
  d3d11_shader_data Shader;
  
  union
  {
    struct
    {
      v2f WindowSize;
      f32   DeltaTime;
      
      m4f Projection;
      m4f Transform;
      
      f32 Throttle;
      v4f Color;
      u32 Input;
      v2f Size;
      v3f Pos;
    };
    u32 CBuffer[9];
    // TODO(MIGUEL): recalc cbuffer count
  };
};


typedef struct d3d11_label d3d11_label;
struct d3d11_label
{
  d3d11_shader_data Shader;
  
  union
  {
    struct
    {
      u32 WindowSize       ;
      u32 DeltaTime        ;
      
      u32 Projection;
      u32 Transform;
    };
    u32 CBuffer[4];
    // TODO(MIGUEL): recalc cbuffer count
  };
};


typedef struct d3d11_renderer d3d11_renderer;
struct d3d11_renderer
{
  u32 WindowWidth;
  u32 WindowHeight;
  f32 DeltaTime;
  
  D3D_FEATURE_LEVEL       FeatureLevel;
  ID3D11Device           *Device;
  ID3D11DeviceContext    *Context;
  IDXGISwapChain         *SwapChain;
  ID3D11RenderTargetView *TargetView;
  
  ID3D11InputLayout *InputLayout;
  ID3D11Buffer      *UntexturedVBuffer;
  ID3D11InputLayout *UntexturedVLayout;
  ID3D11Buffer      *UntexturedIBuffer; // NOTE(MIGUEL): 
  ID3D11Buffer      *TexturedVBuffer;
  ID3D11InputLayout *TexturedVLayout;
  ID3D11Buffer      *TexturedIBuffer; // NOTE(MIGUEL): shared
  
  ID3D11ShaderResourceView *GlyphTexView;
  ID3D11SamplerState       *GlyphSamplerState;
  
  render_data RenderData;
  
  ID3D11Buffer *GuageConsts;
  ID3D11Buffer *LabelConsts;
  ID3D11Buffer *PrimitiveConsts;
  
  d3d11_guage  Guage;
  d3d11_label  Label;
  d3d11_quad   Primitive;
  memory_arena Scratch;
};

b32
win32_InitD3D11(HWND Window,
                D3D_FEATURE_LEVEL       *FeatureLevel,
                ID3D11Device           **Device,
                ID3D11DeviceContext    **Context,
                IDXGISwapChain         **SwapChain,
                ID3D11RenderTargetView **TargetView);

void
D3D11InitShaderFileInfo(d3d11_shader_file_info *FileInfo,
                        char *FilePath,
                        u32 FilePathSize);
b32
D3D11LoadShader(d3d11_renderer *Renderer,
                ID3D11VertexShader **NewVertexShader,
                ID3D11PixelShader  **NewPixelShader,
                HANDLE ShaderCodeHandle,
                size_t ShaderFileSize,
                D3D11_INPUT_ELEMENT_DESC *VertexLayout,
                u32 LayoutCount,
                memory_arena *AssetLoadingArena);

#endif //WIN32_D3D11_H
