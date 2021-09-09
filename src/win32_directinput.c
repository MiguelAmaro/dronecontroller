

// NOTE(MIGUEL): may need to support more version
#define DIRECTINPUT_VERSION 0x0800
#include "dc_platform.h"
#include "LAL.h"
#include <windows.h>
#include <dinput.h>
#include <dinputd.h>

//TODO(MIGUEL): rename this module to controller
global u32 num_flight_devices = 0;

typedef enum
{
    stick    = 0xb10a044f,
    throttle = 0xb687044f
} controller_id;

global LPDIRECTINPUT8       g_DirectInput_interface   ;

global LPDIRECTINPUTDEVICE8 g_throttle             ;
global DIDEVCAPS            g_throttle_capabilities;
global b32                  g_is_connected_throttle;


global LPDIRECTINPUTDEVICE8 g_flightstick             ;
global DIDEVCAPS            g_flightstick_capabilities;
global b32                  g_is_connected_flightstick;



HRESULT       win32_DirectInput_device_poll              (LPDIRECTINPUTDEVICE8 device, platform *Platform);

b32           win32_DirectInput_device_init              (LPDIRECTINPUTDEVICE8 device, DIDEVCAPS capabilities, HWND window);
b32           win32_DirectInput_direct_input_log_error   (HRESULT result, const char* expectation);
BOOL CALLBACK win32_DirectInput_direct_input_enum_devices(LPCDIDEVICEINSTANCE device, LPVOID prevInstance);

internal b32 win32_DirectInput_init(HWND window, HINSTANCE instance)
{
    //******************************
    // CREATING A DEVICE INTERFACE
    //******************************
    HRESULT result;
    
    result = DirectInput8Create(instance,
                                DIRECTINPUT_VERSION,
                                &IID_IDirectInput8,
                                (VOID**)&g_DirectInput_interface,
                                NULL);
    
    
    ASSERT(win32_DirectInput_direct_input_log_error(result, "Create Device Interface"));
    
    
    //**************************************
    // QUERYING DEVICES ON SYSTEM FOR STICK & THROTTLE
    //**************************************
    
    result = g_DirectInput_interface->lpVtbl->EnumDevices(
                                                          g_DirectInput_interface,
                                                          DI8DEVCLASS_GAMECTRL,
                                                          win32_DirectInput_direct_input_enum_devices,
                                                          NULL,
                                                          DIEDFL_ATTACHEDONLY);
    
    ASSERT(win32_DirectInput_direct_input_log_error(result, "Enumerate Devices And Fetch Desired One"));
    
    g_is_connected_flightstick = win32_DirectInput_device_init(g_flightstick,
                                                               g_flightstick_capabilities, window);
    g_is_connected_throttle    = win32_DirectInput_device_init(g_throttle   ,
                                                               g_throttle_capabilities, window);
    return 1;
}

// NOTE(MIGUEL): should be in the main win32 file
void win32_print_last_sys_error(void)
{
    LPTSTR error_msg;
    u32 error_code    = GetLastError();
    u32 error_msg_len = 0;
    
    error_msg_len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                   FORMAT_MESSAGE_FROM_SYSTEM     ,
                                   NULL                           ,
                                   error_code                     ,
                                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                   (u8 *)&error_msg, 0, NULLPTR);
    
    OutputDebugStringA(error_msg);
    
    LocalFree(error_msg);
    
    return;
}


