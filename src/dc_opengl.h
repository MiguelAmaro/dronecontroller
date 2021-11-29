/* date = January 5th 2021 6:43 pm */
#ifndef FLIGHTCONTROL_OPENGL_H
#define FLIGHTCONTROL_OPENGL_H

#include "win32_opengl.h"
#include "dc_types.h"
#include "dc_render_commands.h"

typedef struct opengl_shader_guage opengl_shader_guage;
struct opengl_shader_guage
{
    u32 ID;
    
    //Uniform IDs
    u32 UIDWindowSize       ;
    u32 UIDDeltaTime        ;
    
    u32 UIDProjection;
    
    u32 UIDThrottle;
    u32 UIDColor;
    u32 UIDInput;
    u32 UIDThrottleSize     ;
    u32 UIDThrottlePos      ;
    u32 UIDThrottleTransform;
    
    opengl_shader_file_info FileInfo;
};


typedef struct opengl_shader_label opengl_shader_label;
struct opengl_shader_label
{
    u32 ID;
    
    //Uniform IDs
    u32 UIDWindowSize       ;
    u32 UIDDeltaTime        ;
    
    u32 UIDProjection;
    u32 UIDThrottleTransform;
    
    opengl_shader_file_info FileInfo;
};

typedef struct opengl_renderer opengl_renderer;
struct opengl_renderer
{
    HDC   DeviceContext;
    HGLRC RealContext;
    
    u32   WindowWidth;
    u32   WindowHeight;
    
    f32 DeltaTime;
    
    u32 UntexturedVertBufferID;
    u32 UntexturedVertAttribID;
    
    u32 TexturedVertBufferID;
    u32 TexturedVertAttribID;
    
    u32 IndexBufferID; // NOTE(MIGUEL): shared
    
    
    //u32 TextureID;
    render_data RenderData;
    
    
    opengl_shader_guage GuageShader;
    opengl_shader_label LabelShader;
};


#define OpenGLCheckError() GL_CheckError_(__FILE__, __LINE__)

#define OPENGL_DBG(function) OpenGLClearError();\
function;\
OpenGLLog(__FILE__, __LINE__, #function);


GLenum  OpenGLCheckError_(readonly u8 *file, u32 line);
b32     OpenGLLog        (readonly u8 *file,
                          readonly u32 line,
                          readonly u8 *function);
void    OpenGLClearError (void);


#endif //FLIGHTCONTROL_OPENGL_H
