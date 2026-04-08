/*
 * @file: RingBuffer.c
 * @作者：张荣
 * @data: 2026-04-02
 * @brief: 实现环形缓冲区的函数，目的是增加可移植性，非底层文件
 */


#include "RingBuffer.h"

/*
 * @功能：初始化申请的缓冲区
 * @参数：缓冲区实际数组的地址
 * @参数：缓冲区的大小（单位：字节）
 * @返回：无
 */
void RingBuffer_Init(RingBuffer *rb, unsigned char *buf, unsigned short size) {
    rb->buffer = buf;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

/*
 * @功能：从指定的环形缓冲区写入一个字节
 * @参数：指定的环形缓冲区的句柄地址
 * @参数：要写入的数据
 * @返回: 写入是否成功，失败返回0，成功返回1
 */
unsigned char RingBuffer_WriteByte(RingBuffer *rb, unsigned char data) {
    if (rb->count == rb->size) {
        return 0;
    } else {
        rb->buffer[rb->head] = data;
        rb->head = (rb->head + 1) % rb->size; 
        rb->count = rb->count + 1;
        return 1;
    }
}




/*
 *@功能：从指定的环形缓冲区读取一个字节的数据
 *@参数：指定的环形缓冲区的句柄地址
 *@参数：接收该数据的地址
 *@返回：失败：0，成功：1
 */
unsigned char RingBuffer_ReadByte(RingBuffer *rb, unsigned char *data) {
    if (rb->head == rb->tail && rb->count == 0) {
         return 0;
    } else {
        *data = rb->buffer[rb->tail];
        rb->tail = (rb->tail + 1) % rb->size;
        rb->count = rb->count - 1;
        return 1;
    }
}
