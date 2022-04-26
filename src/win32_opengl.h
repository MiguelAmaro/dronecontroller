/* date = November 28th 2021 1:17 am */

#ifndef WIN32_OPENGL_H
#define WIN32_OPENGL_H

#include <windows.h>
#include <GLAD/glad.h>  // NOTE(MIGUEL): Causes SATUS_ACCESS_VIOLATION! FUCK USING GLAD
//#include <GL/gl.h> 
#include <WGL/wglext.h>
//#include <GL/gl.h>

#include "dc_types.h"

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092

#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B
#define GL_INVALID_FRAMEBUFFER_OPERATION          0x0506


typedef struct opengl_shader_file_info opengl_shader_file_info;
struct opengl_shader_file_info
{
  HANDLE InUseShaderFileA;
  HANDLE InUseShaderFileB;
  WIN32_FIND_DATAA CurrentShaderFileInfo;
  u8 ShaderPath[256];
};


typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc,
                                                     HGLRC hShareContext,
                                                     const int *attribList);

typedef BOOL WINAPI  wglChoosePixelFormatARB_type(HDC hdc,
                                                  const int *piAttribIList,
                                                  const FLOAT *pfAttribFList,
                                                  UINT nMaxFormats,
                                                  int *piFormats,
                                                  UINT *nNumFormats);

wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;
wglChoosePixelFormatARB_type    *wglChoosePixelFormatARB;


HGLRC win32_InitOpenGL(HDC real_dc);
b32
OpenGLCreateShader(u32 *ShaderID, opengl_shader_file_info *Info, u8 *Path, u32 PathSize);
void 
OpenGLHotSwapShader(u32 *ShaderID, opengl_shader_file_info *Info);
#endif //WIN32_OPENGL_H
