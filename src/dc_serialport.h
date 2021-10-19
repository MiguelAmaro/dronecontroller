// NOTE(MIGUEL): Should This code go in the windows platfom layer?? and i just keep abastractions and generics here
//#include "RingBuffer.h"
#include "LAL.h"

typedef struct device device;
struct device
{
    HANDLE StreamHandle;
    b32 Connected;
    u8  TransmitQueue[256];
    u8  RecieveQueue [256];
    u16 padding;
};

global device g_SerialPortDevice = {0}; 


b32 win32_SerialPort_InitDevice(device *Device);


// NOTE(MIGUEL): should take params to decide a device
b32
win32_SerialPort_InitDevice(device *Device)
{
    // Created An IO Stream to talk with micro controller
    
    // NOTE(MIGUEL): How do I know which com it is??
    // TODO(MIGUEL): Find out why comm get initialize with garbage
    Device->StreamHandle = CreateFile("\\\\.\\COM12",
                                      GENERIC_READ | GENERIC_WRITE,
                                      0,
                                      NULL,
                                      OPEN_EXISTING,
                                      0,
                                      NULL);
    
    
    if(Device->StreamHandle == INVALID_HANDLE_VALUE)
    {
        printf("Error in opening serial port \n");
        return 0;
    }
    else
    {
        printf("Opening serial port successful \n");
    }
    
    DCB DeviceSerialParams       = { 0 }; // Initializing DCB structure
    DeviceSerialParams.DCBlength = sizeof(DeviceSerialParams);
    DeviceSerialParams.BaudRate  = 115200    ;    
    DeviceSerialParams.ByteSize  = 8         ;         
    DeviceSerialParams.StopBits  = ONESTOPBIT;
    DeviceSerialParams.Parity    = NOPARITY  ;
    
    SetCommState(Device->StreamHandle, &DeviceSerialParams);
    
    
    COMMTIMEOUTS TimeoutsComm                = { 0 };
    TimeoutsComm.ReadIntervalTimeout         = 50; // in milliseconds
    TimeoutsComm.ReadTotalTimeoutConstant    = 50; // in milliseconds
    TimeoutsComm.ReadTotalTimeoutMultiplier  = 10; // in milliseconds
    TimeoutsComm.WriteTotalTimeoutConstant   = 50; // in milliseconds
    TimeoutsComm.WriteTotalTimeoutMultiplier = 10; // in milliseconds
    
    SetCommTimeouts(Device->StreamHandle, &TimeoutsComm);
    
    DWORD dwEventMask;
    b32 status = 0;
    
    if(!SetCommMask(Device->StreamHandle, EV_RXCHAR))
    { return 0; }
    
    if(!WaitCommEvent(Device->StreamHandle, &dwEventMask, NULL))
    { return 0; }
    
    Device->Connected = 1;
    
    return 1;
}

void
win32_SerialPort_CloseDevice(device *Device)
{
    Device->Connected = 0;
    CloseHandle(Device->StreamHandle); /// Closing the Serial Port
    
    return;
}

void
win32_SerialPort_SendData(device *Device, u8 *Bytes, u32 Size)
{
    
    u32 BytesWritten = 0;
    b32 TransmissionSuccessful = 0;
    
    TransmissionSuccessful = WriteFile(Device->StreamHandle    , // Handle to the Serial port
                                       Bytes                   , // Data to be written to the port
                                       Size                    , // Number of bytes to write
                                       &BytesWritten           , // Bytes written
                                       NULLPTR)                ;
    
    if(!TransmissionSuccessful)
    {
        win32_SerialPort_CloseDevice(Device);
    }
    
    return;
}

void
win32_SerialPort_RecieveData(device *Device)
{
    // Reading
    
    u8 TempByte;
    DWORD BytesToRead;
    b32 ReceptionSuccessful = 0;
    
    int i = 0;
    
    do
    {
        ReceptionSuccessful = ReadFile(Device->StreamHandle, //Handle of the Serial port
                                       &TempByte          ,
                                       sizeof(TempByte)   ,
                                       &BytesToRead       ,
                                       NULL              );
        
        
        if(!ReceptionSuccessful)
        {
            win32_SerialPort_CloseDevice(Device);
            break;
        }
        
        Device->RecieveQueue[i] = TempByte;
        i++;
    }
    while (BytesToRead > 0 && i < 256);
    
    return;
}
