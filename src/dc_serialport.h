// NOTE(MIGUEL): Should This code go in the windows platfom layer?? and i just keep abastractions and generics here
//#include "RingBuffer.h"
#include "LAL.h"

#define DEVICE_QUEUE_SIZE 256

typedef struct device device;
struct device
{
    HANDLE StreamHandle;
    b32 Connected;
    u8  TransmitQueue[DEVICE_QUEUE_SIZE];
    u8  TransmitQueueSize ;
    u8  RecieveQueue [DEVICE_QUEUE_SIZE];
    u8  RecieveQueueSize  ;
    u16 padding;
};

typedef enum
{
    ddc_dbg_req = (1 << 2),
    ddc_dbg_ack,
    dcc_mcu_clock_freq,
    ddc_dma_dest_addr, 
    ddc_dma_src_addr, 
    ddc_print,
    ddc_print_end,
} device_debug_code;


internaldef void
DebugPrint()
{
    
    
    return;
}

global device g_SerialPortDevice = {0}; 


// TODO(MIGUEL): bits[2]
typedef enum telem_type telem_type;
enum telem_type
{
    Telem_Data    = 2,
    Telem_Status  = 1,
    Telem_Address = 0,
};

// TODO(MIGUEL): bits[3]
typedef enum telem_data_type telem_data_type;
enum telem_data_type
{
    Telem_s8    = 0,
    Telem_u8    = 1,
    Telem_u32   = 2,
    Telem_str8  = 3,
    Telem_f32   = 4,
    Telem_v3f32 = 5,
    Telem_s32   = 6,
};

u32 TelemDataTypeLookUpTable[8] =
{
    1, 1, 4, 4, 4, 12, 1, 0,
};

/* --- PRINTF_BYTE_TO_BINARY macro's --- */
#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i)    \
(((i) & 0x80ll) ? '1' : '0'), \
(((i) & 0x40ll) ? '1' : '0'), \
(((i) & 0x20ll) ? '1' : '0'), \
(((i) & 0x10ll) ? '1' : '0'), \
(((i) & 0x08ll) ? '1' : '0'), \
(((i) & 0x04ll) ? '1' : '0'), \
(((i) & 0x02ll) ? '1' : '0'), \
(((i) & 0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 \
PRINTF_BINARY_PATTERN_INT8              PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
PRINTF_BYTE_TO_BINARY_INT8((i) >> 8),   PRINTF_BYTE_TO_BINARY_INT8(i)
#define PRINTF_BINARY_PATTERN_INT32 \
PRINTF_BINARY_PATTERN_INT16             PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i) \
PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), PRINTF_BYTE_TO_BINARY_INT16(i)
#define PRINTF_BINARY_PATTERN_INT64    \
PRINTF_BINARY_PATTERN_INT32             PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i) \
PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), PRINTF_BYTE_TO_BINARY_INT32(i)
/* --- end macros --- */



void
win32_SerialPort_CloseDevice(device *Device, win32_state *Win32State)
{
    Device->Connected = 0;
    
    CloseHandle(Win32State->CommThreadInfo.Handle);
    Win32State->CommThreadInfo.Handle = 0;
    
    CloseHandle(Device->StreamHandle); /// Closing the Serial Port
    
    return;
}


