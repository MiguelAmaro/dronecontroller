
#include "dc_opengl.h"

#include "dc_strings.h"
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb/stb_image.h"

#define VERTSEC ("//~VERT SHADER")
#define FRAGSEC ("//~FRAG SHADER")


void
OpenGLCreateShader(u32 *ShaderID, opengl_shader_file_info *Info, u8 *Path, u32 PathSize)
{
    memset(&Info->CurrentShaderFileInfo, 0, sizeof(Info->CurrentShaderFileInfo));
    
    MemoryCopy(Path,
               PathSize,
               &Info->ShaderPath,
               ARRAY_SIZE(Info->ShaderPath));
    
    FindFirstFileA(Info->ShaderPath,
                   &Info->CurrentShaderFileInfo);
    
    
    u8 InUseShaderPath[256];
    MemoryCopy(&Info->ShaderPath,
               ARRAY_SIZE(Info->ShaderPath),
               InUseShaderPath,
               ARRAY_SIZE(InUseShaderPath));
    
    // NOTE(MIGUEL): Update Extension
    str8 A = str8Init(InUseShaderPath, PathSize);
    str8 B = str8Init("_inuse_a.glsl"   , sizeof("_inuse_a.glsl"));
    
    str8RemoveFromEndToChar('.', &A);
    str8AppendBtoA(&A, ARRAY_SIZE(InUseShaderPath), B);
    
    CopyFile(Info->ShaderPath,
             InUseShaderPath, 0);
    
    size_t ShaderFileSize = ((Info->CurrentShaderFileInfo.nFileSizeHigh << 32) |
                             (Info->CurrentShaderFileInfo.nFileSizeLow));
    
    Info->InUseShaderFileA = 0;
    Info->InUseShaderFileB = 0;
    
    Info->InUseShaderFileA = CreateFileA(InUseShaderPath,
                                         GENERIC_READ, 0, 0,
                                         OPEN_EXISTING,
                                         FILE_FLAG_DELETE_ON_CLOSE,
                                         0);
    ASSERT(Info->InUseShaderFileA);
    
    OpenGLLoadShaderFromSource(ShaderID,
                               Info->ShaderPath,
                               Info->InUseShaderFileA, ShaderFileSize);
    
    return;
}

void 
OpenGLHotSwapShader(u32 *ShaderID, opengl_shader_file_info *Info)
{
    WIN32_FIND_DATAA UpdatedShaderFileInfo = {0};
    
    
    FindFirstFileA(Info->ShaderPath,
                   &UpdatedShaderFileInfo);
    
    if((UpdatedShaderFileInfo.ftLastWriteTime.dwLowDateTime !=
        Info->CurrentShaderFileInfo.ftLastWriteTime.dwLowDateTime) ||
       (UpdatedShaderFileInfo.ftLastWriteTime.dwHighDateTime !=
        Info->CurrentShaderFileInfo.ftLastWriteTime.dwHighDateTime)) 
    {
        
        u32 NewShader = 0;
        
        if(Info->InUseShaderFileA)
        {
            u8 InUseShaderPath[256];
            MemoryCopy(&Info->ShaderPath,
                       ARRAY_SIZE(Info->ShaderPath),
                       InUseShaderPath,
                       ARRAY_SIZE(InUseShaderPath));
            
            // NOTE(MIGUEL): Update Extension
            str8 A = str8Init(InUseShaderPath, str8GetCStrLength(InUseShaderPath));
            str8 B = str8Init("_inuse_b.glsl"   , sizeof("_inuse_b.glsl"));
            
            str8RemoveFromEndToChar('.', &A);
            str8AppendBtoA(&A, ARRAY_SIZE(InUseShaderPath), B);
            
            
            CopyFile(Info->ShaderPath,
                     InUseShaderPath, 0);
            
            
            size_t ShaderFileSize = ((UpdatedShaderFileInfo.nFileSizeHigh << 32) |
                                     (UpdatedShaderFileInfo.nFileSizeLow));
            
            Info->InUseShaderFileB = CreateFileA(InUseShaderPath,
                                                 GENERIC_READ, 0, 0,
                                                 OPEN_EXISTING,
                                                 FILE_FLAG_DELETE_ON_CLOSE,
                                                 0);
            
            if(OpenGLLoadShaderFromSource(&NewShader, InUseShaderPath,
                                          Info->InUseShaderFileB, ShaderFileSize))
            {
                glDeleteShader(*ShaderID);
                *ShaderID = NewShader;
            }
            
            CloseHandle(Info->InUseShaderFileA);
            Info->InUseShaderFileA = 0;
            
            Info->CurrentShaderFileInfo.ftLastWriteTime =
                UpdatedShaderFileInfo.ftLastWriteTime;
        }
        else if(Info->InUseShaderFileB)
        {
            u8 InUseShaderPath[256];
            MemoryCopy(&Info->ShaderPath,
                       ARRAY_SIZE(Info->ShaderPath),
                       InUseShaderPath,
                       ARRAY_SIZE(InUseShaderPath));
            
            // NOTE(MIGUEL): Update Extension
            str8 A = str8Init(InUseShaderPath, str8GetCStrLength(InUseShaderPath));
            str8 B = str8Init("_inuse_a.glsl"   , sizeof("_inuse_a.glsl"));
            
            str8RemoveFromEndToChar('.', &A);
            str8AppendBtoA(&A, ARRAY_SIZE(InUseShaderPath), B);
            
            
            CopyFile(Info->ShaderPath,
                     InUseShaderPath, 0);
            
            size_t ShaderFileSize = ((UpdatedShaderFileInfo.nFileSizeHigh << 32) |
                                     (UpdatedShaderFileInfo.nFileSizeLow));
            
            Info->InUseShaderFileA = CreateFileA(InUseShaderPath,
                                                 GENERIC_READ, 0, 0,
                                                 OPEN_EXISTING,
                                                 FILE_FLAG_DELETE_ON_CLOSE,
                                                 0);
            
            if(OpenGLLoadShaderFromSource(&NewShader, InUseShaderPath,
                                          Info->InUseShaderFileA, ShaderFileSize))
            {
                glDeleteShader(*ShaderID);
                *ShaderID = NewShader;
            }
            
            CloseHandle(Info->InUseShaderFileB);
            Info->InUseShaderFileB = 0;
            
            Info->CurrentShaderFileInfo.ftLastWriteTime = UpdatedShaderFileInfo.ftLastWriteTime;
        }
    }
    
    return;
}


