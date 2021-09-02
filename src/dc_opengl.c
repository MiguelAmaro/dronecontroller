#include "dc_opengl.h"

//~ PLATFORM INDEPENDENT
// TODO(MIGUEL): Change GL_ to OpenGL_
GLenum 
GL_CheckError_(readonly u8 *file, u32 line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        u8 error[100];
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  strcpy(error, "INVALID_ENUM\0" ); break;
            case GL_INVALID_VALUE:                 strcpy(error, "INVALID_VALUE\0"); break; 
            case GL_INVALID_OPERATION:             strcpy(error, "INVALID_OPERATION\0"); break;
            case GL_STACK_OVERFLOW:                strcpy(error, "STACK_OVERFLOW\0"); break;
            case GL_STACK_UNDERFLOW:               strcpy(error, "STACK_UNDERFLOW\0"); break;
            case GL_OUT_OF_MEMORY:                 strcpy(error, "OUT_OF_MEMORY\0"); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: strcpy(error, "INVALID_FRAMEBUFFER_OPERATION\0"); break;
        }
        printf("%s | %s ( %d ) \r\n", error, file, line);
    }
    return errorCode;
}

b32 GL_Log(readonly u8 *file, readonly u32 line, readonly u8* function)
{
    GLenum errorCode;
    while (errorCode = glGetError())
    {
        u8 error[100];
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  strcpy(error, "INVALID_ENUM" ); break;
            case GL_INVALID_VALUE:                 strcpy(error, "INVALID_VALUE"); break; 
            case GL_INVALID_OPERATION:             strcpy(error, "INVALID_OPERATION"); break;
            case GL_STACK_OVERFLOW:                strcpy(error, "STACK_OVERFLOW"); break;
            case GL_STACK_UNDERFLOW:               strcpy(error, "STACK_UNDERFLOW"); break;
            case GL_OUT_OF_MEMORY:                 strcpy(error, "OUT_OF_MEMORY"); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: strcpy(error, "INVALID_FRAMEBUFFER_OPERATION"); break;
        }
        printf("%s | %s ( %d ) | %s \r\n", error, file, line, function);
        
        return false;
    }
    
    return true;
}

void GL_ClearError(void)
{
    while(glGetError() != GL_NO_ERROR);
    
    return;
}

void OpenGL_VertexBuffer_Create(u32 *vertex_buffer_id, f32 *vertices, u32 size)
{
    GL_Call(glGenBuffers(1, vertex_buffer_id));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer_id));
    GL_Call(glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW));
    
    return;
}

void OpenGL_VertexBuffer_Destroy(u32 vertex_buffer_id)
{
    GL_Call(glDeleteBuffers(1, &vertex_buffer_id));
    
    return;
}

void OpenGL_VertexBuffer_Bind(u32 vertex_buffer_id)
{
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id));
    
    return;
}

void OpenGL_VertexBuffer_Unbind(void)
{
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0));
    
    return;
}

// NOTE(MIGUEL): Mimic VertexBuffer functions for index buffers

void OpenGL_IndexBuffer_Create(u32 *index_buffer_id, f32 *index_buffer, u32 count)
{
    GL_Call(glGenBuffers(1, index_buffer_id));
    GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *index_buffer_id));
    GL_Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32), index_buffer, GL_STATIC_DRAW));
    
    return;
}

void OpenGL_IndexBuffer_Destroy(u32 index_buffer_id)
{
    GL_Call(glDeleteBuffers(1, &index_buffer_id));
    
    return;
}

void OpenGL_IndexBuffer_Bind(u32 index_buffer_id)
{
    GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id));
    
    return;
}

void OpenGL_IndexBuffer_Unbind(void)
{
    GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    
    return;
}


//~ WINDOWS

// NOTE(MIGUEL): Sould I get a higher version of opengl via glad?
//PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
//PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
//PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
//PFNGLGETSTRINGPROC glGetString;

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
                                                     const int *attribList);
wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;

typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList,
                                                 const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
wglChoosePixelFormatARB_type *wglChoosePixelFormatARB;

