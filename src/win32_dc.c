#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <GLAD/glad.h>
#include "win32_dc.h"
#include "dc_program_options.h"
#include "dc_platform.h"
#include "dc_math.h"
#include "win32_directinput.c"
#include "dc_serialport.h"
#include "dc.h"
#include "dc_opengl.h"
#include "dc_renderer.h" // TODO(MIGUEL): get rid of this file
#include "fast_pipe.h" // TODO(MIGUEL): get rid of this file
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#include <WGL/wglext.h>
#include <io.h>
#include <fcntl.h>
#include <d3d11.h>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <strsafe.h>
#include <conio.h>

// TODO(MIGUEL): get freetype to work
/*
#include <ft2build.h>
#include FT_FREETYPE_H
*/
// TODO(MIGUEL): Add openCV

// global FT_Library  g_freetype_lib;

//TODO(MIGUEL): Move back to the platform layer
global BITMAPINFO g_BitmapInfo;
global void *g_BitmapMemory;
global int g_BitmapWidth;
global int g_BitmapHeight;
global int g_BytesPerPixel = 4;

global vec4 color       = { 1.0f, 1.0f, 1.0f };
global mat3 translation = GLM_MAT3_ZERO_INIT;
global mat3 scale       = GLM_MAT3_ZERO_INIT;
global mat3 rotation    = GLM_MAT3_ZERO_INIT;
global vec3 scalefactor = { 100.0f, 100.0f, 0.0f };

// NOTE(MIGUEL): this is controlling the window screen coordinates

global platform g_Platform = {0};
//
//internal void 
//win32_resize_DIB_Section(int Width, int Height);
//
//internal void 
//win32_update_Window(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height);
//
//LRESULT CALLBACK 
//win32_Main_Window_Procedure(HWND Window, UINT Message , WPARAM w_param, LPARAM l_param);
//


// NOTE(MIGUEL): DEPRECATED - moving to opengl
internal void 
win32_update_Window(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height) {
    int WindowWidth  = ClientRect->right  - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top ;
    StretchDIBits(DeviceContext,
                  0, 0, g_BitmapWidth, g_BitmapHeight,
                  0, 0, WindowWidth, WindowHeight,
                  g_BitmapMemory                   ,
                  &g_BitmapInfo                    ,
                  DIB_RGB_COLORS, SRCCOPY);
    return;
}

LRESULT CALLBACK 
win32_Main_Window_Procedure(HWND Window, UINT Message , WPARAM w_param, LPARAM l_param) {
    LRESULT Result = 0;
    
    u32 key_code  =     0;
    u32 key_index =     0;
    b32 key_down  = false; 
    
    switch(Message)
    {
        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect); //Get RECT of window excludes borders
            g_Platform.WindowWidth  = ClientRect.right  - ClientRect.left;
            g_Platform.WindowHeight = ClientRect.bottom - ClientRect.top ;
            //win32_resize_DIB_Section(Width, Height);
        }
        break;
        case WM_CLOSE:
        {
            g_Platform.QuitApp = true;
        }
        break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        }
        break;
        case WM_DESTROY:
        {
            g_Platform.QuitApp = true;
        }
        break;
        case WM_PAINT:
        {
            // NOTE(MIGUEL): WILL PAINT WITH OPEN GL NOT WINDOWS
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width  = Paint.rcPaint.right  - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top ;
            
            RECT ClientRect;
            //GetClientRect(Window, &ClientRect); //Get RECT of window excludes borders
            
            win32_update_Window(DeviceContext, &ClientRect, X, Y, Width, Height);
            EndPaint(Window, &Paint);
        }
        break;
        
        // TODO(MIGUEL): Add code for WM_KEYUP
#if 0
        case WM_KEYUP:
        {
            key_down = Message == WM_KEYDOWN;
            key_code = w_param;
            key_index = 0;
            
            if(key_code >= 'A' && key_code <= 'Z')
            { 
                key_index = KEY_a + (key_code - 'A');
            }
            
            g_Platform.AppInput[0].KeyDown[key_index] = key_down;
        }