b32 win32_DirectInput_device_init( LPDIRECTINPUTDEVICE8 device, DIDEVCAPS capabilities,HWND window)
{
    HRESULT result;
    
    //**************************************
    // TELL THE OS HOW TO FORMAT DEVICES INPUT DATA
    //**************************************
    
    result = device->lpVtbl->SetDataFormat(device,
                                           &c_dfDIJoystick2);
    
    if(!win32_DirectInput_direct_input_log_error(result, "Set Device Data Format"))
    { return 0; }
    
    
    //**************************************
    // INPUT DEVICE SETUP
    //**************************************
    
    result = device->lpVtbl->SetCooperativeLevel(device,
                                                 window,
                                                 DISCL_EXCLUSIVE | DISCL_FOREGROUND);
    
    if(!win32_DirectInput_direct_input_log_error(result, "Set Device Cooperative Levels"))
    { return 0; }
    
    
    //**************************************
    // FIND OUT THE CAPABILETIES OF THE STICK
    //**************************************
    
    // Try to print capabilities later
    capabilities.dwSize = sizeof(DIDEVCAPS);
    
    result = device->lpVtbl->GetCapabilities(device,
                                             &capabilities);
    
    if(!win32_DirectInput_direct_input_log_error(result, "Get Device Capabilities"))
    { return 0; }
    
    
    //**************************************
    // INPUT DEVICE SETUP
    //
    // CONFIGURE AXES AND BUTTONS
    //**************************************
    //https://www.cs.cmu.edu/~jparise/directx/joystick/
    // Use Defaults configure later ^ (Enumerating Axes)
    //Joystick->lpVtbl->EnumObjects(Joystick, enumAxesCallback, NULL, DIDFT_AXIS)
    
    return 1;
}

HRESULT win32_DirectInputProcessFlightStickInput(LPDIRECTINPUTDEVICE8 device, platform *Platform)
{
    HRESULT result;
    DIJOYSTATE2 joystick_state;
    
    if(device == NULL)
    {
        return S_OK;
    }
    
    result = device->lpVtbl->Poll(device);
    
    if(FAILED(result))
    {
        result = device->lpVtbl->Acquire(device);
        
        while (result == DIERR_INPUTLOST) 
        {
            result = device->lpVtbl->Acquire(device);
        }
        
        
        if ((result == DIERR_INVALIDPARAM) || (result == DIERR_NOTINITIALIZED)) 
        {
            return E_FAIL;
        }
        
        if (result == DIERR_OTHERAPPHASPRIO) 
        {
            return S_OK;
        }
    }
    if (FAILED(result = device->lpVtbl->GetDeviceState(device, sizeof(DIJOYSTATE2), &joystick_state))) 
    {
        return result; // The device should have been acquired during the Poll()
    }
    
    Platform->AppInput->DroneControls.StickPos.X = joystick_state.lX;
    Platform->AppInput->DroneControls.StickPos.Y = joystick_state.lY;
    
    return S_OK;
}


HRESULT win32_DirectInputProcessThrottleInput(LPDIRECTINPUTDEVICE8 device, platform *Platform)
{
    HRESULT result;
    DIJOYSTATE2 joystick_state;
    
    if(device == NULL)
    {
        return S_OK;
    }
    
    result = device->lpVtbl->Poll(device);
    
    if(FAILED(result))
    {
        result = device->lpVtbl->Acquire(device);
        
        while (result == DIERR_INPUTLOST) 
        {
            result = device->lpVtbl->Acquire(device);
        }
        
        if ((result == DIERR_INVALIDPARAM) || (result == DIERR_NOTINITIALIZED)) 
        {
            return E_FAIL;
        }
        
        if (result == DIERR_OTHERAPPHASPRIO) 
        {
            return S_OK;
        }
    }
    if (FAILED(result = device->lpVtbl->GetDeviceState(device, sizeof(DIJOYSTATE2), &joystick_state))) 
    {
        return result; // The device should have been acquired during the Poll()
    }
    
    Platform->AppInput->DroneControls.NormalizedThrottle = (1.0f - (f32)joystick_state.lZ / (f32)65535.0f);
    
    return S_OK;
}


