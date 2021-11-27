/* date = January 5th 2021 6:43 pm */
#ifndef FLIGHTCONTROL_OPENGL_H
#define FLIGHTCONTROL_OPENGL_H

#include <windows.h>
#include <GLAD/glad.h>  // NOTE(MIGUEL): Causes SATUS_ACCESS_VIOLATION! FUCK USING GLAD
//#include <GL/gl.h> 
#include <cglm/cglm.h>
#include <cglm/util.h>
#include <cglm/vec4.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>
#include "dc_platform.h"
#include <stdbool.h>
#include <WGL/wglext.h>

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
#define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506

#define GL_CheckError() GL_CheckError_(__FILE__, __LINE__)

GLenum  GL_CheckError_(readonly u8 *file, u32 line);

#define GL_Call(function) GL_ClearError();\
function;\
GL_Log(__FILE__, __LINE__, #function);


HGLRC win32_Init_OpenGL(HDC real_dc);
b32 GL_Log(readonly u8 *file, readonly u32 line, readonly u8 *function);
void GL_ClearError(void);


typedef struct opengl_render_info opengl_render_info;
struct opengl_render_info
{
    u32 VertexBufferID;
    u32 VertexAttribID;
    u32 IndexBufferID;
    u32 ShaderID;
    u32 TextureID;
    
    // Dynamic Shader Reload Data
    HANDLE InUseShaderFileA;
    HANDLE InUseShaderFileB;
    WIN32_FIND_DATAA CurrentShaderFileInfo;
    u8 ShaderPath[256];
    
    //Uniforms
    u32 UModel;
    u32 UThrottle;
    u32 UProjection;
    u32 UColor;
    u32 UInput;
    u32 UWindowSize       ;
    u32 UDeltaTime        ;
    u32 UThrottleSize     ;
    u32 UThrottlePos      ;
    u32 UThrottleTransform;
};

typedef struct opengl_renderer opengl_renderer;
struct opengl_renderer
{
    HDC   DeviceContext;
    HGLRC RealContext;
    u32   CurrentWidth;
    u32   CurrentHeight;
    
    //Uniforms
    opengl_render_info Models[64];
};

#endif //FLIGHTCONTROL_OPENGL_H
