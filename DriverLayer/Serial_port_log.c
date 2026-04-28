/*
 * Serial_port_log.c
 *
 *  Created on: Mar 27, 2026
 */
#include "Serial_port_log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "stm32f1xx_hal.h"//替换为HAL_GetTick()函数的声明头文件

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
    unsigned char loop_num = strlen(buf);
    for(int i = 0; i < loop_num; i++){
       RingBuffer_WriteByte(&Serial_log, (unsigned char)buf[i]);
    }
}


/*
 * @功能：进行一次缓冲区数据打印，缓冲区清空则退出函数，一般放在主循环中，执行间隔
 * 过久导致缓冲区频繁溢出可以在 Serial_port_log.h 中适量增加缓冲区大小。
 * @参数：无。
 * @返回：执行时长。
 */
unsigned short log_send_loop(void){
	unsigned int time_start = HAL_GetTick();
    unsigned char data = 0;
    while(RingBuffer_ReadByte(&Serial_log, &data) == 1){
        HAL_UART_Transmit(&SERIAL_PORT_NUMBER, &data, 1, 10);//发送函数
    }
	unsigned int time_now = HAL_GetTick();
	return (unsigned short)(time_now - time_start);
}
