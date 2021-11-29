

// NOTE(MIGUEL): may need to support more version
#define DIRECTINPUT_VERSION 0x0800
#include "dc_platform.h"
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

// TODO(MIGUEL): Change existing implementation to use this structure
typedef struct controller controller;
struct controller
{
    LPDIRECTINPUTDEVICE8 Interface;
    DIDEVCAPS            Capabilities;
    b32                  Connected;
    controller_id        ID;
};

global controller g_Throttle;
global controller g_Flightstick;

global LPDIRECTINPUT8       g_DirectInputInterface;

//global LPDIRECTINPUTDEVICE8 g_throttle             ;
//global DIDEVCAPS            g_throttle_capabilities;
//global b32                  g_is_connected_throttle;


//global LPDIRECTINPUTDEVICE8 g_flightstick             ;
//global DIDEVCAPS            g_flightstick_capabilities;
//global b32                  g_is_connected_flightstick;



HRESULT       win32_DirectInput_device_poll(LPDIRECTINPUTDEVICE8 device, platform *Platform);

b32           win32_InitController         (controller *Controller, controller_id id, HWND window);
b32           win32_DirectInputLogError    (HRESULT result, const char* expectation);
BOOL CALLBACK win32_DirectInputEnumDevices (LPCDIDEVICEINSTANCE device, LPVOID prevInstance);

internaldef b32 win32_DirectInput_init(HWND window, HINSTANCE instance)
{
    //******************************
    // CREATING A DEVICE INTERFACE
    //******************************
    HRESULT Result;
    
    Result = DirectInput8Create(instance,
                                DIRECTINPUT_VERSION,
                                &IID_IDirectInput8,
                                (VOID**)&g_DirectInputInterface,
                                NULL);
    
    
    ASSERT(win32_DirectInputLogError(Result, "Create Device Interface"));
    
    
    //**************************************
    // QUERYING DEVICES ON SYSTEM FOR STICK & THROTTLE
    //**************************************
    
    Result = g_DirectInputInterface->lpVtbl->EnumDevices(g_DirectInputInterface,
                                                         DI8DEVCLASS_GAMECTRL,
                                                         win32_DirectInputEnumDevices,
                                                         NULL,
                                                         DIEDFL_ATTACHEDONLY);
    
    ASSERT(win32_DirectInputLogError(Result, "Enumerate Devices And Fetch Desired One"));
    
    win32_InitController(&g_Flightstick, stick,    window);
    win32_InitController(&g_Throttle   , throttle, window);
    return 1;
}

// NOTE(MIGUEL): should be in the main win32 file
void win32_PrintLastSystemError(void)
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


b32 win32_InitController(controller *Controller, controller_id ID, HWND window)
{
    HRESULT Result;
    
    LPDIRECTINPUTDEVICE8      Device       = Controller->Interface;
    IDirectInputDevice8AVtbl *DeviceVTable = Device->lpVtbl;
    
    
    Controller->ID = ID;
    
    //**************************************
    // TELL THE OS HOW TO FORMAT DEVICES INPUT DATA
    //**************************************
    
    Result = DeviceVTable->SetDataFormat(Device,
                                         &c_dfDIJoystick2);
    
    if(!win32_DirectInputLogError(Result, "Set Device Data Format"))
    { return 0; }
    
    
    //**************************************
    // INPUT DEVICE SETUP
    //**************************************
    
    Result = DeviceVTable->SetCooperativeLevel(Device,
                                               window,
                                               DISCL_EXCLUSIVE | DISCL_FOREGROUND);
    
    if(!win32_DirectInputLogError(Result, "Set Device Cooperative Levels"))
    { return 0; }
    
    
    //**************************************
    // FIND OUT THE CAPABILETIES OF THE STICK
    //**************************************
    
    // Try to print capabilities later
    Controller->Capabilities.dwSize = sizeof(DIDEVCAPS);
    
    Result = DeviceVTable->GetCapabilities(Device,
                                           &Controller->Capabilities);
    
    if(!win32_DirectInputLogError(Result, "Get Device Capabilities"))
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

HRESULT win32_DirectInputProcessFlightStickInput(controller *Controller, platform *Platform)
{
    HRESULT Result;
    DIJOYSTATE2 JoystickState;
    
    LPDIRECTINPUTDEVICE8 Device = Controller->Interface;
    
    if(Device == NULL)
    {
        return S_OK;
    }
    
    Result = Device->lpVtbl->Poll(Device);
    
    if(FAILED(Result))
    {
        Result = Device->lpVtbl->Acquire(Device);
        
        while (Result == DIERR_INPUTLOST) 
        {
            Result = Device->lpVtbl->Acquire(Device);
        }
        
        
        if ((Result == DIERR_INVALIDPARAM) || (Result == DIERR_NOTINITIALIZED)) 
        {
            return E_FAIL;
        }
        
        if (Result == DIERR_OTHERAPPHASPRIO) 
        {
            return S_OK;
        }
    }
    if (FAILED(Result = Device->lpVtbl->GetDeviceState(Device, sizeof(DIJOYSTATE2), &JoystickState))) 
    {
        return Result; // The Device should have been acquired during the Poll()
    }
    
    Platform->Controls->StickPos.x = JoystickState.lX;
    Platform->Controls->StickPos.y = JoystickState.lY;
    
    return S_OK;
}


HRESULT win32_DirectInputProcessThrottleInput(controller *Controller, platform *Platform)
{
    HRESULT result;
    DIJOYSTATE2 joystick_state;
    LPDIRECTINPUTDEVICE8 device = Controller->Interface;
    
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
    
    Platform->Controls[0].NormThrottlePos = (1.0f - (f32)joystick_state.lZ / (f32)65535.0f);
    
    return S_OK;
}


BOOL CALLBACK win32_DirectInputEnumDevices(LPCDIDEVICEINSTANCE device, LPVOID prevInstance)
{
    HRESULT Result;
    
    if(device->dwDevType & ~DI8DEVTYPE_FLIGHT)
    {
        //controller_id controller;
        
        if(device->guidProduct.Data1 == stick)
        {
            
            Result = g_DirectInputInterface->lpVtbl->CreateDevice(g_DirectInputInterface,
                                                                  &device->guidInstance,
                                                                  &g_Flightstick.Interface,
                                                                  NULL);
            num_flight_devices++;
        }
        if(device->guidProduct.Data1 == throttle)
        {
            Result = g_DirectInputInterface->lpVtbl->CreateDevice(g_DirectInputInterface,
                                                                  &device->guidInstance,
                                                                  &g_Throttle.Interface,
                                                                  NULL);
            num_flight_devices++;
        }
        
        win32_DirectInputLogError(Result, "Create a Device");
    }
    
    if(g_Flightstick.Interface &&
       g_Throttle.Interface)
    {
        return DIENUM_STOP;
    }
    
    return DIENUM_CONTINUE;
}

void win32_DirectInputLogDeviceInfo(LPCDIDEVICEINSTANCE device)
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

b32 win32_DirectInputLogError(HRESULT result, const char* expectation)
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
