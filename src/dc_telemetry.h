/* date = December 3rd 2021 0:39 am */

#ifndef DC_TELEMETRY_H
#define DC_TELEMETRY_H


#define TELEM_PACKET_HEADER_SIZE 2
#define TELEM_PAYLOAD_MAXSIZE 256
#define DEVICE_QUEUE_SIZE 256

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
    u8                  Payload[TELEM_PAYLOAD_MAXSIZE];
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



#define TELEM_STATUS_ACK   (1 << 4)
#define TELEM_STATUS_NOACK (1 << 3)
#define TELEM_STATUS_PING  (1 << 0)

typedef enum telem_status_code telem_status_code;
enum telem_status_code
{
    Telem_Ack   = (1 << 4),
    Telem_Noack = (1 << 1),
};

typedef enum telem_state telem_state;
enum telem_state
{ 
    Telem_NoConnection,
    Telem_Handshake,
    Telem_Waiting,
    Telem_ReceivingPacket,
    Telem_TransmittingPacket,
};

typedef enum telem_queue telem_queue;
enum telem_queue
{
    Telem_QueueRecieve  = 0,
    Telem_QueueTransmit = 1,
};

typedef struct telem_packet_queues telem_packet_queues;
struct telem_packet_queues
{
    telem_packet Queue[2][DEVICE_QUEUE_SIZE];
    u32          QueueMaxCount [2];
    u32          QueueCount    [2];
    u32          QueueHead     [2]; 
    u32          QueueTail     [2]; 
};

internaldef b32
TelemetryQueueFull(telem_packet_queues *Queues, telem_queue Direction)
{
    b32 Result = 0;
    
    Result = (&Queues->QueueCount[Direction] >= &Queues->QueueMaxCount[Direction]);
    
    return Result;
}

internaldef b32
TelemetryQueueEmpty(telem_packet_queues *Queues, telem_queue Direction)
{
    b32 Result = 0;
    
    Result = (&Queues->QueueCount[Direction] <= 0);
    
    return Result;
}

internaldef void
TelemetryEnqueuePacket(telem_packet_queues *Queues,
                       telem_queue Direction,
                       telem_packet Packet)
{
    telem_packet *Queue =  Queues->Queue        [Direction];
    u32        MaxCount =  Queues->QueueMaxCount[Direction];
    u32          *Count = &Queues->QueueCount   [Direction];
    u32           *Tail = &Queues->QueueTail    [Direction]; 
    
    if(*Count < MaxCount)
    {
        telem_packet *Entry = &Queue[*Tail];
        *Entry = Packet;
        
        *Tail = ++*Tail % MaxCount;
        ++*Count;
    }
    
    ASSERT(*Count <= MaxCount);
    ASSERT(*Count >= 0);
    
    return;
}

internaldef telem_packet
TelemetryDequeuePacket(telem_packet_queues *Queues,
                       telem_queue Direction)
{
    
    telem_packet Result = { 0 };
    
    telem_packet *Queue =  Queues->Queue         [Direction];
    u32        MaxCount =  Queues->QueueMaxCount[Direction];
    u32          *Count = &Queues->QueueCount   [Direction];
    u32           *Head = &Queues->QueueHead    [Direction]; 
    
    if(*Count > 0)
    {
        telem_packet *Entry = &Queue[*Head];
        Result = *Entry;
        
        *Head = ++*Head % MaxCount;
        --*Count;
    }
    
    
    ASSERT(*Count <= MaxCount);
    ASSERT(*Count >= 0);
    
    
    return Result;
}

#endif //DC_TELEMETRY_H
