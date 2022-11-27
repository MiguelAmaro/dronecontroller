#include "win32_d3d11.h"
#include <d3d11.h>

b32
win32_InitD3D11(HWND Window,
                D3D_FEATURE_LEVEL       *FeatureLevel,
                ID3D11Device           **Device,
                ID3D11DeviceContext    **Context,
                IDXGISwapChain         **SwapChain,
                ID3D11RenderTargetView **TargetView)
{
  HRESULT Status;
  b32     Result = true;
  
  UINT Flags = (D3D11_CREATE_DEVICE_BGRA_SUPPORT   |
                D3D11_CREATE_DEVICE_SINGLETHREADED |
                D3D11_CREATE_DEVICE_DEBUG);
  
  D3D_FEATURE_LEVEL Levels[] = {D3D_FEATURE_LEVEL_11_0};
  
#if 0
  // NOTE(MIGUEL): For if I want to ceate the device and swapchain seperately.
  Status = D3D11CreateDevice(0,
                             D3D_DRIVER_TYPE_HARDWARE,
                             0,
                             Flags,
                             Levels,
                             ARRAYSIZE(Levels),
                             D3D11_SDK_VERSION,
                             &AppState->Device, 0, &AppState->Context);
  
  Assert(SUCCEEDED(Status));
#endif
  
  DXGI_SWAP_CHAIN_DESC SwapChainDescription = {0};
  SwapChainDescription.BufferCount = 2;
  SwapChainDescription.BufferDesc.Width  = 0;// NOTE(MIGUEL): Let DXGIRT deal with it.
  SwapChainDescription.BufferDesc.Height = 0;
  SwapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  SwapChainDescription.BufferDesc.RefreshRate.Numerator   = 60;
  SwapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
  SwapChainDescription.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  SwapChainDescription.SwapEffect        = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  SwapChainDescription.OutputWindow       = Window;
  SwapChainDescription.Windowed           = true;
  SwapChainDescription.SampleDesc.Count   = 1;
  SwapChainDescription.SampleDesc.Quality = 0;
  
  Status = D3D11CreateDeviceAndSwapChain(0, // NOTE(MIGUEL): Get default video adapter
                                         D3D_DRIVER_TYPE_HARDWARE,
                                         0, // NOTE(MIGUEL): Handle to the dll containing the software render if one is used
                                         Flags,
                                         Levels, ARRAYSIZE(Levels),
                                         D3D11_SDK_VERSION,
                                         &SwapChainDescription,
                                         SwapChain,
                                         Device,
                                         FeatureLevel,
                                         Context );
  
  Assert(SUCCEEDED(Status));
  
  
  ID3D11Texture2D        *BackBufferTexture;
  
  
  IDXGISwapChain_GetBuffer(*SwapChain, 0,
                           &IID_ID3D11Texture2D,
                           (LPVOID *)&BackBufferTexture);
  
  Status = ID3D11Device_CreateRenderTargetView(*Device, BackBufferTexture, 0,
                                               TargetView);
  
  ID3D11Texture2D_Release(BackBufferTexture);
  
  Assert(SUCCEEDED(Status));
  
  
  ID3D11DeviceContext_OMSetRenderTargets(*Context, 1, TargetView, 0);
  
  D3D11_VIEWPORT ViewPort;
  ViewPort.TopLeftX = 0.0f;
  ViewPort.TopLeftY = 0.0f;
  ViewPort.Width  = 0;//(f32)gWindowDim.Width;
  ViewPort.Height = 0;//(f32)gWindowDim.Height;
  ViewPort.MinDepth = 0.0f;
  ViewPort.MaxDepth = 1.0f;
  
  ID3D11DeviceContext_RSSetViewports(*Context, 1, &ViewPort);
  
  return Result;
}

