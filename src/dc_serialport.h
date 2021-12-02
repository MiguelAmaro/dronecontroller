// NOTE(MIGUEL): Should This code go in the windows platfom layer?? and i just keep abastractions and generics here

#define DEVICE_QUEUE_SIZE 256

// 2 Byte header
// payload

typedef struct telem_packet_header telem_packet_header;
struct telem_packet_header
{
    u8 Info;
    u8 PayloadSize;
};


typedef struct telem_packet telem_packet;
struct telem_packet
{
    telem_packet_header Header;
    u8                  Payload[256];
};

// TODO(MIGUEL): bits[2]
typedef enum telem_type telem_type;
enum telem_type
{
    Telem_Data    = 2,
    Telem_Status  = 1,
    Telem_Address = 0,
    Telem_Frame   = 3,
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

typedef enum telem_state telem_state;
enum telem_state
{
    Telem_Ack
};

typedef enum telem_queue telem_queue;
enum telem_queue
{
    Telem_QueueRecieve  = 0,
    Telem_QueueTransmit = 1,
};

typedef struct device device;
struct device
{
    HANDLE StreamHandle;
    b32    Connected;
    
    telem_packet PacketQueue[2][DEVICE_QUEUE_SIZE];
    u32          PacketQueueMaxCount [2];
    u32          PacketQueueCount    [2];
    u32          PacketQueueHead     [2]; 
    u32          PacketQueueTail     [2]; 
    
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

internaldef void
TelemetryEnqueuePacket(device *Device,
                       telem_queue Direction,
                       telem_packet Packet)
{
    telem_packet *Queue = Device->PacketQueue         [Direction];
    u32        MaxCount =  Device->PacketQueueMaxCount[Direction];
    u32          *Count = &Device->PacketQueueCount   [Direction];
    u32           *Tail = &Device->PacketQueueTail    [Direction]; 
    
    if(*Count < MaxCount)
    {
        telem_packet *Entry = &Queue[*Tail];
        *Entry = Packet;
        
        *Tail = ++*Tail % MaxCount;
        ++*Count;
    }
    
    ASSERT(*Count < MaxCount);
    ASSERT(*Count >= 0);
    
    return;
}

internaldef telem_packet
TelemetryDequeuePacket(device *Device,
                       telem_queue Direction)
{
    
    telem_packet Result = { 0 };
    
    telem_packet *Queue = Device->PacketQueue         [Direction];
    u32        MaxCount =  Device->PacketQueueMaxCount[Direction];
    u32          *Count = &Device->PacketQueueCount   [Direction];
    u32           *Head = &Device->PacketQueueHead    [Direction]; 
    
    if(*Count > 0)
    {
        telem_packet *Entry = &Queue[*Head];
        Result = *Entry;
        
        *Head = ++*Head % MaxCount;
        --*Count;
    }
    
    
    ASSERT(*Count < MaxCount);
    ASSERT(*Count >= 0);
    
    
    return Result;
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
#define TELEM_PACKET_HEADER_SIZE 2

void
win32_SerialPort_RecieveData(device *Device)
{
#if 1
    telem_packet TelemetryPacket = { 0 };
    
    DWORD BytesToRead = 0;
    DWORD BytesRead   = 0;
    b32   ReceptionSuccessful = 0;
    
    BytesToRead = TELEM_PACKET_HEADER_SIZE;
    ReceptionSuccessful = ReadFile(Device->StreamHandle,
                                   &TelemetryPacket.Header,
                                   BytesToRead,
                                   &BytesRead,
                                   NULL);
    
    if(!ReceptionSuccessful)
    { 
        Device->Connected = 0;
        
        return;
    }
    
    u8 Buffer[256] = { 0 };
    BytesRead   = 0;
    BytesToRead = TelemetryPacket.Header.PayloadSize;
    
    ReadFile(Device->StreamHandle,
             Buffer,
             BytesToRead,
             &BytesRead,
             NULL);
    
    if(BytesRead == TelemetryPacket.Header.PayloadSize)
    {
        MemoryCopy(Buffer, 256, TelemetryPacket.Payload, 256);
        
        TelemetryEnqueuePacket(Device, Telem_QueueRecieve, TelemetryPacket);
    }
    
#endif 
    
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
    Device->PacketQueueHead    [0] = 0;
    Device->PacketQueueTail    [0] = 0;
    Device->PacketQueueCount   [0] = 0;
    Device->PacketQueueMaxCount[0] = DEVICE_QUEUE_SIZE;
    
    Device->PacketQueueHead    [1] = 0;
    Device->PacketQueueTail    [1] = 0;
    Device->PacketQueueCount   [1] = 0;
    Device->PacketQueueMaxCount[1] = DEVICE_QUEUE_SIZE;
    
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

