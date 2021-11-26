#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <GLAD/glad.h>

#include "win32_dc.h"
#include "win32_directinput.c"

#include "dc_config.h"
#include "dc.h"
#include "dc_math.h"
#include "dc_memory.h"
#include "dc_opengl.h"
#include "dc_platform.h"
#include "dc_serialport.h"

#include <GL/gl.h>
#include <WGL/wglext.h>

//#include "fast_pipe.h" // TODO(MIGUEL): get rid of this file
//#include <io.h>
//#include <stdio.h>
//#include <fcntl.h>
//#include <string.h>
//#include <d3d11.h>
//#include <Winsock2.h>
//#include <Ws2tcpip.h>
//#include <strsafe.h>
//#include <conio.h>

#include <ft2build.h>
#include FT_FREETYPE_H

global platform    g_Platform   = {0};
global win32_state g_Win32State = {0};
global win32_backbuffer g_BackBuffer = {0};

typedef struct string8 string8;
struct string8
{
    u8 *Data;
    u32 Size;
};

internaldef void
GlyphHashTableInit(app_state *AppState)
{
    AppState->GlyphHash.Count    = 0;
    AppState->GlyphHash.MaxCount = 256;
    
    return;
}

internaldef glyph
GlyphHashTableInsert(glyph_hash *GlyphHash,
                     u32   CharIndex,
                     u32   TextureID,
                     v2s32 Dim,
                     v2s32 Bearing,
                     u32   Advance)
{
    u32 GlyphHashIndex = CharIndex % GlyphHash->MaxCount;
    
    glyph Found = { 0 };
    
    
    for(u32 NumVisited = 0;  NumVisited < GlyphHash->MaxCount; NumVisited++)
    {
        u32 Index = (GlyphHashIndex + NumVisited) % GlyphHash->MaxCount;
        
        if(GlyphHash->CharIndex[Index] == 0)
        {
            GlyphHash->CharIndex[Index] = CharIndex;
            GlyphHash->TexID    [Index] = TextureID;
            GlyphHash->Dim      [Index] = Dim;
            GlyphHash->Bearing  [Index] = Bearing;
            GlyphHash->Advance  [Index] = Advance;
            break;
        }
    }
    
    return Found;
}


internaldef glyph
GlyphHashTableLookup(glyph_hash *GlyphHash, u32   CharIndex)
{
    u32 GlyphHashIndex = CharIndex % GlyphHash->MaxCount;
    
    glyph Found = { 0 };
    
    for(u32 NumVisited = 0;  NumVisited < GlyphHash->MaxCount; NumVisited++)
    {
        u32 Index = (GlyphHashIndex + NumVisited) % GlyphHash->MaxCount;
        
        if(GlyphHash->CharIndex[Index] == CharIndex)
        {
            Found.CharIndex = GlyphHash->CharIndex[Index];
            Found.TexID     = GlyphHash->TexID  [Index];
            Found.Dim       = GlyphHash->Dim    [Index];
            Found.Bearing   = GlyphHash->Bearing[Index];
            Found.Advance   = GlyphHash->Advance[Index];
            break;
        }
    }
    
    
    return Found;
}


internaldef void
GlyphHashTableFill(app_state *AppState)
{
    // NOTE(MIGUEL): Move this to app state maybe?
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        OutputDebugString("FreeType Error: Could not init FreeType Library");
        ASSERT(0);
    }
    
    FT_Face Face;
    if (FT_New_Face(ft, "..\\res\\fonts\\cour.ttf", 0, &Face))
    {
        OutputDebugString("FreeType Error: Could not load Font");
        ASSERT(0);
    }
    
    FT_Set_Pixel_Sizes(Face, 0, 48);
    
    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    for(u32 CharIndex = 0; CharIndex < 128; CharIndex++)
    {
        if (FT_Load_Char(Face, CharIndex, FT_LOAD_RENDER))
        {
            OutputDebugString("FreeType Error: Could not load Glyph");
            ASSERT(0);
            continue;
        }
        
        u32 TextureID;
        glGenTextures(1, &TextureID);
        glBindTexture(GL_TEXTURE_2D, TextureID);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RED,
                     Face->glyph->bitmap.width,
                     Face->glyph->bitmap.rows,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     Face->glyph->bitmap.buffer);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        GlyphHashTableInsert(&AppState->GlyphHash,
                             CharIndex,
                             TextureID,
                             v2s32Init(Face->glyph->bitmap.width, Face->glyph->bitmap.rows),
                             v2s32Init(Face->glyph->bitmap_left , Face->glyph->bitmap_top),
                             Face->glyph->advance.x);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    FT_Done_Face(Face);
    FT_Done_FreeType(ft);
    
    return;
}

