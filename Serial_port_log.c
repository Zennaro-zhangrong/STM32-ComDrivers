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
*   2.主循环中使用DMA搬运缓冲区的数据给目标串口
*/


volatile unsigned short Log_re_idx = 0;//读指针
volatile unsigned short Log_wr_idx = 0;//写指针
volatile unsigned short Log_count = 0;//缓冲区总字节数

/*
 *功能：将传入参数自动添加到环形缓冲区，如果缓冲区满则取消写入
 *参数：要添加的一字节字符内容
 *返回：无
*/
static void uart_write_buff_Byte(unsigned char data){
    if(Log_count != BUFFER_LENGTH){
        UartTransmit_Buffer[Log_wr_idx] = data;
        Log_count++;
        Log_wr_idx++;
        Log_wr_idx = Log_wr_idx % BUFFER_LENGTH;
    }
 }



/*
*功能：将环形缓冲区的当前一个可读字节地址返回,如果当前缓冲区不可读字节，返回空指针
*参数：无
*返回：数据的地址或空
*/
static unsigned char* uart_read_buff_Byte(void){
    unsigned char* Data = NULL;
    if(Log_count == 0){
       return NULL;
    }
    Data = &UartTransmit_Buffer[Log_re_idx]; 
    Log_count--;
    Log_re_idx++;
    Log_re_idx = Log_re_idx % BUFFER_LENGTH;
    return Data;
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
        uart_write_buff_Byte((unsigned char)buf[i]);
    }
}
