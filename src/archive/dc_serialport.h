// NOTE(MIGUEL): Should This code go in the windows platfom layer?? and i just keep abastractions and generics here


// 2 Byte header
// payload

#include "dc_telemetry.h"

// NOTE(MIGUEL): device comms
typedef struct device device;
struct device
{
  b32    Connected;
  HANDLE StreamHandle;
  HANDLE StreamEventHandle;
  OVERLAPPED AsyncRxIOInfo;
  OVERLAPPED AsyncTxIOInfo;
  
  telem_state         State;
  telem_packet_queues PacketQueues;
  
  u16 padding;
};

HANDLE SerialPortLog;

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

b32
VerifyBuffer(u8 *Buffer, s32 Size, HANDLE Log, SYSTEMTIME Time)
{
  u8  Msg[4096];
  u32 MsgSize = 0;
  b32 Result = 0;
  b32 ByteWritten = 0;
  u32 NullLimit = 100;
  u32 NullCount = 0;
  b32 IsNullMsg = 1;
  s32 Byte = sizeof(telem_packet_header); //skip the header
  
  for(; Byte < Size; Byte++)
  {
    if(Buffer[Byte] != 0)
    {
      IsNullMsg = 0;
      NullCount = 0;
    }
    else
    {
      //NullCount++;
      // TODO(MIGUEL): Set the Byte var back by NullCount b4 breaking.
      //if(NullCount > NullLimit) Byte -= NullCount; break;
    }
  }
  
  if(IsNullMsg)
  {
#if 0
    MemorySet(Msg, 4096, 0);
    u32 MsgSize = wsprintf(Msg, "Nothing written to the buffer.\n");
    
    WriteFile(SerialPortLog, // Handle to the Serial port
              Msg,           // Data to be written to the port
              MsgSize,       // Number of bytes to write
              NULLPTR,       // Bytes written
              NULLPTR);
    
#endif
    Result = 0;
  }
  else
  {
    
    MemorySet(Msg, 4096, 0);
    MsgSize = wsprintf(Msg, "LOG: [%u:%u:%u]\n",
                       Time.wMinute,
                       Time.wSecond,
                       Time.wMilliseconds);
    
    WriteFile(SerialPortLog, // Handle to the Serial port
              Msg,           // Data to be written to the port
              MsgSize,       // Number of bytes to write
              NULLPTR,       // Bytes written
              NULLPTR);
    
    MemorySet(Msg, 4096, 0);
    MsgSize = wsprintf(Msg, 
                       "Expected Length: %u\n"
                       "Msg Size: %d bytes\n"
                       "NULL Count: %d bytes\n",
                       (u32)Buffer[1],
                       Byte,
                       TELEM_PAYLOAD_MAXSIZE - Byte + sizeof(telem_packet_header));
    
    WriteFile(SerialPortLog, // Handle to the Serial port
              Msg,           // Data to be written to the port
              MsgSize,       // Number of bytes to write
              NULLPTR,       // Bytes written
              NULLPTR);
    
    MemorySet(Msg, 4096, 0);
    
    u32 BytesWritten = 0;
    BytesWritten += wsprintf(Msg + BytesWritten, "Header:\n");
    BytesWritten += MemoryHexDump(Msg  + BytesWritten,
                                  4096 - BytesWritten,
                                  Buffer, sizeof(telem_packet_header),
                                  2, 1, 8);
    BytesWritten += wsprintf(Msg + BytesWritten, "Message:\n");
    BytesWritten += MemoryHexDump(Msg  + BytesWritten,
                                  4096 - BytesWritten,
                                  Buffer + sizeof(telem_packet_header), TELEM_PAYLOAD_MAXSIZE,
                                  32, 8, 4);
    
    
    
    WriteFile(SerialPortLog, // Handle to the Serial port
              Msg,
              BytesWritten,                 // Number of bytes to write
              NULLPTR,              // Bytes written
              NULLPTR);
    
    
    WriteFile(SerialPortLog, "\n", 1, NULLPTR, NULLPTR);
    Result = 1;
  }
  
  return Result;
}

void
win32_SerialPortCloseDevice(device *Device, win32_state *Win32State)
{
  Device->Connected = 0;
  
  CloseHandle(Win32State->CommThreadInfo.Handle);
  Win32State->CommThreadInfo.Handle = 0;
  
  CloseHandle(Device->StreamHandle); /// Closing the Serial Port
  
  return;
}

