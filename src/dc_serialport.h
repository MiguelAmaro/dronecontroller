// NOTE(MIGUEL): Should This code go in the windows platfom layer?? and i just keep abastractions and generics here


// 2 Byte header
// payload

#include "dc_telemetry.h"

typedef struct device device;
struct device
{
    b32    Connected;
    HANDLE StreamHandle;
    HANDLE StreamEventHandle;
    OVERLAPPED Overlapped;
    
    telem_packet_queues PacketQueues;
    
    u16 padding;
};

global device g_SerialPortDevice = {0}; 

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


void ManageReadRequests()
{
#if 0
    if(GetOverlappedResult(
                           [in]  HANDLE       hFile,
                           [in]  LPOVERLAPPED lpOverlapped,
                           [out] LPDWORD      lpNumberOfBytesTransferred,
                           [in]  BOOL         bWait
                           ))
    {
        // NOTE(MIGUEL): IO Request was Completed!!!
        ReadFileEx();
    }
    else
    {
        // NOTE(MIGUEL): IO Request pending
        //do nothing
    }
    
#endif
    return;
}


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
    local_persist b32 FirstRead = 1;
    local_persist u8 Buffer[TELEM_PACKET_HEADER_SIZE + TELEM_PAYLOAD_MAXSIZE] = { 0 };
    
    DWORD BytesToRead = TELEM_PACKET_HEADER_SIZE + TELEM_PAYLOAD_MAXSIZE;
    DWORD BytesRead   = 0;
    u32   Event       = 0;
    b32   ExpectingHeader = 1;
    
    if(WaitCommEvent(Device->StreamHandle,
                     &Event,
                     &Device->Overlapped))
    {
        
        u32 BytesTransfered;
        GetOverlappedResult(Device->StreamHandle,
                            &Device->Overlapped,
                            &BytesTransfered,
                            FALSE);
        DWORD Result = GetLastError();
        if((Result != ERROR_IO_PENDING) &&
           (Result != ERROR_IO_INCOMPLETE))
        {
            if(Event & EV_RXCHAR)
            {
                ReadFile(Device->StreamHandle,
                         Buffer,
                         sizeof(telem_packet_header),
                         &BytesRead,
                         0);
            }
            
            telem_packet_header *PacketHeader =
                (telem_packet_header *)Buffer;
            
            
            Device->StreamEventHandle = CreateEventA(NULL, TRUE, FALSE, NULL);
            
            ReadFile(Device->StreamHandle,
                     Buffer + sizeof(telem_packet_header),
                     PacketHeader->PayloadSize,
                     &BytesRead,
                     &Device->Overlapped);
        }
    }
    
    if( WaitForSingleObject(Device->StreamEventHandle, 0))
    {
        telem_packet *TelemetryPacket = (telem_packet *)Buffer;
        
        TelemetryEnqueuePacket(&Device->PacketQueues, Telem_QueueRecieve, *TelemetryPacket);
        
        MemorySet(&Device->Overlapped,
                  sizeof(Device->Overlapped),
                  0);
        
        MemorySet(Buffer, 256, 0);
    }
    else
    {
        ASSERT(ERROR_IO_PENDING == GetLastError());
    }
    
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
                                      0, //FILE_FLAG_OVERLAPPED,
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
    TimeoutsComm.ReadTotalTimeoutMultiplier  = MAXDWORD; // in milliseconds
    TimeoutsComm.WriteTotalTimeoutConstant   = 1; // in milliseconds
    TimeoutsComm.WriteTotalTimeoutMultiplier = 1; // in milliseconds
    
    SetCommTimeouts(Device->StreamHandle, &TimeoutsComm);
    
    DWORD Event;
    if(!SetCommMask(Device->StreamHandle, EV_RXCHAR))
    { return 0; }
    if(!WaitCommEvent(Device->StreamHandle, &Event, NULL))
    { return 0; }
    
    Device->Connected = 1;
    Device->PacketQueues.QueueHead    [0] = 0;
    Device->PacketQueues.QueueTail    [0] = 0;
    Device->PacketQueues.QueueCount   [0] = 0;
    Device->PacketQueues.QueueMaxCount[0] = DEVICE_QUEUE_SIZE;
    
    Device->PacketQueues.QueueHead    [1] = 0;
    Device->PacketQueues.QueueTail    [1] = 0;
    Device->PacketQueues.QueueCount   [1] = 0;
    Device->PacketQueues.QueueMaxCount[1] = DEVICE_QUEUE_SIZE;
    
    return 1;
}

void
win32_SerialPort_SendData(device *Device, telem_packet Packet)
{
    
    u32 BytesWritten = 0;
    b32 TransmissionSuccessful = 0;
    
    TransmissionSuccessful = WriteFile(Device->StreamHandle, // Handle to the Serial port
                                       &Packet, // Data to be written to the port
                                       sizeof(Packet), // Number of bytes to write
                                       &BytesWritten           , // Bytes written
                                       NULLPTR)                ;
    
    if(!TransmissionSuccessful)
    {
        Device->Connected = 0;
    }
    
    return;
}