void RenderText(glyph_hash *GlyphHash, opengl_render_info *Info, str8 Text, f32 x, f32 y, f32 Scale, v3f32 Color)
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    m4f32 GlyphProjection = m4f32Orthographic(0.0f, g_Platform.WindowWidth,
                                              0.0f, g_Platform.WindowHeight,
                                              0.1f, 100.0f);
    
    m4f32 GlyphTransform = GlyphProjection;
    
    GL_Call(glUseProgram(Info->ShaderID));
    
    GL_Call(glUniformMatrix4fv(Info->UThrottleTransform, 1, 0, GlyphTransform.e));
    glUniform3f(glGetUniformLocation(Info->ShaderID, "TextColor"), Color.x, Color.y, Color.z);
    
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(Info->VertexAttribID);
    
    for (u32 Index = 0; Index < Text.Count; Index++)
    {
        u32 Char = Text.String[Index];
        
        glyph Glyph = GlyphHashTableLookup(GlyphHash, Char);
        
        float xpos = x + Glyph.Bearing.x * Scale;
        float ypos = y - (Glyph.Dim.y - Glyph.Bearing.y) * Scale;
        
        float w = Glyph.Dim.x * Scale;
        float h = Glyph.Dim.y * Scale;
        
        float vertices[6][4] =
        {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, Glyph.TexID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, Info->VertexBufferID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (Glyph.Advance >> 6) * Scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
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
            GetClientRect(Window, &ClientRect);
            HDC DeviceContext = GetDC(Window);
            g_Platform.WindowWidth  = ClientRect.right  - ClientRect.left;
            g_Platform.WindowHeight = ClientRect.bottom - ClientRect.top ;
        } break;
        
        case WM_CLOSE:
        {
            g_Platform.QuitApp = true;
        } break;
        
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        
        case WM_DESTROY:
        {
            g_Platform.QuitApp = true;
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            g_Platform.WindowWidth  = ClientRect.right  - ClientRect.left;
            g_Platform.WindowHeight = ClientRect.bottom - ClientRect.top ;
            EndPaint(Window, &Paint);
        } break;
        
        default:
        {
            OutputDebugStringA("Default\n");
            Result = DefWindowProc(Window, Message, w_param, l_param);
        } break;
    }
    
    return(Result);
}