b32 win32_SerialPortHandshake(device *Device, b32 InitiateHandshake)
{
  b32 HandshakeSuccesful = 0;
  
  local_persist b32 HandshakePingSent = 0;
  local_persist u8  HandshakePing     = TELEM_STATUS_PING;
  local_persist u8  HandshakeFinalAck = Telem_Ack;
  local_persist u32 BytesWritten = 0;
  local_persist u32 BytesRead    = 0;
  
  if(InitiateHandshake == 1 ||
     HandshakePingSent == 0)
  {
    WriteFile(Device->StreamHandle,
              &HandshakePing,
              sizeof(u8),
              &BytesWritten,
              &Device->AsyncTxIOInfo);
    
    printf("sending ping...\n");
    printf("expecting "
           "Ping:" PRINTF_BINARY_PATTERN_INT8" and "
           "ACK:"  PRINTF_BINARY_PATTERN_INT8
           " ...\n",
           PRINTF_BYTE_TO_BINARY_INT8(TELEM_STATUS_PING),
           PRINTF_BYTE_TO_BINARY_INT8(Telem_Ack));
    HandshakePingSent = 1;
  }
  
  if(HandshakePingSent == 1)
  {
    local_persist u8 Buffer[2];
    
    
    ReadFile(Device->StreamHandle,
             &Buffer,
             sizeof(u8) * 2,
             NULLPTR,
             &Device->AsyncRxIOInfo);
    
    if((Buffer[0] == TELEM_STATUS_PING) &&
       (Buffer[1] == Telem_Ack))
    {
      WriteFile(Device->StreamHandle,
                &HandshakeFinalAck,
                sizeof(u8),
                &BytesWritten,
                &Device->AsyncTxIOInfo);
      
      printf("ping acknowledged, Handshake Succesful...\n");
      HandshakeSuccesful = 1;
    }
  }
  
  return HandshakeSuccesful;
}

void
win32_SerialPortSendData(device *Device, u8 TempThrottle)
{
#if 0
  u32 BytesWritten = 0;
  b32 TransmissionSuccessful = 0;
  
  telem_packet Packet = TelemetryDequeuePacket(&Device->PacketQueues, Telem_QueueTransmit);
  
  TransmissionSuccessful = WriteFile(Device->StreamHandle, // Handle to the Serial port
                                     &Packet,              // Data to be written to the port
                                     sizeof(Packet),       // Number of bytes to write
                                     &BytesWritten,        // Bytes written
                                     NULLPTR);
  
  if(!TransmissionSuccessful)
  {
    Device->Connected = 0;
  }
  
#else
  u32 BytesWritten = 0;
  b32 TransmissionSuccessful = 0;
  
  TransmissionSuccessful = WriteFile(Device->StreamHandle, // Handle to the Serial port
                                     &TempThrottle,              // Data to be written to the port
                                     sizeof(TempThrottle),       // Number of bytes to write
                                     &BytesWritten,        // Bytes written
                                     NULL);//&Device->AsyncTxIOInfo);
  /*
  if(!TransmissionSuccessful)
  {
      Device->Connected = 0;
  }*/
#endif
  return;
}


void
win32_SerialPortRecieveData(device *Device)
{
  b32 RecieveRequestSuccessful;
  
  local_persist b32 FirstRead = 1;
  local_persist u8 Buffer[TELEM_PACKET_HEADER_SIZE + TELEM_PAYLOAD_MAXSIZE] = { 0 };
  
  DWORD BytesToRead = TELEM_PACKET_HEADER_SIZE + TELEM_PAYLOAD_MAXSIZE;
  DWORD BytesRead   = 0;
  u32   Event       = 0;
  b32   ExpectingHeader = 1;
  
  
  // NOTE(MIGUEL): Make and comm event so i get notified when windows
  //               recieves a character from MCU on the comm port.
  WaitCommEvent(Device->StreamHandle,
                &Event,
                &Device->AsyncRxIOInfo);
  
  // NOTE(MIGUEL): Forgot what i indtended with this
  u32 BytesTransfered;
  GetOverlappedResult(Device->StreamHandle,
                      &Device->AsyncRxIOInfo,
                      &BytesTransfered,
                      FALSE);
  
  // NOTE(MIGUEL): Make sure i havent already sent out a read request.
  //               If i haven i check if the char came using commevent and 
  //               if it did send a request to data in to a temp buffer.
  
  DWORD Result = GetLastError();
  if((Result != ERROR_IO_PENDING) &&
     (Result != ERROR_IO_INCOMPLETE))
  {
    if(Event & EV_RXCHAR)
    {
      RecieveRequestSuccessful = ReadFile(Device->StreamHandle,
                                          Buffer,
                                          sizeof(telem_packet_header),
                                          &BytesRead,
                                          &Device->AsyncRxIOInfo);
      
      
      if(!RecieveRequestSuccessful)
      {
        //Device->Connected = 0;
      }
    }
    
    telem_packet_header *PacketHeader =
      (telem_packet_header *)Buffer;
    
    
    Device->StreamEventHandle = CreateEventA(NULL, TRUE, FALSE, NULL);
    
    ReadFile(Device->StreamHandle,
             Buffer + sizeof(telem_packet_header),
             PacketHeader->PayloadSize,
             &BytesRead,
             &Device->AsyncRxIOInfo);
    
  }
  
  SYSTEMTIME Time = { 0 }; GetSystemTime(&Time);
  
  telem_packet *TelemetryPacket = (telem_packet *)Buffer;
  
  
  // NOTE(MIGUEL): This function is causing some sort of corruption in the stack
  //               which cause crt code to access invalid memory. The accurs randomly
  //               except when clicking the background console which guarentees the 
  //               the access violation
  //VerifyBuffer(Buffer, sizeof(Buffer), SerialPortLog, Time);
  
  TelemetryEnqueuePacket(&Device->PacketQueues, Telem_QueueRecieve, *TelemetryPacket);
  
  //MemorySet(Buffer, 256, 0);
  MemorySet(&Device->AsyncRxIOInfo,
            sizeof(Device->AsyncRxIOInfo),
            0);
  
  if(1)//WAIT_OBJECT_0 == WaitForSingleObject(Device->StreamEventHandle, 0))
  {
  }
  
  else
  {
    win32_PrintLastSystemError();
  }
  
  return;
}

