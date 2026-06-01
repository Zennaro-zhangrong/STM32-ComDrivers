#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef struct {
    unsigned char *buffer;       // 指向环形缓冲区数据的指针
    unsigned short size;           // 缓冲区的大小
    volatile unsigned short head;           // 头部索引
    volatile unsigned short tail;           // 尾部索引
    volatile unsigned short count;             // 缓冲区存储的总字节数
} RingBuffer;


void RingBuffer_Init(RingBuffer *rb, unsigned char *buf, unsigned short size);
unsigned char RingBuffer_WriteByte(RingBuffer *rb, unsigned char data);
unsigned char RingBuffer_ReadByte(RingBuffer *rb, unsigned char *data);


#ifdef __cplusplus
}
#endif

#endif