#endif
        default:
        {
            
            OutputDebugStringA("Default\n");
            Result = DefWindowProc(Window, Message, w_param, l_param);
        }
        break;
    }
    
    return(Result);
}

internal void
win32_ProcessKeyboardMessage(app_button_state *NewState, b32 IsDown)
{
    if(NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
    
    return;
}

internal void
win32_ProcessPendingMessages(win32_State *State, app_input *Keyboard)
{
    MSG message;
    
    u32 key_code  =     0;
    u32 key_index =     0;
    b32 key_down  = false; 
    
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch(message.message)
        {
            case WM_QUIT:
            {
                g_Platform.QuitApp = true;
            }  break;
            
            case WM_SYSKEYUP:
            
            case WM_SYSKEYDOWN:
            
            case WM_MOUSEMOVE:
            {
                //g_Platform.mouse_x_direction = g_Platform.mouse_x < (l_param & 0x0000FFFF)? (u32)(1): (u32)(-1);
                g_Platform.AppInput[0].UIControls.MousePos.X = (message.lParam & 0x0000FFFF);
                
                //g_Platform.mouse_y_direction = g_Platform.mouse_y < (l_param & 0xFFFF0000 >> 16)? (u32)(-1): (u32)(1);
                g_Platform.AppInput[0].UIControls.MousePos.Y = ((message.lParam & 0xFFFF0000) >> 16);
                printf("mouse y: %f \n", g_Platform.AppInput[0].UIControls.MousePos.Y);
                
            } break;
            
            case WM_LBUTTONDOWN:
            {
                g_Platform.AppInput[0].UIControls.MouseLeftButtonDown = 1;
            } break;
            
            case WM_LBUTTONUP:
            {
                g_Platform.AppInput[0].UIControls.MouseLeftButtonDown = 0;
            } break;
            
            case WM_RBUTTONDOWN:
            
            case WM_RBUTTONUP:
            
            case WM_MOUSEHWHEEL:
            
            case WM_KEYDOWN:
            
            case WM_KEYUP:
            {
                key_down = message.message == WM_KEYDOWN;
                key_code = message.wParam;
                key_index = 0;
                
                if(key_code >= 'A' && key_code <= 'Z')
                { 
                    key_index = KEY_a + (key_code - 'A');
                }
                Keyboard->UIControls.KeyDown[key_index] = key_down;
            } break;
#if 0
            {
                u32 vk_code          = (u32)message.wParam;
                b32 was_down         = ((message.lParam & (1 << 30)) != 0);
                b32 is_down          = ((message.lParam & (1 << 31)) == 0);
                
                if(was_down != is_down)
                {
                    if     (vk_code == 'W')
                    {
                        //MoveUp
                        win32_ProcessKeyboardMessage(&keyboard->button_y, is_down);
                    }
                    else if(vk_code == 'A')
                    {
                        //MoveLeft
                        win32_ProcessKeyboardMessage(&keyboard->button_x, is_down);
                    }
                    else if(vk_code == 'S')
                    {
                        //MoveDown
                        win32_ProcessKeyboardMessage(&keyboard->button_a, is_down);
                    }
                    else if(vk_code == 'D')
                    {
                        //MoveRight
                        win32_ProcessKeyboardMessage(&keyboard->button_b, is_down);
                    }
                    
                    else if(vk_code == 'Q')
                    {
                        win32_ProcessKeyboardMessage(&keyboard->shoulder_left, is_down);
                    }
                    else if(vk_code == 'E')
                    {
                        win32_ProcessKeyboardMessage(&keyboard->shoulder_left, is_down);
                    }
                    else if(vk_code == VK_UP)
                    {
                        win32_ProcessKeyboardMessage(&keyboard->action_up, is_down);
                    }
                    else if(vk_code == VK_LEFT)
                    {
                        win32_ProcessKeyboardMessage(&keyboard->action_left, is_down);
                    }
                    else if(vk_code == VK_DOWN)
                    {
                        win32_ProcessKeyboardMessage(&keyboard->action_down, is_down);
                    }
                    else if(vk_code == VK_RIGHT)
                    {
                        win32_ProcessKeyboardMessage(&keyboard->action_right, is_down);
                    }
                    else if(vk_code == VK_ESCAPE)
                    {
                        win32_ProcessKeyboardMessage(&keyboard->button_back, is_down);
                    }
                    else if(vk_code == VK_SPACE) 
                    {
                        win32_ProcessKeyboardMessage(&keyboard->button_start, is_down);
                    }
#if SGE_INTERNAL
                    if(vk_code == 'P')
                    {
                        if(is_down)
                        {
                            g_pause= !g_pause;
                        }
                    }
                    //~ Extra:Live Loop Stuff
                    if(vk_code == 'L')
                    {
                        if(is_down)
                        {
                            if(state->input_playback_index == 0)
                            {
                                
                                if(state->input_record_index == 0)
                                {
                                    win32_input_begin_recording(state, 1);
                                }
                                else
                                {
                                    win32_input_end_recording (state  );
                                    win32_input_begin_playback(state, 1);
                                }
                            }
                            else
                            {
                                win32_input_end_playback(state);
                            }
                        }
                    }
                    
#endif
                    if(is_down)
                    {
                        
                        b32 alt_key_was_down = ( message.lParam & (1 << 29));
                        if((vk_code == VK_F4) && alt_key_was_down)
                        {
                            g_Platform.quit = true;
                        }
                        if((vk_code == VK_RETURN) && alt_key_was_down)
                        {
                            if(message.hwnd)
                            {
                                win32_toggle_fullscreen(message.hwnd );
                            }
                        }
                    }
                }
            } break;
#endif
            
            default:
            {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            } break;
        }
    }
    
    return;
}