internaldef void
win32_ProcessKeyboardMessage(app_button_state *NewState, b32 IsDown)
{
    if(NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
    
    return;
}

internaldef void
win32_ProcessPendingMessages(win32_state *State, app_input *Keyboard)
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
                g_Platform.AppInput[0].UIControls.MousePos.x = (message.lParam & 0x0000FFFF);
                g_Platform.AppInput[0].UIControls.MousePos.y = ((message.lParam & 0xFFFF0000) >> 16);
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

int CALLBACK 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) 
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    
    WNDCLASS WindowClass      = {0};
    WindowClass.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc   = win32_Main_Window_Procedure;
    WindowClass.hInstance     = Instance;
    //WindowClass.hIcon;
    WindowClass.lpszClassName = "MyWindowClass";
    
    if(RegisterClass(&WindowClass)) 
    {
        g_Platform.WindowWidth  = INITIAL_WINDOW_WIDTH;
        g_Platform.WindowHeight = INITIAL_WINDOW_HEIGHT;
        
        HWND Window = CreateWindowEx(0, WindowClass.lpszClassName,
                                     "Drone Controller",
                                     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                     INITIAL_WINDOW_COORD_X, INITIAL_WINDOW_COORD_Y,
                                     g_Platform.WindowWidth, g_Platform.WindowHeight,
                                     0, 0, Instance, 0);
        
#if RENDERER_OPENGL
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
        //OutputDebugStringA("OPENGL VERSION: %d.%d \n", gl_major, gl_minor);
#endif
        
        // PLATFORM INITIALIZATION
        {
            g_Win32State.MainMemoryBlockSize = (PERMANENT_STORAGE_SIZE +
                                                TRANSIENT_STORAGE_SIZE);
            
            g_Win32State.MainMemoryBlock = VirtualAlloc(0, g_Win32State.MainMemoryBlockSize,
                                                        MEM_COMMIT | MEM_RESERVE,
                                                        PAGE_READWRITE);
            
            g_Platform.PermanentStorageSize = PERMANENT_STORAGE_SIZE;
            g_Platform.PermanentStorage     = g_Win32State.MainMemoryBlock;
            
            g_Platform.TransientStorageSize = TRANSIENT_STORAGE_SIZE;
            g_Platform.TransientStorage     = ((u8 *)g_Win32State.MainMemoryBlock +
                                               g_Platform.PermanentStorageSize);
            
            g_Platform.TargetSecondsPerFrame = 60.0f;
        }
        
        App_Init(&g_Platform);
        
        //~ INIT OPENGL RENDER STUFF
        f32 SpriteVerts[] =
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
        opengl_render_info *Sprite = &OpenGLRenderer.Models[0];
        opengl_render_info *Glyph  = &OpenGLRenderer.Models[1];
        
        OpenGL_CreateVertexBuffer(&Sprite->VertexBufferID,
                                  SpriteVerts,
                                  sizeof(SpriteVerts),
                                  GL_STATIC_DRAW);
        
        GL_Call(glGenVertexArrays(1, &Sprite->VertexAttribID));
        GL_Call(glBindVertexArray(    Sprite->VertexAttribID));
        
        GL_Call(glEnableVertexAttribArray(0));
        GL_Call(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void *)0x00));
        
        OpenGL_CreateShader(Sprite,
                            "../res/shaders/throttle.glsl",
                            sizeof("../res/shaders/throttle.glsl"));
        
        // UNBIND BUFFER
        GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
        GL_Call(glBindVertexArray(0));
        
        Sprite->UThrottle            = glGetUniformLocation(Sprite->ShaderID, "ThrottleValue"  );
        
        Sprite->UWindowSize        = glGetUniformLocation(Sprite->ShaderID, "WindowSize"  );
        Sprite->UDeltaTime         = glGetUniformLocation(Sprite->ShaderID, "DeltaTime"  );
        Sprite->UThrottleSize      = glGetUniformLocation(Sprite->ShaderID, "UISize"  );
        Sprite->UThrottlePos       = glGetUniformLocation(Sprite->ShaderID, "UIPos"  );
        Sprite->UThrottleTransform = glGetUniformLocation(Sprite->ShaderID, "Transform"  );
        //-
        OpenGL_CreateVertexBuffer(&Glyph->VertexBufferID,
                                  NULL,
                                  sizeof(f32) * 6 * 4,
                                  GL_DYNAMIC_DRAW);
        
        GL_Call(glGenVertexArrays(1, &Glyph->VertexAttribID));
        GL_Call(glBindVertexArray(    Glyph->VertexAttribID));
        
        GL_Call(glEnableVertexAttribArray(0));
        GL_Call(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)0x00));
        
        // UNBIND BUFFER
        GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
        GL_Call(glBindVertexArray(0));
        
        OpenGL_CreateShader(Glyph,
                            "../res/shaders/text.glsl",
                            sizeof("../res/shaders/text.glsl"));
        
        Glyph->UThrottle            = glGetUniformLocation(Glyph->ShaderID, "ThrottleValue"  );
        
        Glyph->UWindowSize        = glGetUniformLocation(Glyph->ShaderID, "WindowSize"  );
        Glyph->UDeltaTime         = glGetUniformLocation(Glyph->ShaderID, "DeltaTime"  );
        Glyph->UThrottleSize      = glGetUniformLocation(Glyph->ShaderID, "UISize"  );
        Glyph->UThrottlePos       = glGetUniformLocation(Glyph->ShaderID, "UIPos"  );
        Glyph->UThrottleTransform = glGetUniformLocation(Glyph->ShaderID, "Transform"  );
        
