// NOTE(MIGUEL): Should This code go in the windows platfom layer?? and i just keep abastractions and generics here
#include "LAL.h"

// TODO(MIGUEL): Align struct | get rid of windows typedef
// NOTE(MIGUEL): change name to device? does this reperesnt the device?
typedef struct IO_Message
{
    HANDLE comm;
    u8* buffer;
    DWORD bytes_to_write;
    DWORD bytes_written;
    DWORD status;
} IO_Message;

global IO_Message global_Device = {0}; 

void
Init_SerialPort(void)
{
    
    // Created An IO Stream to talk with micro controller
    
    HANDLE Comm_Handle;
    
    // NOTE(MIGUEL): How do I know which com it is??
    // TODO(MIGUEL): Find out why comm get initialize with garbage
    global_Device.comm = CreateFile("\\\\.\\COM3",
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    0,
                                    NULL);
    
    
    if(global_Device.comm == INVALID_HANDLE_VALUE)
    {
        printf("Error in opening serial port \n");
        
    }
    else
    {
        printf("Opening serial port successful \n");
    }
    
    DCB DeviceSerialParams = { 0 }; // Initializing DCB structure
    DeviceSerialParams.DCBlength = sizeof(DeviceSerialParams);
    DeviceSerialParams.BaudRate = CBR_115200;  // Setting BaudRate = 9600
    DeviceSerialParams.ByteSize = 8;         // Setting ByteSize = 8
    DeviceSerialParams.StopBits = ONESTOPBIT;// Setting StopBits = 1
    DeviceSerialParams.Parity   = NOPARITY;  // Setting Parity = None
    
    SetCommState(global_Device.comm, &DeviceSerialParams);
    
    
    COMMTIMEOUTS TimeoutsComm = { 0 };
    TimeoutsComm.ReadIntervalTimeout         = 50; // in milliseconds
    TimeoutsComm.ReadTotalTimeoutConstant    = 50; // in milliseconds
    TimeoutsComm.ReadTotalTimeoutMultiplier  = 10; // in milliseconds
    TimeoutsComm.WriteTotalTimeoutConstant   = 50; // in milliseconds
    TimeoutsComm.WriteTotalTimeoutMultiplier = 10; // in milliseconds
    
    SetCommTimeouts(global_Device.comm, &TimeoutsComm);
    
    // Writing 
    
    local_persist u8 lpBuffer[] = "b";
    global_Device.buffer = lpBuffer;
    DWORD dNoOFBytestoWrite = sizeof(lpBuffer);         // No of bytes to write into the port
    DWORD dNoOfBytesWritten = 0;     // No of bytes written to the port
    dNoOFBytestoWrite = sizeof(lpBuffer);
    
    global_Device.status = WriteFile(global_Device.comm,        // Handle to the Serial port
                                     lpBuffer,     // Data to be written to the port
                                     dNoOFBytestoWrite,  //No of bytes to write
                                     &dNoOfBytesWritten, //Bytes written
                                     NULL);
    
    
    // Reading
    
    DWORD dwEventMask;
    
    global_Device.status =  SetCommMask(global_Device.comm, EV_RXCHAR);
    
    global_Device.status = WaitCommEvent(global_Device.comm, &dwEventMask, NULL);
    
    
    //RoundBuffer* SerialBuffer = RoundBuffer_Create(256);
    
    u8 TempChar;
    u8 SerialBuffer[256];
    DWORD NoBytesRead;
    int i = 0;
    // What does this do?
    do
    {
        ReadFile( global_Device.comm,           //Handle of the Serial port
                 &TempChar,       //Temporary character
                 sizeof(TempChar),//Size of TempChar
                 &NoBytesRead,    //Number of bytes read
                 NULL);
        
        SerialBuffer[i] = TempChar;
        
        //RoundBuffer_Enqueue(SerialBuffer, &TempChar);// Store Tempchar into buffer
        i++;
    }while (NoBytesRead > 0);
    
    //printf((char*)&(RoundBuffer_Dequeue(SerialBuffer)));
    printf(SerialBuffer);
    
    
    
    //CloseHandle(global_Device.comm);//Closing the Serial Port
    
    
    return;
}