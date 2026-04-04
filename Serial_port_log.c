/*
 * Serial_port_log.c
 *
 *  Created on: Mar 27, 2026
 */
#include "Serial_port_log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

extern UART_HandleTypeDef SERIAL_PORT_NUMBER;
static unsigned char UartTransmit_Buffer[BUFFER_LENGTH] = {0};


/*
*该日志实现方法为：
*   1.将要打印的数据放在环形缓冲区，
*   2.主循环中使用uart阻塞式发送
*/

RingBuffer Serial_log;//实例化环形缓冲区







/*
 * @功能：初始化日志打印。
 * @参数：无
 * @返回：无
 */
void Log_Init(void){
	RingBuffer_Init(&Serial_log, UartTransmit_Buffer, BUFFER_LENGTH);
}


/**
 * @brief  串口格式化发送函数（支持printf格式）
 * @param  fmt: 格式化字符串
 * @retval 无
 */
void log_printf(const char *fmt, ...)
{
    char buf[256];  // 发送缓冲区，可根据需求调整大小
    va_list args;

    // 初始化可变参数列表
    va_start(args, fmt);
    // 格式化字符串到缓冲区
    vsnprintf(buf, sizeof(buf), fmt, args);
    // 结束可变参数处理
    va_end(args);

    //写入环形缓冲区
    unsign char loop_num = strlen(buf);
    for(int i = 0; i < loop_num; i++){
       RingBuffer_WriteByte(Serial_log, (unsigned char)buf[i]);
    }
}

void log_send_loop(void){
    unsigned char data = 0;
    if(RingBuffer_ReadByte(Serial_log, &data) == 1){
        //DMA发送库函数
    }
}