#endif
        
        if(Window)
        {
            app_state *AppState = (app_state *)g_Platform.PermanentStorage;
            GlyphHashTableInit(AppState);
            GlyphHashTableFill(AppState);
            
            win32_SerialPort_InitDevice(&g_Win32State, &g_SerialPortDevice);
            // TODO(MIGUEL): Set initialization to individual controller structures
            g_Platform.StickIsInitialized        = win32_DirectInput_init(Window, Instance);
            
            
            s64 performance_counter_frequency   = 1;
            LARGE_INTEGER freq              = { 0LL };
            QueryPerformanceFrequency(&freq);
            performance_counter_frequency = freq.QuadPart;
            
            LARGE_INTEGER begin_frame_time_data = { 0LL };
            LARGE_INTEGER end_frame_time_data   = { 0LL };
            
            /// MAIN LOOP
            while(!g_Platform.QuitApp)
            {
                
                g_Platform.LastTime     = g_Platform.CurrentTime;
                g_Platform.CurrentTime += 1 / g_Platform.TargetSecondsPerFrame;
                s64 desired_frame_time_counts = performance_counter_frequency / g_Platform.TargetSecondsPerFrame;
                QueryPerformanceCounter(&begin_frame_time_data);
                
                
                win32_ProcessPendingMessages(0, &g_Platform.AppInput[0]);
                
                //~ INPUT
                
                //- FLIGHTSTICK
                
                // NOTE(MIGUEL): rename stick_is_inithalized
                if(g_Platform.StickIsInitialized)
                {
                    win32_DirectInputProcessThrottleInput   (&g_Throttle   , &g_Platform);
                    win32_DirectInputProcessFlightStickInput(&g_Flightstick, &g_Platform);
                }
                
                //- SERIALPORT 
                
                if(g_SerialPortDevice.Connected)
                {
                    // TODO(MIGUEL): Find out what is causing the stall.
                    win32_SerialPort_RecieveData(&g_SerialPortDevice);
                }
                else
                {
                    win32_SerialPort_InitDevice(&g_Win32State, &g_SerialPortDevice);
                }
                
                //~ PROCESSING
                app_backbuffer AppRenderBuffer = { 0 };
                AppRenderBuffer.Data          = g_BackBuffer.Data;
                AppRenderBuffer.Width         = g_BackBuffer.Width;
                AppRenderBuffer.Height        = g_BackBuffer.Height;
                AppRenderBuffer.BytesPerPixel = g_BackBuffer.BytesPerPixel;
                
                g_Platform.QuitApp |= App_Update(&AppRenderBuffer, &g_Platform);
                
                
                
                
                //~ OUTPUT
                
                //- SERIALPORT
                
                if(g_SerialPortDevice.Connected)
                {
                    f32 ThrottleValue = g_Platform.AppInput[0].DroneControls.NormalizedThrottle;
                    
                    telem_packet Packet = { 0 };
                    Packet.Header.Info = (u8)((Telem_Data << 6) |
                                              (Telem_f32  << 3)); 
                    
                    MemoryCopy((u8 *)&ThrottleValue, sizeof(f32),
                               Packet.Payload, 256);
                    
                    win32_SerialPort_SendData(&g_SerialPortDevice, Packet);
                }
                else
                {
                    win32_SerialPort_InitDevice(&g_Win32State, &g_SerialPortDevice);
                }
                if(g_SerialPortDevice.Connected)
                {
                    
                    //printf("%d \n\r", (u32)g_SerialPortDevice.RecieveQueue);
                }
                
                //- SOFTWARE RENDERING
#if RENDERER_SOFTWARE
                // Rendering
                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;
                GetClientRect(Window, &ClientRect); //Get RECT of window
                int WindowWidth  = ClientRect.right  - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top ;
                win32_update_Window(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);
                
#endif
                //- OPENGL RENDERING
#if RENDERER_OPENGL
                OpenGL_HotSwapShader(&OpenGLRenderer.Models[0]);
                OpenGL_HotSwapShader(&OpenGLRenderer.Models[1]);
                
                if(g_Platform.WindowWidth  != OpenGLRenderer.CurrentWidth || g_Platform.WindowHeight != OpenGLRenderer.CurrentHeight)
                {
                    OpenGLRenderer.CurrentWidth  = g_Platform.WindowWidth;
                    OpenGLRenderer.CurrentHeight = g_Platform.WindowHeight;
                    
                    glViewport(0, 0, OpenGLRenderer.CurrentWidth, OpenGLRenderer.CurrentHeight);
                }
                
                glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                app_state *AppState = (app_state *)g_Platform.PermanentStorage;
                
                entity *Entity = AppState->Entities;
                for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
                {
                    m4f32 ThrottleWidgetProjection = m4f32Orthographic(0.0f, g_Platform.WindowWidth,
                                                                       0.0f, g_Platform.WindowHeight,
                                                                       0.1f, 100.0f);
                    
                    
                    m4f32 ThrottleWidgetTransform = m4f32Identity();
                    m4f32 Trans  = m4f32Translate(v3f32Init(Entity->Pos.x, Entity->Pos.y, 0.0f));
                    m4f32 Scale  = m4f32Scale(Entity->Dim.x / 2.0f, Entity->Dim.y / 2.0f, 1.0f);
                    m4f32 Rotate = m4f32Identity();
                    
                    m4f32 World = m4f32Multiply(Scale, Trans);
                    ThrottleWidgetTransform = m4f32Multiply(World, ThrottleWidgetProjection);
                    
                    GL_Call(glUseProgram(Sprite->ShaderID));
                    GL_Call(glUniformMatrix4fv(Sprite->UThrottleTransform, 1, 0, ThrottleWidgetTransform.e));
                    GL_Call(glUniform2fv(Sprite->UWindowSize , 1, v2f32Init(g_Platform.WindowWidth,
                                                                            g_Platform.WindowHeight).c));
                    GL_Call(glUniform2fv(Sprite->UThrottlePos , 1, Entity->Pos.c));
                    GL_Call(glUniform2fv(Sprite->UThrottleSize, 1, Entity->Dim.c));
                    GL_Call(glUniform1f(Sprite->UThrottle, g_Platform.AppInput[0].DroneControls.NormalizedThrottle));
                    GL_Call(glUniform1f(Sprite->UDeltaTime, AppState->DeltaTime));
                    
                    GL_Call(glEnable(GL_BLEND));
                    GL_Call(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                    
                    GL_Call(glBindVertexArray(Sprite->VertexAttribID));
                    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
                    GL_Call(glBindVertexArray(0));
                }
                
                if(g_SerialPortDevice.Connected)
                {
                    telem_packet Packet = TelemetryDequeuePacket(&g_SerialPortDevice,
                                                                 Telem_QueueRecieve);
                    
                    str8 TestString = str8Init(Packet.Payload, Packet.Header.PayloadSize);
                    
                    RenderText(&AppState->GlyphHash,
                               &OpenGLRenderer.Models[1],
                               TestString,
                               200, 299, 1, v3f32Init(1.0f, 1.0f, 0.0f));
                }
                
                SwapBuffers(gl_device_context);
#endif
                
                //~ DELAY
                QueryPerformanceCounter(&end_frame_time_data);
                
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
            
            win32_SerialPort_CloseDevice(&g_SerialPortDevice, &g_Win32State);
            
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
