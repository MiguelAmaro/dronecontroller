#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <GLAD/glad.h>
#include "dc_Program_Options.h"
#include "win32_directinput.c"
#include "dc_serialport.h"
#include "dc_platform.h"
#include "dc_opengl.h"
#include "dc.c"
#include <stdio.h>
#include <string.h>
#include "LAL.h"
#include <GL/gl.h>
#include <WGL/wglext.h>
#include <io.h>
#include <fcntl.h>
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
global BITMAPINFO BitmapInfo;
global void *BitmapMemory;
global int BitmapWidth;
global int BitmapHeight;
global int BytesPerPixel = 4;


global Entity Sprite    = {0};
global Render_info sprite_render_info;
global vec4 color       = { 1.0f, 1.0f, 1.0f };
global mat4 translation = GLM_MAT4_ZERO_INIT;
global mat4 scale       = GLM_MAT4_ZERO_INIT;
global mat4 rotation    = GLM_MAT4_ZERO_INIT;
global vec3 scalefactor = { 100.0f, 100.0f, 0.0f };

#define INITIAL_WINDOW_WIDTH  (400)
#define INITIAL_WINDOW_HEIGHT (400)

global Platform global_platform = {0};

internal void 
win32_resize_DIB_Section(int Width, int Height);

internal void 
win32_update_Window(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height);

LRESULT CALLBACK 
win32_Main_Window_Procedure(HWND Window, UINT Message , WPARAM w_param, LPARAM l_param);