internal void
win32_Init_OpenGL_Extensions(void)
{
    WNDCLASS dummy_window_class = {
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = DefWindowProcA,
        .hInstance = GetModuleHandle(0), // this function sus
        .lpszClassName = "Dummy_WGL_Ext_Init",
    };
    
    if (!RegisterClass(&dummy_window_class)) {
        printf("Failed to register dummy OpenGL window.");
    }
    
    HWND dummy_window = CreateWindowEx(
                                       0,
                                       dummy_window_class.lpszClassName,
                                       "Dummy OpenGL Window",
                                       0,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       0,
                                       0,
                                       dummy_window_class.hInstance,
                                       0);
    
    if (!dummy_window) {
        printf("Failed to create dummy OpenGL window.");
    }
    
    HDC dummy_dc = GetDC(dummy_window);
    
    PIXELFORMATDESCRIPTOR desired_pixel_format = {
        .nSize = sizeof(PIXELFORMATDESCRIPTOR),
        .nVersion = 1,
        .iPixelType = PFD_TYPE_RGBA,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .cColorBits = 32,
        .cAlphaBits = 8,
        .iLayerType = PFD_MAIN_PLANE,
        .cDepthBits = 24,
        .cStencilBits = 8,
    };
    
    int pixel_format = ChoosePixelFormat(dummy_dc, &desired_pixel_format);
    if (!pixel_format) {
        printf("Failed to find a suitable pixel format.");
    }
    if (!SetPixelFormat(dummy_dc, pixel_format, &desired_pixel_format)) {
        printf("Failed to set the pixel format.");
    }
    
    HGLRC dummy_context = wglCreateContext(dummy_dc);
    if (!dummy_context) {
        printf("Failed to create a dummy OpenGL rendering context.");
    }
    
    if (!wglMakeCurrent(dummy_dc, dummy_context)) {
        printf("Failed to activate dummy OpenGL rendering context.");
    }
    
    wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress("wglCreateContextAttribsARB");
    wglChoosePixelFormatARB    = (wglChoosePixelFormatARB_type*)   wglGetProcAddress("wglChoosePixelFormatARB"   );
    
    wglMakeCurrent  (dummy_dc, 0);
    wglDeleteContext(dummy_context);
    ReleaseDC       (dummy_window, dummy_dc);
    DestroyWindow   (dummy_window);
    
    return;
}

HGLRC
win32_Init_OpenGL(HDC real_dc)
{
    win32_Init_OpenGL_Extensions();
    
    // Now we can choose a pixel format the modern way, using wglChoosePixelFormatARB.
    s32 pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_STENCIL_BITS_ARB,       8,
        0
    };
    
    s32 pixel_format;
    u32 num_formats;
    wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, 
                            &pixel_format, &num_formats);
    if (!num_formats) {
        printf("Failed to set the OpenGL 3.3 pixel format.");
    }
    
    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(real_dc, pixel_format, sizeof(pfd), &pfd);
    if (!SetPixelFormat(real_dc, pixel_format, &pfd)) {
        printf("Failed to set the OpenGL 3.3 pixel format.");
    }
    
    // Specify that we want to create an OpenGL 3.3 core profile context
    int gl_version_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
    
    HGLRC gl_version_context = wglCreateContextAttribsARB(real_dc, 0, gl_version_attribs);
    if (!gl_version_context) {
        printf("Failed to create %d.%d rendering context.", gl_version_attribs[1], gl_version_attribs[3]);
    }
    
    if (!wglMakeCurrent(real_dc, gl_version_context)) {
        printf("Failed to activate OpenGL %d.%d rendering context.", gl_version_attribs[1], gl_version_attribs[3]);
    }
    
    
    return gl_version_context;
}

#include "dc_fileio.h"
#include "dc_renderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define VERTSEC ("//~VERT SHADER")
#define FRAGSEC ("//~FRAG SHADER")