BOOL CALLBACK win32_DirectInput_direct_input_enum_devices(LPCDIDEVICEINSTANCE device, LPVOID prevInstance)
{
    HRESULT result;
    
    if(device->dwDevType & ~DI8DEVTYPE_FLIGHT)
    {
        //controller_id controller;
        
        if(device->guidProduct.Data1 == stick)
        {
            
            result = g_DirectInput_interface->lpVtbl->CreateDevice(g_DirectInput_interface,
                                                                   &device->guidInstance,
                                                                   &g_flightstick,
                                                                   NULL);
        }
        if(device->guidProduct.Data1 == throttle)
        {
            result = g_DirectInput_interface->lpVtbl->CreateDevice(g_DirectInput_interface,
                                                                   &device->guidInstance,
                                                                   &g_throttle,
                                                                   NULL);
        }
        
        win32_DirectInput_direct_input_log_error(result, "Create a Device");
        
        num_flight_devices++;
    }
    
    if(num_flight_devices == 2)
    {
        return DIENUM_STOP;
    }
    
    return DIENUM_CONTINUE;
}

void win32_DirectInput_log_device_info(LPCDIDEVICEINSTANCE device)
{
    // NOTE(MIGUEL): Use this in the callback for device enumeration function
    u8  device_log[256];
    u32 device_log_len ;
    u32 bytes_written ;
    HANDLE debug_file ;
    LARGE_INTEGER debug_file_ptr;
    
    debug_file = CreateFileA("F:\\Dev\\FlightControl\\res\\device_log.txt",
                             GENERIC_WRITE,
                             FILE_SHARE_READ, NULL,
                             CREATE_NEW,
                             0, NULL );
    
    if(GetLastError() == ERROR_FILE_EXISTS)
    {
        
        debug_file = CreateFileA("F:\\Dev\\FlightControl\\res\\device_log.txt",
                                 GENERIC_WRITE,
                                 FILE_SHARE_READ, NULL,
                                 OPEN_EXISTING,
                                 0, NULL );
        
        SetFilePointerEx(debug_file,
                         (LARGE_INTEGER){0, 0},
                         &debug_file_ptr,
                         FILE_END);
        
    }
    
    device_log_len = wsprintf(device_log, 
                              "Prod Name: %s \n"
                              "Id: %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\n\n",
                              device->tszInstanceName,
                              
                              device->guidProduct.Data1,
                              
                              device->guidProduct.Data2,
                              
                              device->guidProduct.Data3,
                              
                              device->guidProduct.Data4[0],
                              device->guidProduct.Data4[1],
                              
                              device->guidProduct.Data4[2],
                              device->guidProduct.Data4[3],
                              device->guidProduct.Data4[4],
                              device->guidProduct.Data4[5],
                              device->guidProduct.Data4[6],
                              device->guidProduct.Data4[7]);
    
    WriteFile(debug_file,
              device_log, device_log_len,
              &bytes_written, NULLPTR);
    
    CloseHandle(debug_file);
    
    return;
}

b32 win32_DirectInput_direct_input_log_error(HRESULT result, const char* expectation)
{
    printf("%s - ", expectation);
    switch(result)
    {
        case DI_OK:
        {
            printf("Successful ! \n");
            return TRUE;
        }
        break;
        case DIERR_BETADIRECTINPUTVERSION:
        {
            printf("Error: %s", "Beta Direct Input version \n");
            return FALSE;
        }
        break;
        case DIERR_INVALIDPARAM:
        {
            printf("Error: %s", "Invalid Parameter \n  ");
            return FALSE;
        }
        break;
        case DIERR_OLDDIRECTINPUTVERSION:
        {
            printf("Error: %s", "Old Direct Input Version \n");
            return FALSE;
        }
        break;
        case DIERR_NOTINITIALIZED:
        {
            printf("Error: %s", "Not initialized\n");
            return FALSE;
        }
        break;
        case DIERR_OUTOFMEMORY:
        {
            printf("Error: %s", "Out of Memory \n");
            return FALSE;
        }
        break;
        case DIERR_DEVICENOTREG:
        {
            printf("Error: %s", "Error: Device Not Registered \n");
            return FALSE;
        }
        break;
        case E_HANDLE:
        {
            printf("Error: %s", "Error with the handle \n");
            return FALSE;
        }
        break;
        default:
        {
            printf("Error: %s", "Error Log Failed \n");
            return FALSE;
        }
    }
    
    return FALSE;
}