void D3D11InitShaderFileInfo(d3d11_shader_file_info *FileInfo, char *FilePath, u32 FilePathSize)
{
  
  WIN32_FIND_DATAA *ShaderFileInfo = &FileInfo->DataOfActive;
  
  memcpy(&FileInfo->Path, FilePath, FilePathSize);
  
  char *ShaderPath = FileInfo->Path;
  
  FindFirstFileA(ShaderPath,
                 ShaderFileInfo);
  
  HANDLE *ShaderCodeHandle = &FileInfo->FileA;
  
  *ShaderCodeHandle = CreateFileA(ShaderPath,
                                  GENERIC_READ, 0, 0,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL, 0);
  
  Assert(*ShaderCodeHandle);
  
  FileInfo->FileSize = ((ShaderFileInfo->nFileSizeHigh << 31) |
                        (ShaderFileInfo->nFileSizeLow));
  
  return;
}

// NOTE(MIGUEL): SHADER STUFFFF
b32 D3D11LoadShader(d3d11_renderer *Renderer,
                    ID3D11VertexShader **NewVertexShader,
                    ID3D11PixelShader  **NewPixelShader,
                    HANDLE ShaderCodeHandle,
                    size_t ShaderFileSize,
                    D3D11_INPUT_ELEMENT_DESC *VertexLayout,
                    u32 LayoutCount,
                    memory_arena *AssetLoadingArena)
{
  HRESULT Result;
  
  /// CREATE VERTEX SHADER
  ID3DBlob *VertexShaderBuffer;
  
  DWORD ShaderFlags =  D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
  
  ID3DBlob *ErrorBuffer;
  
  // NOTE(MIGUEL): D3DX11CompileFromFile is depricated
  
  u8 *ShaderCode = MEMORY_ARENA_PUSH_ARRAY(AssetLoadingArena,
                                           ShaderFileSize, u8);
  
  ReadFile(ShaderCodeHandle, ShaderCode, ShaderFileSize, 0, 0);
  CloseHandle(ShaderCodeHandle);
  
  Result = D3DCompile(ShaderCode, ShaderFileSize,
                      0, 0, 0, "VS_Main", "vs_4_0", ShaderFlags, 0,
                      &VertexShaderBuffer, &ErrorBuffer);
  
  if(FAILED(Result))
  {
    //OutputDebugString((LPCSTR)ID3D11ID3DBlob_GetBufferPointer(&ErrorBuffer));
    
    if(ErrorBuffer != 0)
    {
      ID3D11Buffer_Release(ErrorBuffer);
      
      return false;
    }
  }
  
  Result = ID3D11Device_CreateVertexShader(Renderer->Device, &VertexShaderBuffer,
                                           sizeof(VertexShaderBuffer), 0,
                                           NewVertexShader);
  
  Result = ID3D11Device_CreateVertexShader(Renderer->Device, &VertexShaderBuffer,
                                           sizeof(VertexShaderBuffer), 0,
                                           NewVertexShader);
  
  if(FAILED(Result))
  {
    if(VertexShaderBuffer) ID3D11Buffer_Release(VertexShaderBuffer);
    
    return false;
  }
  
  Result = ID3D11Device_CreateInputLayout(Renderer->Device, VertexLayout,
                                          LayoutCount,
                                          &VertexShaderBuffer,
                                          sizeof(VertexShaderBuffer),
                                          &Renderer->InputLayout);
  
  if(VertexShaderBuffer) ID3D11Buffer_Release(VertexShaderBuffer);
  
  /// CREATE PIXEL SHADER
  
  ID3DBlob* PixelShaderBuffer = 0;
  
  Result = D3DCompile(ShaderCode,
                      ShaderFileSize,
                      0,
                      0, 0,
                      "PS_Main",
                      "ps_4_0",
                      ShaderFlags,
                      0,
                      &PixelShaderBuffer,
                      &ErrorBuffer);
  
  if(FAILED(Result))
  {
    if(ErrorBuffer != 0)
    {
      //ErrorBuffer->Release();
      
      return false;
    }
  }
  
  Result = ID3D11Device_CreatePixelShader(Renderer->Device,
                                          &PixelShaderBuffer,
                                          sizeof(PixelShaderBuffer),
                                          0,
                                          NewPixelShader);
  
  
  if(PixelShaderBuffer) ID3D11PixelShader_Release(PixelShaderBuffer);
  
  if(FAILED(Result))
  {
    
    return false;
  }
  
  return true; 
}