internal void 
win32_resize_DIB_Section(int Width, int Height) {
    if(g_BitmapMemory)
    {
        VirtualFree(g_BitmapMemory, 0, MEM_RELEASE);
    }
    
    g_BitmapWidth  = Width;
    g_BitmapHeight = Height;
    
    g_BitmapInfo.bmiHeader.biSize        = sizeof(g_BitmapInfo.bmiHeader);
    g_BitmapInfo.bmiHeader.biWidth       = g_BitmapWidth;
    g_BitmapInfo.bmiHeader.biHeight      = -g_BitmapHeight;
    g_BitmapInfo.bmiHeader.biPlanes      = 1;
    g_BitmapInfo.bmiHeader.biBitCount    = 32;
    g_BitmapInfo.bmiHeader.biCompression = BI_RGB;
    
    
    int g_BitmapMemorySize = (Width * Height)*g_BytesPerPixel;
    g_BitmapMemory = VirtualAlloc(0, g_BitmapMemorySize, MEM_COMMIT,PAGE_READWRITE);
    
    return;
}

int CALLBACK 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) 
{
#if 1
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
#else
    /// Use Refterm Fast pipe
    //ASSERT(USE_FAST_PIPE_IF_AVAILABLE());
    
#endif
    
    //~ FREETYPE SETTUP
	/*
    s32 error = FT_Init_FreeType(&g_freetype_lib);
    if(error)
    {
        printf("Error initializeing freetype btich\n");
    }
    */
    
    //**************************************
    // MAIN WINDOW SETUP
    //
    // DEFINING WINDOW CLASS TO REGISTER
    //**************************************
    
    WNDCLASS WindowClass      = {0};
    WindowClass.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc   = win32_Main_Window_Procedure;
    WindowClass.hInstance     = Instance;
    //WindowClass.hIcon;
    WindowClass.lpszClassName = "MyWindowClass";
    
    
    //**************************************
    // MAIN WINDOW SETUP
    //
    // REGISTER WINDOW CLASS WITH OS
    //**************************************
    if(RegisterClass(&WindowClass)) 
    {
        //**************************************
        // MAIN WINDOW SETUP
        //
        // CREATE THE WINDOW AND DISPLAY IT
        //**************************************
        
        g_Platform.WindowWidth  = INITIAL_WINDOW_WIDTH;
        g_Platform.WindowHeight = INITIAL_WINDOW_HEIGHT;
        
        HWND Window = CreateWindowEx(0, WindowClass.lpszClassName,
                                     "Drone Controller",
                                     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                     INITIAL_WINDOW_COORD_X, INITIAL_WINDOW_COORD_Y,
                                     g_Platform.WindowWidth, g_Platform.WindowHeight,
                                     0, 0, Instance, 0);
        
        
#ifdef RENDERER_OPENGL
        HDC gl_device_context = GetDC(Window);
        HGLRC gl_real_context = win32_Init_OpenGL(gl_device_context);
        
        opengl_renderer OpenGLRenderer =
        {
            .DeviceContext = gl_device_context,
            .RealContext   = gl_real_context,
            .CurrentWidth  = g_Platform.WindowWidth,
            .CurrentHeight = g_Platform.WindowHeight,
        };
        
        
        ASSERT(gladLoadGL());
        
        u32 gl_major = 0;
        u32 gl_minor = 0;
        
        glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
        glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
        printf("OPENGL VERSION: %d.%d \n", gl_major, gl_minor);
#endif
        
        // PLATFORM INITIALIZATION
        {
            g_Platform.PermanentStorageSize = PERMANENT_STORAGE_SIZE;
            g_Platform.PermanentStorage     = VirtualAlloc(0, 
                                                           g_Platform.PermanentStorageSize,
                                                           MEM_COMMIT | MEM_RESERVE,
                                                           PAGE_READWRITE);
            
            g_Platform.TransientStorageSize = TRANSIENT_STORAGE_SIZE;
            g_Platform.TransientStorage     = VirtualAlloc(0, 
                                                           g_Platform.TransientStorageSize,
                                                           MEM_COMMIT | MEM_RESERVE,
                                                           PAGE_READWRITE);
            
            g_Platform.TargetSecondsPerFrame = 60.0f;
        }
        
        App_Init(&g_Platform);
        
        //~ INIT OPENGL RENDER STUFF
        
        // INIT SPRITE
        
        // TODO(MIGUEL): make this implicit
        f32 sprite_vertices[] =
        {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            
            1.0f, -1.0f,
            1.0f, 1.0f,
            -1.0f, 1.0f,
        };
        
        //Sprite.vertices = sprite_vertices;
#if RENDERER_OPENGL
        opengl_render_info sprite_render_info;
        OpenGL_VertexBuffer_Create(&(sprite_render_info.vertex_buffer_id), sprite_vertices, sizeof(sprite_vertices));
        
        GL_Call(glGenVertexArrays(1, &sprite_render_info.vertex_attributes_id));
        GL_Call(glBindVertexArray(sprite_render_info.vertex_attributes_id));
        
        GL_Call(glEnableVertexAttribArray(0));
        GL_Call(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void *)0x00));
        
        WIN32_FIND_DATAA CurrentShaderFileInfo = {0};
        
        FindFirstFileA("../res/shaders/throttle.glsl",
                       &CurrentShaderFileInfo);
        
        CopyFile("../res/shaders/throttle.glsl",
                 "../res/shaders/throttle_inuse_a.glsl", 0);
        
        size_t ShaderFileSize = ((CurrentShaderFileInfo.nFileSizeHigh << 32) |
                                 (CurrentShaderFileInfo.nFileSizeLow));
        
        HANDLE InUseShaderFileA = 0;
        HANDLE InUseShaderFileB = 0;
        
        InUseShaderFileA = CreateFileA("../res/shaders/throttle_inuse_a.glsl",
                                       GENERIC_READ, 0, 0,
                                       OPEN_EXISTING,
                                       FILE_FLAG_DELETE_ON_CLOSE,
                                       0);
        ASSERT(InUseShaderFileA);
        
        OpenGL_LoadShaderFromSource(&sprite_render_info.shader,
                                    "../res/shaders/throttle.glsl",
                                    InUseShaderFileA, ShaderFileSize);
        
        
        // UNBIND BUFFER
        GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
        GL_Call(glBindVertexArray(0));
        
        sprite_render_info.uniform_throttle = glGetUniformLocation(sprite_render_info.shader, "ThrottleValue"  );
        u32 WindowSizeUniform        = glGetUniformLocation(sprite_render_info.shader, "WindowSize"  );
        u32 DeltaTimeUniform         = glGetUniformLocation(sprite_render_info.shader, "DeltaTime"  );
        u32 ThrottleSizeUniform      = glGetUniformLocation(sprite_render_info.shader, "UISize"  );
        u32 ThrottlePosUniform       = glGetUniformLocation(sprite_render_info.shader, "UIPos"  );
        u32 ThrottleTransformUniform = glGetUniformLocation(sprite_render_info.shader, "Transform"  );
