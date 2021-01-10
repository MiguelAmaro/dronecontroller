#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <GLAD/glad.h>
#include "FlightControl_Program_Options.h"
#include "FlightControl_FlightStick.c"
#include "FlightControl_SerialPort.h"
#include "FlightControl_Platform.h"
#include "FlightControl_OpenGL.h"
#include "FlightControl.c"
#include <stdio.h>
#include <string.h>
#include "LAL.h"
#include "RoundBuffer.c"
#include <GL/gl.h>
#include <WGL/wglext.h>
#include <io.h>
#include <fcntl.h>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <strsafe.h>
//#include <shellapi.h>
//#include <internal.h>
#include <conio.h>

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

HANDLE g_hInputFile = NULL;

// TODO(MIGUEL): Add openCV

global Platform global_platform = {0};

// TODO(MIGUEL): Using Windows File API connect to a kinetis board and recive and send input

internal void 
win32_resize_DIB_Section(int Width, int Height);

internal void 
win32_update_Window(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height);

LRESULT CALLBACK 
win32_Main_Window_Procedure(HWND Window, UINT Message , WPARAM w_param, LPARAM l_param);

void ErrorExit(PTSTR lpszFunction) 

// Format a readable error message, display a message box, 
// and exit from the application.
{ 
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 
    
    FormatMessage(
                  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  dw,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &lpMsgBuf,
                  0, NULL );
    
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
                                      (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
                    LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                    TEXT("%s failed with error %d: %s"), 
                    lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 
    
    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(1);
}

void WriteToPipe(void) 

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data. 
{ 
    DWORD dwRead, dwWritten; 
    CHAR chBuf[DEBUG_CONSOLE_BUFFER_SIZE];
    BOOL bSuccess = FALSE;
    
    for (;;) 
    { 
        bSuccess = ReadFile(g_hInputFile, chBuf, DEBUG_CONSOLE_BUFFER_SIZE, &dwRead, NULL);
        if ( ! bSuccess || dwRead == 0 ) break; 
        
        bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
        if ( ! bSuccess ) break; 
    } 
    
    // Close the pipe handle so the child process stops reading. 
    
    if ( ! CloseHandle(g_hChildStd_IN_Wr) ) 
        ErrorExit(TEXT("StdInWr CloseHandle")); 
} 

void ReadFromPipe(void) 

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
{ 
    DWORD dwRead, dwWritten; 
    CHAR chBuf[DEBUG_CONSOLE_BUFFER_SIZE]; 
    BOOL bSuccess = FALSE;
    HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    
    for (;;) 
    { 
        bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, DEBUG_CONSOLE_BUFFER_SIZE, &dwRead, NULL);
        if( ! bSuccess || dwRead == 0 ) break; 
        
        bSuccess = WriteFile(hParentStdOut, chBuf, 
                             dwRead, &dwWritten, NULL);
        if (! bSuccess ) break; 
    } 
} 

internal void
win32_create_Debug_Console(void)
{
    b32 debug_console_success = false;
    
    PROCESS_INFORMATION debug_console_process_info = { 0 };
    STARTUPINFO         debug_console_startup_info = 
    {
        .cb         = sizeof(STARTUPINFO),
        .dwXSize    = 200,
        .dwYSize    = 800,
        .hStdError  = g_hChildStd_OUT_Wr,
        .hStdOutput = g_hChildStd_OUT_Wr,
        .hStdInput  = g_hChildStd_IN_Rd,
        .dwFlags    = STARTF_USESIZE | STARTF_USESTDHANDLES
    };
    
    CreateProcessA(NULLPTR,
                   "child",
                   NULLPTR,
                   NULLPTR,
                   true,
                   0,
                   NULLPTR,
                   NULLPTR,
                   &debug_console_startup_info,
                   &debug_console_process_info
                   );
    
    if(!debug_console_success)
    {
        ErrorExit(TEXT("CreateProcess"));
    }
    else
    {
        
        CloseHandle( debug_console_process_info.hProcess);
        CloseHandle( debug_console_process_info.hThread );
        
        CloseHandle(g_hChildStd_OUT_Wr);
        CloseHandle(g_hChildStd_IN_Rd);
    }
    
    return;
}

internal void 
win32_create_Debug_Pipes(void)
{
    SECURITY_ATTRIBUTES saAttr; 
    
    printf("\n->Start of parent execution.\n");
    
    // Set the bInheritHandle flag so pipe handles are inherited. 
    
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
    saAttr.bInheritHandle = TRUE; 
    saAttr.lpSecurityDescriptor = NULL; 
    
    // Create a pipe for the child process's STDOUT. 
    
    if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) 
        ErrorExit(TEXT("StdoutRd CreatePipe")); 
    
    // Ensure the read handle to the pipe for STDOUT is not inherited.
    
    if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
        ErrorExit(TEXT("Stdout SetHandleInformation")); 
    
    // Create a pipe for the child process's STDIN. 
    
    if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) 
        ErrorExit(TEXT("Stdin CreatePipe")); 
    
    // Ensure the write handle to the pipe for STDIN is not inherited. 
    
    if ( ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
        ErrorExit(TEXT("Stdin SetHandleInformation")); 
    
    // Create the child process. 
    
    win32_create_Debug_Console();
    
    return;
}

