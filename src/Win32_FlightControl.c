#include <windows.h>
#include "FlightControl_FlightStick.c"
#include "FlightControl_SerialPort.h"
#include "FlightControl_Platform.h"
#include "FlightControl_OpenGL.h"
#include "FlightControl.c"
#include <stdio.h>
#include <string.h>
#include "LAL.h"
#include "RoundBuffer.c"
//#include <GL/gl.h>
//#include <WGL/wglext.h>

// TODO(MIGUEL): Add opengl

//global u8 Running;
global BITMAPINFO BitmapInfo;
global void *BitmapMemory;
global int BitmapWidth;
global int BitmapHeight;
global int BytesPerPixel = 4;

global Platform global_platform = {0};

// NOTE(MIGUEL): move to an app options header file maybe
#define PERMANENT_STORAGE_SIZE MEGABYTES(64) 


// TODO(MIGUEL): Using Windows File API connect to a kinetis board and recive and send input

// NOTE(MIGUEL): Platform Independent
internal void RenderWeirdGradient(int XOffset, int YOffset) {
    int Width  = BitmapWidth;
    int Height = BitmapHeight;
    
    int Pitch = Width * BytesPerPixel;
    u8 *Row = (u8*)BitmapMemory;
    for(int Y = 0; Y < BitmapHeight; Y++) {
        u8 *Pixel = (u8 *)Row;
        for(int X = 0; X < BitmapWidth; X++) {
            // Blue
            *Pixel = (u8)(X + XOffset);
            ++Pixel;
            // Green
            *Pixel = (u8)(Y + YOffset);
            ++Pixel;
            // Red
            *Pixel = 0;
            ++Pixel;
            // Padding
            *Pixel = 0;
            ++Pixel;
        }
        Row += Pitch;
    }
    
    return;
}



// NOTE(MIGUEL): DEPRECATED - moving to opengl
internal void 
Win32ResizeDIBSection(int Width, int Height) {
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
Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height) {
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


LRESULT CALLBACK 
Win32MainWindowCallback(HWND Window, UINT Message , WPARAM w_param, LPARAM l_param) {
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect); //Get RECT of window excludes borders
            int Width  = ClientRect.right  - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top ;
            Win32ResizeDIBSection(Width, Height);
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
            
            Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
            EndPaint(Window, &Paint);
        }
        break;
        // TODO(MIGUEL): Add code for WM_KEYUP
        case WM_KEYDOWN:
        {
            u32 key_code = w_param;
            u32 key_index = 0;
            
            if(key_code == 'A')
            { 
                key_index = KEY_a;
            }
            global_platform.key_down[key_index] = 1;
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

int CALLBACK 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) {
    //**************************************
    // MAIN WINDOW SETUP
    //
    // DEFINING WINDOW CLASS TO REGISTER
    //**************************************
    
    WNDCLASS WindowClass      = {0};
    WindowClass.style         = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc   = Win32MainWindowCallback;
    WindowClass.hInstance     = Instance;
    //WindowClass.hIcon;
    WindowClass.lpszClassName = "MyWindowClass";
    
    
    
    //**************************************
    // MAIN WINDOW SETUP
    //
    // REGISTER WINDOW CLASS WITH OS
    //**************************************
    
    if(RegisterClass(&WindowClass)) {
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
        
        // TODO: figure out why app crashes if stick is not connected
        //This is for the joystick!!!
        CreateDevice(Window, Instance);
        
        HDC gl_device_context = GetDC(Window);
        
        HGLRC gl_real_context = win32_Init_OpenGL(gl_device_context);
        
        
        printf("OPENGL VERSION: %s \n", glGetString(GL_VERSION));
        
        
        // NOTE(MIGUEL): This should Init on users command
        Init_SerialPort();
        
        // Platform Initialization
        {
            global_platform.permanent_storage_size = PERMANENT_STORAGE_SIZE;
            global_platform.permanent_storage = VirtualAlloc(0, 
                                                             global_platform.permanent_storage_size,
                                                             MEM_COMMIT | MEM_RESERVE,
                                                             PAGE_READWRITE);
        }
        
        //**************************************
        // MAIN PROGRAM LOOP
        //
        // MESSAGE PROCESSING AND RENDERING
        //**************************************
        
        if(Window){
            DIJOYSTATE2 Joystick_State = {0};
            int XOffset = 0;
            int YOffset = 0;
            
            //Running = true; // NOTE(MIGUEL): Depracated 
            
            // NOTE(MIGUEL): While depended on "Running" variable
            while(!global_platform.quit){
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        global_platform.quit = true;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage (&Message);
                }
                
                global_platform.quit |= App_Update(&global_platform);
                
                //~ JOYSTICK & SERIALPORT 
                {
                    Joystick_Poll(&Joystick_State);
                    /*
                    printf("Stick X: %d | Stick Y: %d \n\r",
                           Joystick_State.lX,
                           Joystick_State.lY);
                    */
                    
                    
                    // Send command to mcu over IO steam | controlled by joystick 
                    if( Joystick_State.lX > 32767){
                        global_Device.buffer[0] = 'A';
                        
                        global_Device.status = WriteFile(global_Device.comm,        // Handle to the Serial port
                                                         global_Device.buffer,           // Data to be written to the port
                                                         global_Device.bytes_to_write,  //No of bytes to write
                                                         &global_Device.bytes_written, //Bytes written
                                                         NULL);
                    }
                    else if( Joystick_State.lX < 32767){
                        global_Device.buffer[0] = 'B';
                        
                        global_Device.status = WriteFile(global_Device.comm,        // Handle to the Serial port
                                                         global_Device.buffer,           // Data to be written to the port
                                                         global_Device.bytes_to_write,  //No of bytes to write
                                                         &global_Device.bytes_written, //Bytes written
                                                         NULL);
                    }
                }
                
                //~ WIN32 RENDERING
                {
                    RenderWeirdGradient(XOffset - Joystick_State.lX * .5,
                                        YOffset - Joystick_State.lY * .5);
                    
                    // Rendering
                    HDC DeviceContext = GetDC(Window);
                    RECT ClientRect;
                    GetClientRect(Window, &ClientRect); //Get RECT of window
                    int WindowWidth  = ClientRect.right  - ClientRect.left;
                    int WindowHeight = ClientRect.bottom - ClientRect.top ;
                    Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
                    ReleaseDC(Window, DeviceContext);
                    
                    ++YOffset;
                    ++XOffset;
                }
                
                //~ OPENGL RENDERING
                glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                // Do OpenGL rendering here
                
                SwapBuffers(gl_device_context);
            }
            CloseHandle(global_Device.comm);//Closing the Serial Port
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