#include "LAL.h"

#define DEFAULT_VALUE (0)

typedef struct
{
    u8* data;
    u16 capacity;
    u16 size;
    u16 head;
    u16 tail;
} RoundBuffer;

RoundBuffer* RoundBuffer_Create(u16 capacity) {
    RoundBuffer* buffer = malloc(sizeof(RoundBuffer));
    
    if(buffer)
    {
        buffer->data = malloc(sizeof(u8) * capacity);
        buffer->capacity = capacity;
        buffer->size = 0;
        buffer->head = 0;
        buffer->tail = 0;
        
        if(buffer->data){
            for(u32 dataIndex = 0; dataIndex < capacity; dataIndex++)
            {
                buffer->data[dataIndex] = DEFAULT_VALUE;
            }
        }
        else 
        {
            printf("Allocation For RoundBuffer->Data(PTR) Failed");
        }
    }
    else{
        printf("Allocation For RoundBuffer(PTR) Failed");
    }
    
    return buffer;
}

BOOL RoundBuffer_Full(RoundBuffer* buffer) {
    
    return buffer->size == buffer->capacity;
}

BOOL RoundBuffer_Empty(RoundBuffer* buffer) {
    
    return buffer->size == 0;
}

void RoundBuffer_Destroy(RoundBuffer* buffer) {
    free(buffer->data);
    free(buffer);
    
    return;
}

void RoundBuffer_Enqueue(RoundBuffer *buffer, readonly u8 *input) {
    while(input)
    {        
        if(!RoundBuffer_Full(buffer)){
            buffer->data[buffer->tail++] = *input;
            buffer->tail %= buffer->capacity;
            buffer->size++;
        }
    }
    
    return;
}

u8 RoundBuffer_Dequeue(RoundBuffer* buffer) {
    u8 data = 0;
    
    if(!RoundBuffer_Empty(buffer))
    {
        data = buffer->data[buffer->head];
        buffer->data[buffer->head++] = DEFAULT_VALUE;
        buffer->head %= buffer->capacity;
        buffer->size--;
    }
    
    return data;   
}