int CALLBACK 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) 
{
    //win32_create_Debug_Pipes();
    //win32_create_Debug_Console();
    
    //LPWSTR argv = GetCommandLineW();
    //int argc;
    
    //argv = CommandLineToArgvA(GetCommandLine(), &argc);
    //__getmainargs(&argc, &argv, NULLPTR ,0, NULLPTR );
    //if (argc == 1) 
    //{
    //ErrorExit(TEXT("Please specify an input file.\n")); 
    //}
    
    //HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    
    HANDLE Debug_console = GetStdHandle(STD_OUTPUT_HANDLE );
    /*
    g_hInputFile = CreateFileW(&argv[1], 
                               GENERIC_READ, 
                               0, 
                               NULL, 
                               OPEN_EXISTING, 
                               FILE_ATTRIBUTE_READONLY, 
                               NULL); 
    
    if ( g_hInputFile == INVALID_HANDLE_VALUE ) 
    {
        ErrorExit(TEXT("CreateFile")); 
    }
    // Write to the pipe that is the standard input for a child process. 
    // Data is written to the pipe's buffers, so it is not necessary to wait
    // until the child process is running before writing data.
    
    WriteToPipe(); 
    printf( "\n->Contents of %s written to child STDIN pipe.\n", "Win32_FlightControl.exe");
    
    */
    //HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    //**************************************
    // MAIN WINDOW SETUP
    //
    // DEFINING WINDOW CLASS TO REGISTER
    //**************************************
    
    WNDCLASS WindowClass      = {0};
    WindowClass.style         = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
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
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     0, 0, Instance, 0);
        
        //AllocConsole();
        //AttachConsole(GetProcessId(Window));
        //HANDLE Debug_console = GetStdHandle(STD_INPUT_HANDLE);
        
        // TODO: figure out why app crashes if stick is not connected
        //This is for the joystick!!!
        CreateDevice(Window, Instance);
        
        HDC gl_device_context = GetDC(Window);
        
        HGLRC gl_real_context = win32_Init_OpenGL(gl_device_context);
        
        ASSERT(gladLoadGL());
        
        u32 gl_major = 0;
        u32 gl_minor = 0;
        
        glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
        glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
        printf("OPENGL VERSION: %d.%d \n", gl_major, gl_minor);
        
        // NOTE(MIGUEL): This should Init on users command
        win32_serial_Port_Init();
        
        // PLATFORM INITIALIZATION
        {
            global_platform.permanent_storage_size = PERMANENT_STORAGE_SIZE;
            global_platform.permanent_storage = VirtualAlloc(0, 
                                                             global_platform.permanent_storage_size,
                                                             MEM_COMMIT | MEM_RESERVE,
                                                             PAGE_READWRITE);
            
            global_platform.frames_per_second_target = 60.0f;
        }
        
        App_Init(&global_platform);
        
        
        if(Window)
        {
            
            //int XOffset = 0;
            //int YOffset = 0;
            
            MSG Message;
            
            s64 performance_counter_frequency = 1;
            {
                LARGE_INTEGER freq = { 0LL };
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
                global_platform.last_time = global_platform.current_time;
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
                
                //~ JOYSTICK & SERIALPORT 
                
                Joystick_Poll(&global_platform);
                
                u8 TempChar;
                u8 SerialBuffer[256];
                DWORD NoBytesRead;
                int i = 0;
                do
                {
                    ReadFile( global_Device.comm,           //Handle of the Serial port
                             &TempChar,       //Temporary character
                             sizeof(TempChar),//Size of TempChar
                             &NoBytesRead,    //Number of bytes read
                             NULL);
                    
                    SerialBuffer[i] = TempChar;
                    i++;
                }
                while (NoBytesRead > 0 && i < 256);
                printf("HOST: Recieving - %s \n", SerialBuffer);
                
                //~ WIN32 RENDERING
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
                
                // Do OpenGL rendering here
                
                global_platform.quit |= App_Update(&global_platform);
                
                SwapBuffers(gl_device_context);
                
                
                QueryPerformanceCounter(&end_frame_time_data);
                
                //printf("Frame data: %lld <- ( %lld - %lld ) \n", (end_frame_time_data.QuadPart - begin_frame_time_data.QuadPart), end_frame_time_data.QuadPart, begin_frame_time_data.QuadPart);
                
                // NOTE(MIGUEL): Wait any time, if neccssary
                // TODO(MIGUEL): think about changing target fps if current target is not met
                {
                    s64 counts_in_frame = end_frame_time_data.QuadPart - begin_frame_time_data.QuadPart;
                    s64 counts_to_wait = desired_frame_time_counts - counts_in_frame;
                    
                    LARGE_INTEGER begin_wait_time_data;
                    LARGE_INTEGER end_wait_time_data;
                    
                    QueryPerformanceCounter(&begin_wait_time_data);
                    
                    while(counts_to_wait > 0)
                    {
                        QueryPerformanceCounter(&end_wait_time_data);
                        counts_to_wait -= end_wait_time_data.QuadPart - begin_wait_time_data.QuadPart;
                        begin_wait_time_data = end_wait_time_data;
                        
                        //printf("Wait data: %lld <- ( %lld - %lld ) \n", (end_wait_time_data.QuadPart - begin_wait_time_data.QuadPart), end_wait_time_data.QuadPart, begin_wait_time_data.QuadPart);
                    }
                }
                /*
                DWORD numchars;
                COORD coord = { 0, 0 };
                u8 *Clear_string = "blahhhhhh"; 
                WriteConsoleOutputCharacter(Debug_console, Clear_string,4000,coord,&numchars);
                SetConsoleCursorPosition(Debug_console,coord);
                */
                
            }
            CloseHandle(global_Device.comm);//Closing the Serial Port
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
    
    u32 key_code = 0;
    u32 key_index = 0;
    b32 key_down = false; 
    
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
    
    BitmapWidth = Width;
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