u32 
OpenGL_CreateShaderProgram(readonly u8* vertexShaderSource, readonly u8* fragmentShaderSource)
{
    
    u32 vertexShader   = glCreateShader(GL_VERTEX_SHADER);
    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    s32 success;
    u8 infoLog[512];
    
    // CREATING VERTEX SHADER
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED | %s \r\n", infoLog);
        while(1);
    }
    
    
    // CREATING FRAGMENT SHADER
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    // Set Debug log Buffer to Zero
    for(u32 byte = 0; byte < 512; ++byte){
        infoLog[byte] = 0;
    }
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED | %s \r\n", infoLog);
        while(1);
    }
    
    //printf("FFFFFFFUUUUUUUUUUUCCKKKKKKKKKKKKKKKKKKK!!!!!");
    // CREATING A SHADER PROGRAM
    // AND LINKING SHADERS TO IT
    u32 shaderProgram;
    shaderProgram = glCreateProgram();
    
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    
    glLinkProgram(shaderProgram);
    
    //  Set Debug log Buffer to Zero
    for(u32 byte = 0; byte < 512; ++byte){
        infoLog[byte] = 0;
    }
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER_PROGRAM::LINKING_FAILED | %s \r\n", infoLog);
    }
    
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader); 
    
    return shaderProgram;
}


void 
OpenGL_GetShaderSource(u32 *ShaderProgram, readonly u8 *path)
{
    //printf("\n\n************************\n  %s  \n ******************** \n", path);
    // TODO(MIGUEL): get rid of all this c std library bullshit
    FILE *File = (void *)0x00;
    File = fopen(path, "r");
    ASSERT(File);
    
    //u32 j = 0;
    //u32 i = 1;
    
    u32 BytesToRead = FileIO_GetFileSize(path);
    u8 *Shader = calloc( ( BytesToRead + 10 ),  sizeof(u8));
    
    ASSERT(Shader);
    
    // ZERO INITIALIZATION
    for(u32 i = 0; i < ( BytesToRead + 10 ); i++)
    {
        *(Shader + i) = 0x00;
    }
    
    // WRITE FILE INTO BUFFER
    for(u32 i = 0; i < BytesToRead || *(Shader + i) == EOF; i++)
    {
        *(Shader + i) = fgetc(File);
    }
    
    //printf("\n\n************************\n  %s \nsize: %d \n ******************** \n", path, BytesToRead);
    //printf("Shader\n %s \n\n", Shader);
    
    u32 VSpos = StringMatchKMP(Shader, BytesToRead, VERTSEC) + sizeof(VERTSEC);
    u32 FSpos = StringMatchKMP(Shader, BytesToRead, FRAGSEC) + sizeof(FRAGSEC);
    
    *(Shader + FSpos - sizeof(FRAGSEC) - 1) = '\0';
    //*(Shader + BytesToRead) = '\0';
    
    *ShaderProgram = OpenGL_CreateShaderProgram(Shader + VSpos, Shader + FSpos);
    
    //printf("========== Vertex Shader\n%s \nDONE\n\n", Shader + VSpos);
    //printf("========== Fragment Shader\n%s \nDONE\n\n", Shader + FSpos);
    
    //printf("String Match: %d \n", StringMatchKMP(Shader, BytesToRead, ENDSEC));
    //printf("String Match: %d \n", VSpos);
    //printf("String Match: %d \n", FSpos);
    
    fclose(File);
    free(Shader);
    
    return;
}


void OpenGL_LoadTexture(opengl_render_info *render_info, b32 should_flip )
{
    // THE AFFECTS OF THIS MIGHT NOT BE APPARENT UNSLESS THERE ARE CERTAIN CONDITIONS
    GL_Call(glGenTextures(1, &render_info->texture));
    GL_Call(glBindTexture(GL_TEXTURE_2D, render_info->texture));
    // CONFIGUE OPENGL WRAPPING OPTIONS
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
    // CONFIGURE OPENGL FILTERING OPTIONS
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    
    //GL_Call(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texture_border_color)); 
    
    // LOAD TEXTURE
    s32 sprite_tex_width, sprite_tex_height, sprite_nrChannels;
    stbi_set_flip_vertically_on_load(should_flip);  
    u8 *sprite_tex_data = stbi_load("../res/images/geo.png", &sprite_tex_width, &sprite_tex_height, &sprite_nrChannels, STBI_rgb_alpha); 
    if(sprite_tex_data)
    {
        //printf("Tex Data: \n %d | %d | %s  \n", sprite_tex_width, sprite_tex_height, sprite_tex_data);
        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // NOTE(MIGUEL): NO AFFECT
        GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite_tex_width, sprite_tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, sprite_tex_data));
        GL_Call(glGenerateMipmap(GL_TEXTURE_2D));
    }
    else
    {
        printf("Failed to load texture");
    }
    
    stbi_image_free(sprite_tex_data);
    
    
    return;
}

