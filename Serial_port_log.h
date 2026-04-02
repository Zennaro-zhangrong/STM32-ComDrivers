#ifndef SERIAL_PORT_LOG_H_
#define SERIAL_PORT_LOG_H_


#ifdef __cplusplus
extern "C" {
#endif
#include "main.h"
#include "RingBuffer.h"

//这里把设置为打印日志的串口设置正确
#define SERIAL_PORT_NUMBER huart2

//这里是环形缓冲区的长度（Byte）
#define BUFFER_LENGTH 512

void log_printf(const char *fmt, ...);
void log_send_loop(void);


#ifdef __cplusplus
}
#endif


#endif
