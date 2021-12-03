#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include "win32_dc.h"
#include "win32_directinput.c"

#include "dc_memory.h"
#include "dc_config.h"
#include "dc_types.h"
#include "dc.h"
#include "dc_math.h"
#include "dc_renderer.h"
#include "dc_render_commands.h"
#include "dc_platform.h"
#include "dc_serialport.h"

global platform         g_Platform   = {0};
global win32_state      g_Win32State = {0};
global win32_backbuffer g_BackBuffer = {0};

// NOTE(MIGUEL): Not sure if this is needed
#if 0
PLATFORM_SET_CAPTURE(PlatformSetCapture)
{
    if(Capture) SetCapture(g_Win32State.Window);
    else        ReleaseCapture();
    
    return;
}
#endif

typedef struct win32_app_code win32_app_code;
struct win32_app_code
{
    HMODULE     APP_DLL;
    app_update *Update;
    b32         IsValid;
    FILETIME    DLLLastWriteTime;
};

static FILETIME
win32_GetLastWriteTime(char *FileName)
{
    FILETIME LastWriteTime = { 0 };
    
    WIN32_FILE_ATTRIBUTE_DATA FileInfo;
    
    if(GetFileAttributesEx((const char *)FileName,
                           GetFileExInfoStandard,
                           &FileInfo))
    {
        LastWriteTime = FileInfo.ftLastWriteTime;
    }
    
    return LastWriteTime;
}


static void
win32_HotUnloadAppCode(win32_app_code *App)
{
    if(App)
    {
        FreeLibrary(App->APP_DLL);
    }
    
    App->IsValid = false;
    App->Update  = 0;
    
    return;
}


static win32_app_code
win32_HotLoadAppCode(char *SourceDLLName, char *TempDLLName, char *LockedFileName)
{
    win32_app_code Result = { 0 };
    
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesEx((const char *)LockedFileName,
                            GetFileExInfoStandard,
                            &Ignored))
    {
        Result.DLLLastWriteTime = win32_GetLastWriteTime(SourceDLLName);
        
        DeleteFile(TempDLLName);
        CopyFile((const char *)SourceDLLName,
                 (const char *)TempDLLName, FALSE);
        // TODO(MIGUEL): Hotswap this like the shaders
        Sleep(300);
        Result.APP_DLL = LoadLibraryA((const char *)TempDLLName);
        
        if(Result.APP_DLL)
        {
            Result.Update = (app_update *)GetProcAddress(Result.APP_DLL, "Update");
            
            Result.IsValid = (Result.Update != NULL);
        }
    }
    if(!(Result.IsValid))
    {
        Result.Update = 0;
    }
    
    return Result;
}


static void
win32_GetExeFileName(win32_state *State)
{
    u32 FileNameSize = GetModuleFileNameA(0,
                                          (LPSTR)State->ExeFileName,
                                          sizeof(State->ExeFileName));
    
    State->OnePastLastExeFileNameSlash = State->ExeFileName;
    
    for(char *Scan = (char *)State->ExeFileName; *Scan; ++Scan)
    {
        if(*Scan == '\\')
        {
            State->OnePastLastExeFileNameSlash = Scan + 1;
        }
    }
    
    return;
}

static void
win32_BuildExePathFileName(win32_state *State,
                           char *FileName,
                           int DestCount, char *Dest)
{
    str8Concat(State->OnePastLastExeFileNameSlash - State->ExeFileName,
               State->ExeFileName,
               str8GetCStrLength(FileName), FileName,
               DestCount, Dest);
    
    return;
}