internaldef b32 
OpenGLCreateShaderProgram(readonly u8* vertexShaderSource, readonly u8* fragmentShaderSource, u32 *shaderProgram)
{
    
    u32 vertexShader   = glCreateShader(GL_VERTEX_SHADER);
    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    b32 Result = 1;
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
        //while(1);
        Result = 0;
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
        //while(1);
        Result = 0;
    }
    
    *shaderProgram = glCreateProgram();
    
    glAttachShader(*shaderProgram, vertexShader);
    glAttachShader(*shaderProgram, fragmentShader);
    
    glLinkProgram(*shaderProgram);
    
    //  Set Debug log Buffer to Zero
    for(u32 byte = 0; byte < 512; ++byte){
        infoLog[byte] = 0;
    }
    
    glGetProgramiv(*shaderProgram, GL_LINK_STATUS, &success);
    
    if(!success) {
        glGetProgramInfoLog(*shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER_PROGRAM::LINKING_FAILED | %s \r\n", infoLog);
        Result = 0;
    }
    
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader); 
    
    return Result;
}


internaldef b32
OpenGLLoadShaderFromSource(u32 *ShaderProgram, readonly u8 *path, HANDLE File, size_t FileSize)
{
    b32 Result = 0;
    
    ASSERT(File);
    
    u32 BytesToRead = FileSize;
    u8 *Shader      = calloc(( BytesToRead + 10 ),  sizeof(u8));
    
    ASSERT(Shader);
    
    MemorySet(Shader, (BytesToRead + 10), 0);
    
    
    // WRITE FILE INTO BUFFER
    u32 BytesRead;
    
    for(u32 i = 0; i < BytesToRead; i++)
    {
        ReadFile(File,
                 (Shader + i),
                 1,
                 &BytesRead,
                 0);
    }
    
    u32 VSpos = StringMatchKMP(Shader, BytesToRead, VERTSEC) + sizeof(VERTSEC);
    u32 FSpos = StringMatchKMP(Shader, BytesToRead, FRAGSEC) + sizeof(FRAGSEC);
    
    *(Shader + FSpos - sizeof(FRAGSEC) - 1) = '\0';
    
    Result = OpenGLCreateShaderProgram(Shader + VSpos, Shader + FSpos, ShaderProgram);
    
    free(Shader);
    
    return Result;
}

internaldef void
win32_LoadOpenGLExtensions(void)
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
    
    HWND dummy_window = CreateWindowEx(0,
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
    
    if (!dummy_window) 
    {
        printf("Failed to create dummy OpenGL window.");
    }
    
    HDC dummy_dc = GetDC(dummy_window);
    
    PIXELFORMATDESCRIPTOR desired_pixel_format = 
    {
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
    if (!pixel_format)
    {
        printf("Failed to find a suitable pixel format.");
    }
    if (!SetPixelFormat(dummy_dc, pixel_format, &desired_pixel_format)) 
    {
        printf("Failed to set the pixel format.");
    }
    
    HGLRC dummy_context = wglCreateContext(dummy_dc);
    if (!dummy_context) 
    {
        printf("Failed to create a dummy OpenGL rendering context.");
    }
    
    if (!wglMakeCurrent(dummy_dc, dummy_context)) 
    {
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
win32_InitOpenGL(HDC real_dc)
{
    win32_LoadOpenGLExtensions();
    
    // Now we can choose a pixel format the modern way, using wglChoosePixelFormatARB.
    s32 pixel_format_attribs[] = 
    {
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
    if (!num_formats) 
    {
        printf("Failed to set the OpenGL 3.3 pixel format.");
    }
    
    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(real_dc, pixel_format, sizeof(pfd), &pfd);
    if (!SetPixelFormat(real_dc, pixel_format, &pfd)) 
    {
        printf("Failed to set the OpenGL 3.3 pixel format.");
    }
    
    // Specify that we want to create an OpenGL 3.3 core profile context
    int gl_version_attribs[] = 
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
    
    HGLRC gl_version_context = wglCreateContextAttribsARB(real_dc, 0, gl_version_attribs);
    if (!gl_version_context) 
    {
        printf("Failed to create %d.%d rendering context.", gl_version_attribs[1], gl_version_attribs[3]);
    }
    
    if (!wglMakeCurrent(real_dc, gl_version_context)) 
    {
        printf("Failed to activate OpenGL %d.%d rendering context.", gl_version_attribs[1], gl_version_attribs[3]);
    }
    
    
    return gl_version_context;
}