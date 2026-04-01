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

volatile unsigned short Log_wr = 0;//写指针
volatile unsigned short Log_re = 0;//读指针


/*
 *功能：将传入参数自动添加到环形缓冲区，如果缓冲区满则添加失败
 *参数：要添加的一字节字符内容
 *返回：无意义
*/
static char uart_write_buff_Byte(char data){
    if(Log_wr == Log_re){
        return 0;
    }
    UartTransmit_Buffer[Log_wr] = data;
    Log_wr++;
    Log_wr = Log_wr % BUFFER_LENGTH;
    return 0;
}



/*
*功能：将环形缓冲区的当前可读字节传给输入地址的内存
*/
static char uart_read_buff_Byte(unsigned char * Pdata){
    if(Log_re = Log_wr){
        return 0;
    }
    (*Pdata) = UartTransmit_Buffer[Log_re];
    Log_re++;
    Log_re = Log_re % BUFFER_LENGTH;
    return 0;
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

    // 通过USART2阻塞式发送（UTF-8编码，串口助手直接显示）
    HAL_UART_Transmit(&SERIAL_PORT_NUMBER, (unsigned char *)buf, strlen(buf), HAL_MAX_DELAY);
}
