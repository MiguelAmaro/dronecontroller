typedef struct device_win32 device_win32;
struct device_win32
{
  HANDLE StreamHandle;
  HANDLE StreamEventHandle;
  OVERLAPPED AsyncRxIOInfo;
  OVERLAPPED AsyncTxIOInfo;
};

#define DEVICE_MAX_COUNT (4)
HANDLE SerialPortLog;

global device_win32 DeviceWin22[DEVICE_MAX_COUNT] = {0};
global u32 DeviceCount = 0;

b32 DeviceInit(device *Device, char *FilePath, u32 BaudRate, u32 BitCount)
{
  b32 Result = 1;
  //Add new device slot
  if(OutofBounds(DeviceCount, DEVICE_MAX_COUNT))  return 0;
  device_win32 *OSInterface = &DeviceWin22[DeviceCount++];
  OSInterface->StreamHandle = CreateFile(FilePath, GENERIC_READ | GENERIC_WRITE,
                                         0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
  if(OSInterface->StreamHandle == INVALID_HANDLE_VALUE)
  {
    OutputDebugStringA("Error in opening serial port \n");
  }
  
  DCB DeviceSerialParams       = { 0 };
  DeviceSerialParams.DCBlength = sizeof(DeviceSerialParams);
  DeviceSerialParams.BaudRate  = BaudRate  ;    
  DeviceSerialParams.ByteSize  = BitCount  ;         
  DeviceSerialParams.StopBits  = ONESTOPBIT;
  //DeviceSerialParams.EofChar   = 0xff;
  //DeviceSerialParams.EvtChar   = 0xaa;
  DeviceSerialParams.Parity    = NOPARITY  ;
  SetCommState(OSInterface->StreamHandle, &DeviceSerialParams);
  
  COMMTIMEOUTS TimeoutsComm                = { 0 };
  TimeoutsComm.ReadIntervalTimeout         = 1; // in milliseconds
  TimeoutsComm.ReadTotalTimeoutConstant    = 1; // in milliseconds
  TimeoutsComm.ReadTotalTimeoutMultiplier  = MAXDWORD; // in milliseconds
  TimeoutsComm.WriteTotalTimeoutConstant   = 1; // in milliseconds
  TimeoutsComm.WriteTotalTimeoutMultiplier = 1; // in milliseconds
  SetCommTimeouts(OSInterface->StreamHandle, &TimeoutsComm);
  
  DWORD Event;
  if(!SetCommMask(OSInterface->StreamHandle, EV_RXCHAR)) { return 0; }
  
  Device->Connected = 1;
  foreach(Id, 2, u32)
  {
    Device->PacketQueues.QueueHead    [Id] = 0;
    Device->PacketQueues.QueueTail    [Id] = 0;
    Device->PacketQueues.QueueCount   [Id] = 0;
    Device->PacketQueues.QueueMaxCount[Id] = DEVICE_QUEUE_SIZE;
  }
  char *LogFilePath = "F:\\Dev\\DroneController\\res\\SerialPortLog.txt";
  SerialPortLog = CreateFileA(LogFilePath, GENERIC_WRITE, FILE_SHARE_READ,
                              NULL, CREATE_NEW, 0, NULL);
  if(GetLastError() == ERROR_FILE_EXISTS)
  {
    SerialPortLog = CreateFileA(LogFilePath, GENERIC_WRITE, FILE_SHARE_READ,
                                NULL, CREATE_NEW, 0, NULL);
  }
  
  
  SYSTEMTIME Time = { 0 }; GetSystemTime(&Time);
  scratch_mem Scratch = MemoryGetScratch(NULL, 0);
  u8 *LogHeader     = ArenaPushArray(Scratch.Arena, 1024, char);
  u32 LogHeaderSize = wsprintf(LogHeader,
                               "========================================\n"
                               "Log Created %u/%u/%u at %u:%u\n"
                               "========================================\n",
                               Time.wMonth, Time.wDay, Time.wYear, Time.wHour, Time.wMinute);
  WriteFile(SerialPortLog, LogHeader, LogHeaderSize, NULL, NULL);
  MemoryReleaseScratch(Scratch);
  Assert(SerialPortLog != INVALID_HANDLE_VALUE);
  return Result;
};