#endif
        
        if(Window)
        {
            MSG Message;
            
            
            win32_SerialPort_InitDevice(&g_SerialPortDevice);
            // TODO(MIGUEL): Set initialization to individual controller structures
            g_Platform.StickIsInitialized        = win32_DirectInput_init(Window, Instance);
            
            
            s64 performance_counter_frequency   = 1;
            { 
                LARGE_INTEGER freq              = { 0LL };
                
                QueryPerformanceFrequency(&freq);
                
                performance_counter_frequency = freq.QuadPart;
            }
            
            LARGE_INTEGER begin_frame_time_data = { 0LL };
            LARGE_INTEGER end_frame_time_data   = { 0LL };
            
            //**************************************
            // MAIN PROGRAM LOOP
            //
            // MESSAGE PROCESSING AND RENDERING
            //**************************************
            while(!g_Platform.QuitApp)
            {
                
                g_Platform.LastTime     = g_Platform.CurrentTime;
                g_Platform.CurrentTime += 1 / g_Platform.TargetSecondsPerFrame;
                s64 desired_frame_time_counts = performance_counter_frequency / g_Platform.TargetSecondsPerFrame;
                
                QueryPerformanceCounter(&begin_frame_time_data);
                
                /*
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        g_Platform.quit = true;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage (&Message);
                }
                */
                win32_ProcessPendingMessages(0, &g_Platform.AppInput[0]);
                
                //~ DYNAMIC RELOAD
                /// application layer reload
                
                
                /// opengl shader reload
                
                
                /// directx11 shader reload
                
                
                // ************************************************
                // INPUT
                //********=*****************************************
                
                //~ FLIGHTSTICK
                
                // NOTE(MIGUEL): rename stick_is_inithalized
                if(g_Platform.StickIsInitialized)
                {
                    win32_DirectInputProcessThrottleInput   (&g_Throttle   , &g_Platform);
                    win32_DirectInputProcessFlightStickInput(&g_Flightstick, &g_Platform);
                }
                
                //~ SERIALPORT 
                
                if(g_SerialPortDevice.Connected)
                {
                    // TODO(MIGUEL): Find out what is causing the stall.
                    win32_SerialPort_RecieveData(&g_SerialPortDevice);
                }
                else
                {
                    win32_SerialPort_InitDevice(&g_SerialPortDevice);
                }
                
                // ************************************************
                // PROCESSING
                //*************************************************
                app_backbuffer AppRenderBuffer = { 0 };
                AppRenderBuffer.Data          = g_BitmapMemory;
                AppRenderBuffer.Width         = g_BitmapWidth;
                AppRenderBuffer.Height        = g_BitmapHeight;
                AppRenderBuffer.BytesPerPixel = g_BytesPerPixel;
                
                g_Platform.QuitApp |= App_Update(&AppRenderBuffer, &g_Platform);
                
                
                // ************************************************
                // OUTPUT
                //*************************************************
                
                //~ FLIGHTSTICK
                
                //printf("HOST: Recieving - %s \n", g_SerialPort_buffer);
                
                
                //~ SERIALPORT
                
                if(g_SerialPortDevice.Connected)
                {
                    u8 ThrottleValue = 255.0f * (g_Platform.AppInput[0].DroneControls.NormalizedThrottle);
                    win32_SerialPort_SendData(&g_SerialPortDevice, &ThrottleValue, sizeof(u8));
                }
                else
                {
                    win32_SerialPort_InitDevice(&g_SerialPortDevice);
                }
                printf("%s \n\r", g_SerialPortDevice.RecieveQueue);
                
                //~ Software RENDERING
#ifdef RENDERER_SOFTWARE
                // Rendering
                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;
                GetClientRect(Window, &ClientRect); //Get RECT of window
                int WindowWidth  = ClientRect.right  - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top ;
                win32_update_Window(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);
                
#endif
                //~ OPENGL RENDERING
#ifdef RENDERER_OPENGL
                
                WIN32_FIND_DATAA UpdatedShaderFileInfo = {0};
                
                
                FindFirstFileA("../res/shaders/throttle.glsl",
                               &UpdatedShaderFileInfo);
                
                if((UpdatedShaderFileInfo.ftLastWriteTime.dwLowDateTime !=
                    CurrentShaderFileInfo.ftLastWriteTime.dwLowDateTime) ||
                   (UpdatedShaderFileInfo.ftLastWriteTime.dwHighDateTime !=
                    CurrentShaderFileInfo.ftLastWriteTime.dwHighDateTime)) 
                {
                    u32 NewShader = 0;
                    
                    if(InUseShaderFileA)
                    {
                        
                        CopyFile("../res/shaders/throttle.glsl",
                                 "../res/shaders/throttle_inuse_b.glsl", 0);
                        
                        
                        size_t ShaderFileSize = ((UpdatedShaderFileInfo.nFileSizeHigh << 32) |
                                                 (UpdatedShaderFileInfo.nFileSizeLow));
                        
                        InUseShaderFileB = CreateFileA("../res/shaders/throttle_inuse_b.glsl",
                                                       GENERIC_READ, 0, 0,
                                                       OPEN_EXISTING,
                                                       FILE_FLAG_DELETE_ON_CLOSE,
                                                       0);
                        
                        if(OpenGL_LoadShaderFromSource(&NewShader, "../res/shaders/throttle_inuse_b.glsl",
                                                       InUseShaderFileB, ShaderFileSize))
                        {
                            glDeleteShader(sprite_render_info.shader);
                            sprite_render_info.shader = NewShader;
                        }
                        
                        CloseHandle(InUseShaderFileA);
                        InUseShaderFileA = 0;
                        
                        CurrentShaderFileInfo.ftLastWriteTime =
                            UpdatedShaderFileInfo.ftLastWriteTime;
                    }
                    else if(InUseShaderFileB)
                    {
                        
                        CopyFile("../res/shaders/throttle.glsl",
                                 "../res/shaders/throttle_inuse_a.glsl", 0);
                        
                        
                        size_t ShaderFileSize = ((UpdatedShaderFileInfo.nFileSizeHigh << 32) |
                                                 (UpdatedShaderFileInfo.nFileSizeLow));
                        
                        InUseShaderFileA = CreateFileA("../res/shaders/throttle_inuse_a.glsl",
                                                       GENERIC_READ, 0, 0,
                                                       OPEN_EXISTING,
                                                       FILE_FLAG_DELETE_ON_CLOSE,
                                                       0);
                        
                        if(OpenGL_LoadShaderFromSource(&NewShader, "../res/shaders/throttle_inuse_a.glsl",
                                                       InUseShaderFileA, ShaderFileSize))
                        {
                            glDeleteShader(sprite_render_info.shader);
                            sprite_render_info.shader = NewShader;
                        }
                        
                        CloseHandle(InUseShaderFileB);
                        InUseShaderFileB = 0;
                        
                        CurrentShaderFileInfo.ftLastWriteTime = UpdatedShaderFileInfo.ftLastWriteTime;
                    }
                }
                
                if(g_Platform.WindowWidth  != OpenGLRenderer.CurrentWidth || g_Platform.WindowHeight != OpenGLRenderer.CurrentHeight)
                {
                    OpenGLRenderer.CurrentWidth  = g_Platform.WindowWidth;
                    OpenGLRenderer.CurrentHeight = g_Platform.WindowHeight;
                    
                    glViewport(0, 0, OpenGLRenderer.CurrentWidth, OpenGLRenderer.CurrentHeight);
                }
                
                glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                local_persist b32 first_render = true;
                
                // SPRITE RENDERER_01
                
                printf("%f\n", g_Platform.AppInput[0].DroneControls.NormalizedThrottle);
                app_state *AppState = (app_state *)g_Platform.PermanentStorage;
                
                entity *Entity = AppState->Entities;
                for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
                {
                    mat3 ThottleWidgetTransform = GLM_MAT3_IDENTITY_INIT;
                    
                    GL_Call(glUseProgram(sprite_render_info.shader));
                    
                    glm_translate2d(ThottleWidgetTransform, 
                                    (vec2)
                                    {
                                        2.0f *  (Entity->Pos.X / (f32)g_Platform.WindowWidth ) - 1.0f,
                                        2.0f * -(Entity->Pos.Y / (f32)g_Platform.WindowHeight) + 1.0f
                                    });
                    glm_scale2d(ThottleWidgetTransform, 
                                (vec2)
                                {
                                    Entity->Dim.X / (f32)g_Platform.WindowWidth,
                                    Entity->Dim.Y / (f32)g_Platform.WindowHeight
                                });
                    
                    local_persist f32 theta = 0.0;
                    //glm_rotate2d(ThottleWidgetTransform, glm_rad(theta));
                    theta += 0.1f;
                    /// LOCAL SPACE
                    GL_Call(glUniformMatrix3fv(ThrottleTransformUniform, 1, 0, (f32 *)ThottleWidgetTransform));
                    /// SCREEN SPACE
                    GL_Call(glUniform2fv(WindowSizeUniform , 1, (vec2)
                                         {
                                             g_Platform.WindowWidth,
                                             g_Platform.WindowHeight
                                         }));
                    GL_Call(glUniform2fv(ThrottlePosUniform , 1, (vec2)
                                         {
                                             Entity->Pos.X,//(Entity->Pos[0] / (f32)g_Platform.window_width ) ,
                                             Entity->Pos.Y//-(Entity->Pos[1] / (f32)g_Platform.window_height)
                                         }));
                    GL_Call(glUniform2fv(ThrottleSizeUniform, 1, Entity->Dim.E));
                    
                    /// EXTRA SHIT
                    GL_Call(glUniform1f(sprite_render_info.uniform_throttle, g_Platform.AppInput[0].DroneControls.NormalizedThrottle));
                    GL_Call(glUniform1f(DeltaTimeUniform, AppState->DeltaTime));
                    
                    // Enables the alpha channel
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    
                    //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
                    GL_Call(glBindVertexArray(sprite_render_info.vertex_attributes_id));
                    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
                    GL_Call(glBindVertexArray(0));
                }
                
                
                SwapBuffers(gl_device_context);
#endif
#ifdef RENDERER_D3D11
                // TODO(MIGUEL): add d3d11 implementation
#endif
                
                //~ TIMESTAMP
                QueryPerformanceCounter(&end_frame_time_data);
                
                // NOTE(MIGUEL): Wait any time, if neccssary
                // TODO(MIGUEL): think about changing target fps if current target is not met
                {
                    s64 counts_in_frame = end_frame_time_data.QuadPart - begin_frame_time_data.QuadPart;
                    s64 counts_to_wait  = desired_frame_time_counts    - counts_in_frame;
                    
                    LARGE_INTEGER begin_wait_time_data;
                    LARGE_INTEGER end_wait_time_data  ;
                    
                    QueryPerformanceCounter(&begin_wait_time_data);
                    
                    while(counts_to_wait > 0)
                    {
                        QueryPerformanceCounter(&end_wait_time_data);
                        counts_to_wait      -= end_wait_time_data.QuadPart - begin_wait_time_data.QuadPart;
                        begin_wait_time_data = end_wait_time_data;
                    }
                }
            }
            
            win32_SerialPort_CloseDevice(&g_SerialPortDevice);
            
            FreeConsole();
        }
        else
        {
            //Logging
        }
    }else
    {
        //Logging
    }
    
    return (0);
}
