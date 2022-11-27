#ifndef DEVICES_H
#define DEVICES_H

typedef struct device device;
struct device
{
  u32 Id;
  b32 Connected;
  telem_state         State;
  telem_packet_queues PacketQueues;
};
u32 TelemDataTypeLookUpTable[8] =
{
  1, 1, 4, 4, 4, 12, 1, 0,
};




//
//b32 win32_SerialPortInitDevice(device *Device);
//void win32_SerialPortFSM(device *Device, platform *Platform);
//void win32_SerialPortRecieveData(device *Device);
//void win32_SerialPortSendData(device *Device, u8 TempThrottle);
//b32 win32_SerialPortHandshake(device *Device, b32 InitiateHandshake);
//void win32_SerialPortCloseDevice(device *Device, win32_state *Win32State);
//b32 VerifyBuffer(u8 *Buffer, s32 Size, HANDLE Log, SYSTEMTIME Time);
//
void ManageReadRequests();
#endif //DEVICES_H