LRESULT CALLBACK 
win32_Main_Window_Procedure(HWND Window, UINT Message , WPARAM w_param, LPARAM l_param) {
    LRESULT Result = 0;
    
    u32 key_code  = 0;
    u32 key_index = 0;
    b32 key_down  = 0; 
    
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
            g_Platform.QuitApp = 1;
        } break;
        
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        
        case WM_DESTROY:
        {
            g_Platform.QuitApp = 1;
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
win32_ProcessKeyboardMessage(input_button_state *NewState, b32 IsDown)
{
    if(NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
    
    return;
}

internaldef void
win32_ProcessPendingMessages(win32_state *State, input_src *Keyboard)
{
    MSG Message;
    
    u32 KeyCode       = 0;
    u32 KeyIndex      = 0;
    b32 KeyIsDown     = 0; 
    b32 KeyWasDown    = 0; 
    b32 KeyAltWasDown = 0;
    
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                g_Platform.QuitApp = 1;
            }  break;
            
            case WM_MOUSEMOVE :
            {
                g_Platform.Controls->MousePos.x = ((Message.lParam & 0x0000FFFF));
                g_Platform.Controls->MousePos.y = ((Message.lParam & 0xFFFF0000) >> 16);
            } break;
            
            case WM_LBUTTONDOWN: g_Platform.Controls->MouseLeftButtonDown = 1; break;
            case WM_LBUTTONUP  : g_Platform.Controls->MouseLeftButtonDown = 0; break;
            case WM_RBUTTONDOWN: break;
            case WM_RBUTTONUP  : break;
            case WM_MOUSEHWHEEL: break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP  :
            case WM_KEYDOWN   :
            case WM_KEYUP     :
            {
                KeyIndex  = 0;
                
                KeyCode       = (u32)Message.wParam;
                KeyIsDown     = ((Message.lParam & (1 << 31)) == 0);
                KeyWasDown    = ((Message.lParam & (1 << 30)) != 0);
                KeyAltWasDown = ( Message.lParam & (1 << 29));
                
                if(KeyWasDown != KeyIsDown)
                {
                    
                    if(KeyCode >= 'A' && KeyCode <= 'Z')
                    { 
                        KeyIndex = Key_a + (KeyCode - 'A');
                        win32_ProcessKeyboardMessage(&Keyboard->AlphaKeys[KeyIndex], KeyIsDown);
                    }
                    
                    switch(KeyCode)
                    {
                        case VK_UP    : win32_ProcessKeyboardMessage(&Keyboard->NavKeys[0], KeyIsDown); break;
                        case VK_LEFT  : win32_ProcessKeyboardMessage(&Keyboard->NavKeys[1], KeyIsDown); break;
                        case VK_DOWN  : win32_ProcessKeyboardMessage(&Keyboard->NavKeys[2], KeyIsDown); break;
                        case VK_RIGHT : win32_ProcessKeyboardMessage(&Keyboard->NavKeys[3], KeyIsDown); break;
                        case VK_ESCAPE: win32_ProcessKeyboardMessage(&Keyboard->NavKeys[4], KeyIsDown); break;
                        case VK_SPACE : win32_ProcessKeyboardMessage(&Keyboard->NavKeys[5], KeyIsDown); break;
                        case VK_F4    : g_Platform.QuitApp = KeyAltWasDown ? 1 : 0; break;
                    }
                }
                
                if((KeyCode == VK_RETURN) && (KeyAltWasDown) && Message.hwnd)
                {
                    //win32_toggle_fullscreen(Message.hwnd );
                }
                
                if((KeyCode == 'P') && (KeyIsDown))
                {
                    //g_Pause= !g_Pause;
                }
                
            } break;
            
            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
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
        
        //App_Init(&g_Platform);
        
        win32_GetExeFileName(&g_Win32State);
        
        char AppCodeDLLFullPathSource[MAX_PATH];
        win32_BuildExePathFileName(&g_Win32State, "dc.dll",
                                   sizeof(AppCodeDLLFullPathSource), AppCodeDLLFullPathSource);
        
        char AppCodeDLLFullPathTemp  [MAX_PATH];
        win32_BuildExePathFileName(&g_Win32State, "dc_temp.dll",
                                   sizeof(AppCodeDLLFullPathTemp), AppCodeDLLFullPathTemp);
        
        char AppCodeDLLFullPathLock  [MAX_PATH];
        win32_BuildExePathFileName(&g_Win32State, "lock.tmp",
                                   sizeof(AppCodeDLLFullPathLock), AppCodeDLLFullPathLock);
        
        win32_app_code AppCode;
        
        if(Window)
        {
            render_constraints Constraints =
            { 
                .QuadMaxCountPerFrame = 256,
            };
            
            
            opengl_renderer *Renderer = RendererInit(Window,
                                                     g_Platform.WindowWidth,
                                                     g_Platform.WindowHeight,
                                                     &Constraints);
            
            app_state *AppState = (app_state *)g_Platform.PermanentStorage;
            
            win32_SerialPort_InitDevice(&g_Win32State, &g_SerialPortDevice);
            // TODO(MIGUEL): Set initialization to individual controller structures
            g_Platform.StickIsInitialized        = win32_DirectInput_init(Window, Instance);
            
            
            s64 Frequency   = 1;
            LARGE_INTEGER FreqTemp              = { 0LL };
            QueryPerformanceFrequency(&FreqTemp);
            Frequency = FreqTemp.QuadPart;
            
            LARGE_INTEGER StartTick = { 0LL };
            LARGE_INTEGER EndTick   = { 0LL };
            
            /// MAIN LOOP
            while(!g_Platform.QuitApp)
            {
                
                g_Platform.LastTime     = g_Platform.CurrentTime;
                g_Platform.CurrentTime += 1 / g_Platform.TargetSecondsPerFrame;
                s64 DesiredTicksPerFrame = Frequency / g_Platform.TargetSecondsPerFrame;
                QueryPerformanceCounter(&StartTick);
                
                
                win32_ProcessPendingMessages(0, &g_Platform.Controls[0]);
                
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
                    win32_SerialPort_RecieveData(&g_SerialPortDevice);
                }
                else
                {
                    win32_SerialPort_InitDevice(&g_Win32State, &g_SerialPortDevice);
                }
                
                //~ PROCESSING
                
                RendererBeginFrame(Renderer, g_Platform.WindowWidth, g_Platform.WindowHeight);
                
                app_backbuffer AppRenderBuffer = { 0 };
                AppRenderBuffer.Data          = g_BackBuffer.Data;
                AppRenderBuffer.Width         = g_BackBuffer.Width;
                AppRenderBuffer.Height        = g_BackBuffer.Height;
                AppRenderBuffer.BytesPerPixel = g_BackBuffer.BytesPerPixel;
                
                
                FILETIME NewDLLWriteTime = win32_GetLastWriteTime(AppCodeDLLFullPathSource);
                {
                    if(CompareFileTime(&NewDLLWriteTime, &AppCode.DLLLastWriteTime))
                    {
                        win32_HotUnloadAppCode(&AppCode);
                        AppCode = win32_HotLoadAppCode(AppCodeDLLFullPathSource,
                                                       AppCodeDLLFullPathTemp,
                                                       AppCodeDLLFullPathLock);
                    }
                }
                
                if(AppCode.Update)
                {
                    AppCode.Update(&g_Platform, &g_SerialPortDevice.PacketQueues, &Renderer->RenderData);
                }
                
                
                RendererEndFrame(Renderer);
                
                //~ OUTPUT
                //- SERIALPORT
                
                if(g_SerialPortDevice.Connected)
                {
                    f32 ThrottleValue = g_Platform.Controls[0].NormThrottlePos;
                    
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
                
                //~ DELAY
                QueryPerformanceCounter(&EndTick);
                
                {
                    s64 TicksPerFrame  = EndTick.QuadPart     - StartTick.QuadPart;
                    s64 TicksRemaining = DesiredTicksPerFrame - TicksPerFrame;
                    
                    LARGE_INTEGER StartWaitTick;
                    LARGE_INTEGER EndWaitTick;
                    
                    QueryPerformanceCounter(&StartWaitTick);
                    
                    while(TicksRemaining > 0)
                    {
                        QueryPerformanceCounter(&EndWaitTick);
                        TicksRemaining -= EndWaitTick.QuadPart - StartWaitTick.QuadPart;
                        StartWaitTick   = EndWaitTick;
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
