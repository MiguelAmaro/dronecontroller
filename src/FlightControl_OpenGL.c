#include "FlightControl_OpenGL.h"


// NOTE(MIGUEL): Sould I get a higher version of opengl via glad?

/*
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
//PFNGLGETSTRINGPROC glGetString;
*/

//~ WINDOWS
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
    
    //ASSERT(gladLoadGLLoader((GLADloadproc)wglGetProcAddress));
    
    wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
                                                                                     "wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress(
                                                                               "wglChoosePixelFormatARB");
    
    
    wglMakeCurrent(dummy_dc, 0);
    wglDeleteContext(dummy_context);
    ReleaseDC(dummy_window, dummy_dc);
    DestroyWindow(dummy_window);
    
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

//~ PLATFORM INDEPENDENT
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