void
win32_SerialPort_RecieveData(device *Device)
{
    
    /*u8 *StringToPrint = (u8 *)lpParameter;
            
            OutputDebugStringA(StringToPrint);
            // Reading
            */
    u8 TempByte = 0;
    DWORD BytesToRead;
    b32 ReceptionSuccessful = 0;
    
    memset(Device->RecieveQueue, 0, Device->RecieveQueueSize);
    
    int i = 0;
    
    //do
    
    DWORD Event;
    if(WaitCommEvent(Device->StreamHandle, &Event, NULL))
    {
        ReceptionSuccessful = ReadFile(Device->StreamHandle, //Handle of the Serial port
                                       &TempByte          ,
                                       sizeof(TempByte)   ,
                                       &BytesToRead       ,
                                       NULL              );
        
        
        if(!ReceptionSuccessful)
        {
            Device->Connected = 0;
            //break;
        }
        //000 0111
        u32 TelemetryMsgType     = (TempByte >> 6) & 0x2;
        u32 TelemetryMsgDataType = (TempByte >> 3) & 0x7;
        
        printf(PRINTF_BINARY_PATTERN_INT8"\n", PRINTF_BYTE_TO_BINARY_INT8(TempByte));
        switch(TelemetryMsgType)
        {
            case Telem_Data:
            {
                printf("Telemetry Data Message Recieved\n");
            } break;
            
            case Telem_Status:
            {
                printf("Telemetry Status Message Recieved\n");
            } break;
        }
        
        switch(TelemetryMsgDataType)
        {
            case Telem_str8:
            {
                printf("Telemetry Data is String8 \n");
            } break;
            
            case Telem_u8:
            {
                printf("Telemetry Data is String 8\n");
            } break;
            
        }
        
        Device->RecieveQueue[i] = TempByte;
        i++;
    }
    //while (BytesToRead > 0 && i < 1); //256);
    
    return;
}

// NOTE(MIGUEL): should take params to decide a device
b32
win32_SerialPort_InitDevice(win32_state *Win32State, device *Device)
{
    // Created An IO Stream to talk with micro controller
    
    
    
    // NOTE(MIGUEL): How do I know which com it is??
    // TODO(MIGUEL): Find out why comm get initialize with garbage
    Device->StreamHandle = CreateFile("\\\\.\\COM12",
                                      GENERIC_READ | GENERIC_WRITE,
                                      0,
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_FLAG_OVERLAPPED,
                                      NULL);
    
    
    if(Device->StreamHandle == INVALID_HANDLE_VALUE)
    {
        OutputDebugStringA("Error in opening serial port \n");
        return 0;
    }
    else
    {
        OutputDebugStringA("Opening serial port successful \n");
    }
    
    DCB DeviceSerialParams       = { 0 }; // Initializing DCB structure
    DeviceSerialParams.DCBlength = sizeof(DeviceSerialParams);
    DeviceSerialParams.BaudRate  = 115200    ;    
    DeviceSerialParams.ByteSize  = 8         ;         
    DeviceSerialParams.StopBits  = ONESTOPBIT;
    //DeviceSerialParams.EofChar   = 0xff;
    //DeviceSerialParams.EvtChar   = 0xaa;
    DeviceSerialParams.Parity    = NOPARITY  ;
    
    SetCommState(Device->StreamHandle, &DeviceSerialParams);
    
    
    COMMTIMEOUTS TimeoutsComm                = { 0 };
    TimeoutsComm.ReadIntervalTimeout         = 1; // in milliseconds
    TimeoutsComm.ReadTotalTimeoutConstant    = 1; // in milliseconds
    TimeoutsComm.ReadTotalTimeoutMultiplier  = 1; // in milliseconds
    TimeoutsComm.WriteTotalTimeoutConstant   = 1; // in milliseconds
    TimeoutsComm.WriteTotalTimeoutMultiplier = 1; // in milliseconds
    
    SetCommTimeouts(Device->StreamHandle, &TimeoutsComm);
    
    b32 status = 0;
    
    if(!SetCommMask(Device->StreamHandle, EV_RXCHAR))
    { return 0; }
    
    Device->Connected = 1;
    Device->TransmitQueueSize = DEVICE_QUEUE_SIZE;
    Device->RecieveQueueSize  = DEVICE_QUEUE_SIZE;
    
#if 0
    u8 *Param = "Thread Started !!!\n\r";
    
    Win32State->CommThreadInfo.LogicalThreadIndex = 0;
    DWORD ThreadID;
    Win32State->CommThreadInfo.Handle = CreateThread(0, 0,
                                                     win32_SerialPort_RecieveData,
                                                     Param,
                                                     0,
                                                     &Win32State->CommThreadInfo.ID);
#endif
    return 1;
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
        Device->Connected = 0;
    }
    
    return;
}

