#ifndef SERIAL_PORT_LOG_H_
#define SERIAL_PORT_LOG_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "../Middleware/Buffer_scheduling/RingBuffer.h"

typedef enum {
    SERIAL_LOG_SEND_BLOCK,    // 阻塞发送
    SERIAL_LOG_SEND_DMA      // DMA发送
} SerialLog_SendMode;


typedef enum {
    SERIAL_LOG_ACCUMULATE,  //数据积攒状态
    SERIAL_LOG_AVALANCHE   //雪崩发送状态
} SerialLog_Sendstate;

typedef struct{
    UART_HandleTypeDef *Serial_port_huart;//串口编号
    SerialLog_SendMode Serial_SendMode;//发送模式
    volatile SerialLog_Sendstate Serial_Sendstate;//发送状态
    unsigned char *Pbuff;   //环形缓冲区实际地址
    RingBuffer Ringbuffer;//环形缓冲区
    unsigned short RingBuffer_Length;//环形缓冲区长度
}Serial_log_HandleTypedef;

void Log_Init(UART_HandleTypeDef *huart, SerialLog_SendMode SendMode);
void log_printf(const char *fmt, ...);
unsigned short log_send_loop(void);
void Serial_Log_DMA_TxCpltCallback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif


#endif