void D3D11HotLoadShader(d3d11_renderer *Renderer, d3d11_shader_data *Shader, memory_arena *ShaderLoadingArena)
{
  WIN32_FIND_DATAA *CurrentShaderFileInfo = &Shader->FileInfo.DataOfActive;
  WIN32_FIND_DATAA  UpdatedShaderFileInfo = {0};
  
  d3d11_shader_file_info *FileInfo = &Shader->FileInfo;
  //char *CurrentShaderPath = Renderer->CurrentShaderPath;
  FindFirstFileA("..\\src\\default"".hlsl",
                 &UpdatedShaderFileInfo);
  
  
  if((UpdatedShaderFileInfo.ftLastWriteTime.dwLowDateTime !=
      CurrentShaderFileInfo->ftLastWriteTime.dwLowDateTime) ||
     (UpdatedShaderFileInfo.ftLastWriteTime.dwHighDateTime !=
      CurrentShaderFileInfo->ftLastWriteTime.dwHighDateTime)) 
  {
    
    ID3D11VertexShader *NewVertexShader = 0;
    ID3D11PixelShader  *NewPixelShader  = 0;
    
    if(FileInfo->FileA)
    {
      char PostFix[] = "_inuse_b.hlsl";
      
      CopyFile("..\\src\\default"".hlsl",
               "..\\src\\default""_inuse_b"".hlsl", 0);
      
      
      size_t ShaderFileSize = ((UpdatedShaderFileInfo.nFileSizeHigh << 31) |
                               (UpdatedShaderFileInfo.nFileSizeLow));
      
      Shader->FileInfo.FileB = CreateFileA("..\\src\\default""_inuse_b"".hlsl",
                                           GENERIC_READ, 0, 0,
                                           OPEN_EXISTING,
                                           FILE_FLAG_DELETE_ON_CLOSE,
                                           0);
      
      if(D3D11LoadShader(Renderer,
                         &NewVertexShader,
                         &NewPixelShader,
                         ShaderLoadingArena,
                         ShaderFileSize,
                         gVertexLayout,
                         gVertexLayoutCount,
                         ShaderLoadingArena))
      {
        ID3D11VertexShader_Release(Shader->VShader);
        ID3D11PixelShader_Release(Shader->PShader);
        
        Shader->VShader = NewVertexShader;
        Shader->PShader  = NewPixelShader;
        
      }
      
      CloseHandle(FileInfo->FileA);
      FileInfo->FileA = 0;
      
      CurrentShaderFileInfo->ftLastWriteTime =
        UpdatedShaderFileInfo.ftLastWriteTime;
    }
    else if(FileInfo->FileB)
    {
      
      CopyFile("..\\src\\default"".hlsl",
               "..\\src\\default""_inuse_a"".hlsl", 0);
      
      
      size_t ShaderFileSize = ((UpdatedShaderFileInfo.nFileSizeHigh << 31) |
                               (UpdatedShaderFileInfo.nFileSizeLow));
      
      Shader->FileInfo.FileA = CreateFileA("..\\src\\default""_inuse_a"".hlsl",
                                           GENERIC_READ, 0, 0,
                                           OPEN_EXISTING,
                                           FILE_FLAG_DELETE_ON_CLOSE,
                                           0);
      
      if(D3D11LoadShader(Renderer,
                         &NewVertexShader,
                         &NewPixelShader,
                         Shader->FileInfo.FileA,
                         ShaderFileSize,
                         gVertexLayout,
                         gVertexLayoutCount,
                         ShaderLoadingArena))
      {
        
        ID3D11VertexShader_Release(Shader->VShader);
        ID3D11PixelShader_Release(Shader->PShader);
        
        
        Shader->VShader = NewVertexShader;
        Shader->PShader = NewPixelShader;
        
      }
      
      CloseHandle(FileInfo->FileB);
      FileInfo->FileB = 0;
      
      CurrentShaderFileInfo->ftLastWriteTime = 
        UpdatedShaderFileInfo.ftLastWriteTime;
    }
  }
  
  return;
}