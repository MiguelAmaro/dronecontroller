#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <dinputd.h>

global LPDIRECTINPUT8 DirectInput;
global controller Throttle;
global LPDIRECTINPUTDEVICE8 TInterface;
global DIDEVCAPS            TCapabilities;

global controller Flightstick;
global LPDIRECTINPUTDEVICE8 FInterface;
global DIDEVCAPS            FCapabilities;


global u32 gNumFlightDevices = 0;

fn b32 DirectInputLogError(HRESULT ErrorCode, const char* Expects)
{
  b32 Result = TRUE;
  printf("%s - ", Expects);
  printf("Error: ");
  const char *ErrorMsg = NULL;
  switch(ErrorCode)
  {
    case DI_OK: { ErrorMsg = VarToStr(DI_OK); } break;
    case DIERR_OUTOFMEMORY: { ErrorMsg = VarToStr(DIERR_OUTOFMEMORY); } break;
    case DIERR_INVALIDPARAM: { ErrorMsg = VarToStr(DIERR_INVALIDPARAM); } break;
    case DIERR_DEVICENOTREG: { ErrorMsg = VarToStr(DIERR_DEVICENOTREG); } break;
    case DIERR_NOTINITIALIZED: { ErrorMsg = VarToStr(DIERR_NOTINITIALIZED); } break;
    case DIERR_OLDDIRECTINPUTVERSION: { ErrorMsg = VarToStr(DIERR_OLDDIRECTINPUTVERSION); } break;
    case DIERR_BETADIRECTINPUTVERSION: { ErrorMsg = VarToStr(DIERR_BETADIRECTINPUTVERSION); } break;
    default: { printf("Error: %s", "Log Failed \n"); } break;
  }
  printf("%s \n", ErrorMsg);
  switch(ErrorCode)
  {
    case DIERR_BETADIRECTINPUTVERSION:
    case DIERR_INVALIDPARAM:
    case DIERR_OLDDIRECTINPUTVERSION:
    case DIERR_NOTINITIALIZED:
    case DIERR_OUTOFMEMORY:
    case DIERR_DEVICENOTREG:
    case E_HANDLE:
    default: { Result = FALSE; } break;
  }
  return Result;
}
fn b32 ControllerInit(controller *Controller, controller_kind Kind, os_window Window)
{
  // NOTE(MIGUEL): This should be a plat foem independeing insitalizeqitong
  //               This shsould call an abstraction of ve dierct inspt not 
  //              the other whay areound.s
  HRESULT Status;
  LPDIRECTINPUTDEVICE8 Device = ((Kind==ControllerKind_Stick   )?FInterface:
                                 (Kind==ControllerKind_Throttle)?TInterface:NULL);
  DIDEVCAPS *Capabilities = ((Kind==ControllerKind_Stick   )?&FCapabilities:
                             (Kind==ControllerKind_Throttle)?&TCapabilities:NULL);
  Assert(Device);
  Controller->Kind = Kind;
  Status = IDirectInputDevice_SetDataFormat(Device, &c_dfDIJoystick2);
  if(!DirectInputLogError(Status, "Set Device Data Format")) { return 0; }
  Status = IDirectInputDevice_SetCooperativeLevel(Device, (HWND)Window, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
  if(!DirectInputLogError(Status, "Set Device Cooperative Levels")) { return 0; }
  
  Capabilities->dwSize = sizeof(DIDEVCAPS);
  IDirectInputDevice_GetCapabilities(Device, Capabilities);
  if(!DirectInputLogError(Status, "Get Device Capabilities")) { return 0; }
  return 1;
}
BOOL CALLBACK DirectInputEnumDeviceCallback(LPCDIDEVICEINSTANCE Device, LPVOID PrevInstance);
fn b32 DirectInputInit(os_window Window, HINSTANCE Instance)
{
  HRESULT Status = 0;
  Status= DirectInput8Create(Instance, DIRECTINPUT_VERSION, &IID_IDirectInput8, (void **)&DirectInput, NULL);
  Assert(DirectInputLogError(Status, "Create Device Interface"));
  IDirectInput_EnumDevices(DirectInput, DI8DEVCLASS_GAMECTRL, DirectInputEnumDeviceCallback,
                           NULL, DIEDFL_ATTACHEDONLY);
  Assert(DirectInputLogError(Status, "Enumerate Devices And Fetch Desired One"));
  if(FInterface) ControllerInit(&Flightstick, ControllerKind_Stick,    Window);
  if(TInterface) ControllerInit(&Throttle   , ControllerKind_Throttle, Window);
  return 1;
}
BOOL CALLBACK DirectInputEnumDeviceCallback(LPCDIDEVICEINSTANCE Device, LPVOID PrevInstance)
{
  HRESULT Result;
  if(Device->dwDevType & ~DI8DEVTYPE_FLIGHT)
  {
    //controller_id controller;
    if(Device->guidProduct.Data1 == ControllerKind_Stick)
    {
      Result = IDirectInput_CreateDevice(DirectInput, &Device->guidInstance, &FInterface, NULL);
      gNumFlightDevices++;
    }
    if(Device->guidProduct.Data1 == ControllerKind_Throttle)
    {
      Result = IDirectInput_CreateDevice(DirectInput, &Device->guidInstance, &TInterface, NULL);
      gNumFlightDevices++;
    }
    DirectInputLogError(Result, "Create a Device");
  }
  if(TInterface && FInterface)
  {
    return DIENUM_STOP;
  }
  return DIENUM_CONTINUE;
}
fn void PrintLastSystemError(void)
{
  // NOTE(MIGUEL): should be in the main win32 file
  LPTSTR ErrorMsg;
  u32 ErrorCode    = GetLastError();
  u32 ErrorMsgLen = 0;
  ErrorMsgLen = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                               FORMAT_MESSAGE_FROM_SYSTEM     ,
                               NULL                           ,
                               ErrorCode                      ,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                               (u8 *)&ErrorMsg, 0, NULL);
  OutputDebugStringA(ErrorMsg);
  LocalFree(ErrorMsg);
  return;
}
fn HRESULT DirectInputProcessController(controller *Controller, platform *Platform)
{
  HRESULT Status;
  DIJOYSTATE2 StickState;
  LPDIRECTINPUTDEVICE8 Device = ((Controller->Kind==ControllerKind_Stick   )?FInterface:
                                 (Controller->Kind==ControllerKind_Throttle)?TInterface:NULL);
  if(Device == NULL) { return S_OK; }
  
  Status = IDirectInputDevice8_Poll(Device);
  if(FAILED(Status))
  {
    Status = IDirectInputDevice8_Acquire(Device);
    while (Status == DIERR_INPUTLOST) 
    {
      Status = IDirectInputDevice8_Acquire(Device);
    }
    if((Status == DIERR_INVALIDPARAM) || (Status == DIERR_NOTINITIALIZED)) 
    {
      return E_FAIL;
    }
    if(Status == DIERR_OTHERAPPHASPRIO) 
    {
      return S_OK;
    }
  }
  if(FAILED(Status = IDirectInputDevice8_GetDeviceState(Device, sizeof(DIJOYSTATE2), &StickState)))
  {
    return Status; // The device should have been acquired during the Poll()
  }
  if(Controller->Kind==ControllerKind_Throttle)
  {
    Platform->Controls[0].NormThrottlePos = (1.0f - (f32)StickState.lZ / (f32)65535.0f);
  }
  if(Controller->Kind==ControllerKind_Stick)
  {
    Platform->Controls->StickPos.x = (f32)StickState.lX;
    Platform->Controls->StickPos.y = (f32)StickState.lY;
  }
  return S_OK;
}
#if 0
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
                            // -
                            device->guidProduct.Data1,
                            // -
                            device->guidProduct.Data2,
                            // -
                            device->guidProduct.Data3,
                            // -
                            device->guidProduct.Data4[0],
                            device->guidProduct.Data4[1],
                            // -
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
#endif