void
win32_SerialPortFSM(device *Device, platform *Platform)
{
  local_persist u32 LastServicedQueue = 0;
  
  if(g_SerialPortDevice.Connected)
  {
    switch(Device->State)
    {
      case Telem_NoConnection:
      {
        b32 InitiateHandshake = 0;
        // TODO(MIGUEL): make this keyboard controlled
        if(Platform->Controls[0].AlphaKeys[Key_d].IsReleasedNow)
        {
          InitiateHandshake = 1;
        }
        
        if(win32_SerialPortHandshake(Device, InitiateHandshake))
        {
          Device->State = Telem_Waiting;
        }
      } break;
      
      case Telem_Waiting:
      {
        printf("polling drone..");
        win32_SerialPortRecieveData(Device);
        
        u8 TempThrottle = (u8)(Platform->Controls[0].NormThrottlePos * 256);
        win32_SerialPortSendData(Device, TempThrottle);
      } break;
    }
  }
  else
  {
    win32_SerialPortInitDevice(Device);
    Device->State = Telem_NoConnection;
  }
  
  return;
}

// NOTE(MIGUEL): should take params to decide a device
b32
win32_SerialPortInitDevice(device *Device)
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
  TimeoutsComm.ReadTotalTimeoutMultiplier  = MAXDWORD; // in milliseconds
  TimeoutsComm.WriteTotalTimeoutConstant   = 1; // in milliseconds
  TimeoutsComm.WriteTotalTimeoutMultiplier = 1; // in milliseconds
  
  SetCommTimeouts(Device->StreamHandle, &TimeoutsComm);
  
  DWORD Event;
  if(!SetCommMask(Device->StreamHandle, EV_RXCHAR))
  { return 0; }
  //if(!WaitCommEvent(Device->StreamHandle, &Event, NULL))
  //{ return 0; }
  
  Device->Connected = 1;
  Device->PacketQueues.QueueHead    [0] = 0;
  Device->PacketQueues.QueueTail    [0] = 0;
  Device->PacketQueues.QueueCount   [0] = 0;
  Device->PacketQueues.QueueMaxCount[0] = DEVICE_QUEUE_SIZE;
  
  Device->PacketQueues.QueueHead    [1] = 0;
  Device->PacketQueues.QueueTail    [1] = 0;
  Device->PacketQueues.QueueCount   [1] = 0;
  Device->PacketQueues.QueueMaxCount[1] = DEVICE_QUEUE_SIZE;
  
  // NOTE(MIGUEL): Create Log File
  
  SerialPortLog = CreateFileA("F:\\Dev\\DroneController\\res\\SerialPortLog.txt",
                              GENERIC_WRITE,
                              FILE_SHARE_READ,
                              NULL,
                              CREATE_NEW,
                              0,
                              NULL);
  
  if(GetLastError() == ERROR_FILE_EXISTS)
  {
    
    SerialPortLog = CreateFileA("F:\\Dev\\DroneController\\res\\SerialPortLog.txt",
                                GENERIC_WRITE,
                                FILE_SHARE_READ, NULL,
                                TRUNCATE_EXISTING,
                                0, NULL );
    
#if 0
    replay_buffer->memory_map = CreateFileMapping(replay_buffer->file_handle,
                                                  0, PAGE_READWRITE,
                                                  max_size.HighPart,
                                                  max_size.LowPart, 0);
    
    replay_buffer->memory_block = MapViewOfFile(replay_buffer->memory_map,
                                                FILE_MAP_ALL_ACCESS,
                                                0, 0,
                                                state_win32.main_memory_block_size);
#endif
    
#if 0
    SetFilePointerEx(debug_file,
                     (LARGE_INTEGER){0, 0},
                     &debug_file_ptr,
                     FILE_END);
#endif
  }
  
  SYSTEMTIME Time = { 0 }; GetSystemTime(&Time);
  
  u8 Msg[256] = { 0 };
  u32 MsgSize = wsprintf(Msg,
                         "========================================\n"
                         "Log Created %u/%u/%u at %u:%u\n"
                         "========================================\n",
                         Time.wMonth,
                         Time.wDay,
                         Time.wYear,
                         Time.wHour,
                         Time.wMinute);
  
  WriteFile(SerialPortLog, // Handle to the Serial port
            Msg,           // Data to be written to the port
            MsgSize,       // Number of bytes to write
            NULLPTR,       // Bytes written
            NULLPTR);
  
  Assert(SerialPortLog != INVALID_HANDLE_VALUE);
  
  return 1;
}