int CALLBACK 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) 
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    
    HANDLE Debug_console = GetStdHandle(STD_OUTPUT_HANDLE );
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
        
        HWND Window = CreateWindowEx(0, WindowClass.lpszClassName,
                                     "FlighControl",
                                     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                     INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT,
                                     400, 400,
                                     0, 0, Instance, 0);
        
        
        HDC gl_device_context = GetDC(Window);
        
        HGLRC gl_real_context = win32_Init_OpenGL(gl_device_context);
        
        ASSERT(gladLoadGL());
        
        u32 gl_major = 0;
        u32 gl_minor = 0;
        
        glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
        glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
        printf("OPENGL VERSION: %d.%d \n", gl_major, gl_minor);
        
        // PLATFORM INITIALIZATION
        {
            global_platform.permanent_storage_size = PERMANENT_STORAGE_SIZE;
            global_platform.permanent_storage      = VirtualAlloc(0, 
                                                                  global_platform.permanent_storage_size,
                                                                  MEM_COMMIT | MEM_RESERVE,
                                                                  PAGE_READWRITE);
            
            global_platform.frames_per_second_target = 60.0f;
        }
        
        App_Init(&global_platform);
        
        //~ INIT OPENGL RENDER STUFF
        
        // INIT SPRITE
        
        f32 sprite_vertices[] = { 
            // pos     
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
        };
        
        Sprite.vertices = sprite_vertices;
        
        OpenGL_VertexBuffer_Create(&(sprite_render_info.vertex_buffer_id), sprite_vertices, sizeof(sprite_vertices));
        
        GL_Call(glGenVertexArrays(1, &sprite_render_info.vertex_attributes_id));
        GL_Call(glBindVertexArray(sprite_render_info.vertex_attributes_id));
        
        GL_Call(glEnableVertexAttribArray(0));
        GL_Call(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void *)0x00));
        
        // THIS SHADER MAyBE FUCKED UP
        Shader_get_source(&sprite_render_info.shader, "../res/shaders/SpriteShader.glsl");
        
        // UNBIND BUFFER
        GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
        GL_Call(glBindVertexArray(0));
        
        sprite_render_info.uniform_throttle = glGetUniformLocation(sprite_render_info.shader, "throttle_value"  );
        u32 throttle_size = glGetUniformLocation(sprite_render_info.shader, "size"  );
        
        
        if(Window)
        {
            MSG Message;
            
            
            global_platform.serialport_is_initialized = win32_SerialPort_device_init ();
            global_platform.     stick_is_initialized = win32_DirectInput_init(Window, Instance);
            
            
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
            while(!global_platform.quit)
            {
                
                global_platform.last_time     = global_platform.current_time;
                global_platform.current_time += 1 / global_platform.frames_per_second_target;
                s64 desired_frame_time_counts = performance_counter_frequency / global_platform.frames_per_second_target;
                
                QueryPerformanceCounter(&begin_frame_time_data);
                
                
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        global_platform.quit = true;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage (&Message);
                }
                
                
                // ************************************************
                // INPUT
                //********=*****************************************
                
                //~ FLIGHTSTICK
                
                // NOTE(MIGUEL): rename stick_is_inithalized
                if(global_platform.stick_is_initialized)
                {
                    win32_DirectInput_throttle_poll   (g_throttle   , &global_platform);
                    win32_DirectInput_flightstick_poll(g_flightstick, &global_platform);
                }
                
                //~ SERIALPORT 
                
                if(global_platform.serialport_is_initialized)
                {
                    win32_SerialPort_recieve_data();
                }
                
                
                // ************************************************
                // PROCESSING
                //*************************************************
                global_platform.quit |= App_Update(&global_platform);
                
                
                // ************************************************
                // OUTPUT
                //*************************************************
                
                //~ FLIGHTSTICK
                
                //printf("HOST: Recieving - %s \n", g_SerialPort_buffer);
                
                
                //~ SERIALPORT
                
                if(global_platform.serialport_is_initialized)
                {
                    win32_SerialPort_send_data( &(app->throttle_value), sizeof(u8));
                }
                printf("%s \n\r", g_SerialPort_buffer);
                
                //~ Software RENDERING
                {
                    /*
                    RenderWeirdGradient(XOffset - Joystick_State.lX * .5,
                                        YOffset - Joystick_State.lY * .5);
                    
                    // Rendering
                    HDC DeviceContext = GetDC(Window);
                    RECT ClientRect;
                    GetClientRect(Window, &ClientRect); //Get RECT of window
                    int WindowWidth  = ClientRect.right  - ClientRect.left;
                    int WindowHeight = ClientRect.bottom - ClientRect.top ;
                    win32_update_Window(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
                    ReleaseDC(Window, DeviceContext);
                    
                    ++YOffset;
                    ++XOffset;
                    */
                }
                
                //~ OPENGL RENDERING
                
                glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                local_persist b32 first_render = true;
                
                // SPRITE RENDERER_01
                
                mat4 model = GLM_MAT4_ZERO_INIT; // constructor
                
                GL_Call(glUseProgram(sprite_render_info.shader));
                
                if(first_render)
                {
                    glm_translate_make(translation, (vec3){200.0f, 200.0f, 0.0f});
                    glm_scale_make    (scale      , (vec3){200.0f, 200.0f, 1.0f});
                    glm_rotate_make   (rotation   , glm_rad(10.0f) ,(vec3){0.0f, 1.0f, 0.0f});
                    
                    Helpers_Display_Matrix4(translation, "Translate Matrix");
                    Helpers_Display_Matrix4(scale      , "Scale Matrix"    );
                    Helpers_Display_Matrix4(rotation   , "Rotate Matrix"   );
                    
                    first_render = false;
                }
                
                // SPRITE RENDERER_01
                
                glm_mat4_mulN((mat4 *[]){&translation, &scale }, 2, model);
                
                //glm_mat4_print(model, stdout);
                u32 size_x = 1000;
                u32 size_y = 1000;
                printf("%f\n", platform->throttle);
                GL_Call(glUniform1f(sprite_render_info.uniform_throttle, platform->throttle));
                GL_Call(glUniform2ui(throttle_size, size_x, size_y));
                
                // Enables the alpha channel
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
                
                GL_Call(glBindVertexArray(sprite_render_info.vertex_attributes_id));
                GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
                GL_Call(glBindVertexArray(0));
                
                
                SwapBuffers(gl_device_context);
                
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
            
            win32_SerialPort_close_device();
            
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
            global_platform.window_width  = ClientRect.right  - ClientRect.left;
            global_platform.window_height = ClientRect.bottom - ClientRect.top ;
            //win32_resize_DIB_Section(Width, Height);
        }
        break;
        case WM_CLOSE:
        {
            global_platform.quit = true;
        }
        break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        }
        break;
        case WM_DESTROY:
        {
            global_platform.quit = true;
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
        case WM_KEYDOWN:
        {
            key_down = Message == WM_KEYDOWN;
            key_code = w_param;
            key_index = 0;
            
            if(key_code >= 'A' && key_code <= 'Z')
            { 
                key_index = KEY_a + (key_code - 'A');
            }
            global_platform.key_down[key_index] = key_down;
        }
        break;
        // TODO(MIGUEL): Add code for WM_KEYUP
        case WM_KEYUP:
        {
            key_down = Message == WM_KEYDOWN;
            key_code = w_param;
            key_index = 0;
            
            if(key_code >= 'A' && key_code <= 'Z')
            { 
                key_index = KEY_a + (key_code - 'A');
            }
            global_platform.key_down[key_index] = key_down;
        }
        case WM_MOUSEMOVE:
        {
            //global_platform.mouse_x_direction = global_platform.mouse_x < (l_param & 0x0000FFFF)? (u32)(1): (u32)(-1);
            global_platform.mouse_x = (l_param & 0x0000FFFF);
            
            //global_platform.mouse_y_direction = global_platform.mouse_y < (l_param & 0xFFFF0000 >> 16)? (u32)(-1): (u32)(1);
            global_platform.mouse_y = ((l_param & 0xFFFF0000) >> 16);
            
        }
        break;
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
win32_resize_DIB_Section(int Width, int Height) {
    if(BitmapMemory)
    {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }
    
    BitmapWidth  = Width;
    BitmapHeight = Height;
    
    BitmapInfo.bmiHeader.biSize        = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth       = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight      = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes      = 1;
    BitmapInfo.bmiHeader.biBitCount    = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
    
    
    int BitmapMemorySize = (Width * Height)*BytesPerPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT,PAGE_READWRITE);
    
    return;
}


// NOTE(MIGUEL): DEPRECATED - moving to opengl
internal void 
win32_update_Window(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height) {
    int WindowWidth  = ClientRect->right  - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top ;
    StretchDIBits(DeviceContext,
                  0, 0, BitmapWidth, BitmapHeight,
                  0, 0, WindowWidth, WindowHeight,
                  BitmapMemory                   ,
                  &BitmapInfo                    ,
                  DIB_RGB_COLORS, SRCCOPY);
    return;
}

