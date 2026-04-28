/*
 * Serial_port_log.c
 *
 *  Created on: Mar 27, 2026
 * @作者：张荣
 */
#include "Serial_port_log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "stm32f1xx_hal.h"//替换为HAL_GetTick()函数的声明头文件
#include "main.h"

//宏定义数组长度
#define BuffLength 512

Serial_log_HandleTypedef Log = {0};
static unsigned char UartTransmit_Buffer[BuffLength] = {0};



static void Serial_Log_AvalancheSend_Start(void);
/*
 * @功能：初始化日志打印。指定串口号，设置缓冲区长度，运行模式。
 * @参数：串口句柄
 * @参数：设置环形缓冲区字节长度
 * @参数：设置运行模式，SERIAL_LOG_SEND_DMA和SERIAL_LOG_SEND_BLOCK两个选项
 * @返回：无
 */
void Log_Init(UART_HandleTypeDef *huart, SerialLog_SendMode SendMode){
    if(huart == NULL || BuffLength == 0) return; 

    Log.Serial_port_huart = huart;
    Log.RingBuffer_Length = BuffLength;
    Log.Pbuff = UartTransmit_Buffer;
	RingBuffer_Init(&Log.Ringbuffer, UartTransmit_Buffer, Log.RingBuffer_Length);
    Log.Serial_SendMode = SendMode;
    Log.Serial_Sendstate = SERIAL_LOG_ACCUMULATE;//没有任何数据，设为积攒模式
}


/**
 * @brief  串口格式化发送函数
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
    unsigned short loop_num = strlen(buf);
    for(int i = 0; i < loop_num; i++){
       RingBuffer_WriteByte(&Log.Ringbuffer, (unsigned char)buf[i]);
    }
    if(Log.Serial_SendMode == SERIAL_LOG_SEND_DMA){
        Serial_Log_AvalancheSend_Start();//申请雪崩发送
    }
}


/*
 * @功能：进行一次缓冲区数据打印，缓冲区清空则退出函数，放在主循环中，执行间隔
 * 过久导致缓冲区频繁溢出可以在 Serial_port_log.h 中适量增加缓冲区大小。
 * @参数：无。
 * @返回：执行时长。
 */
unsigned short log_send_loop(void){
    if(Log.Serial_SendMode == SERIAL_LOG_SEND_DMA) return 0;
	unsigned int time_start = HAL_GetTick();
    unsigned char data = 0;
    while(RingBuffer_ReadByte(&Log.Ringbuffer, &data) == 1){
        HAL_UART_Transmit(Log.Serial_port_huart, &data, 1, 10);//发送函数
    }
	unsigned int time_now = HAL_GetTick();
	return (unsigned short)(time_now - time_start);
}


/**
 * @brief  串口日志 DMA 发送完成回调函数
 * @param  huart: 串口句柄指针 (HAL库标准类型)
 * @retval 无
 */
void Serial_Log_DMA_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == Log.Serial_port_huart){
        unsigned char data = 0;
        if(RingBuffer_ReadByte(&Log.Ringbuffer, &data) == 1){
            HAL_UART_Transmit_DMA(Log.Serial_port_huart, &data, 1);//发送函数
        }else{
            Log.Serial_Sendstate = SERIAL_LOG_ACCUMULATE;//缓冲区发送空了之后切换为积攒状态
        }
    }
}


/**
 * @功能：在积攒模式下主动启动雪崩状态，仅能放在log_printf()函数末尾
 */
static void Serial_Log_AvalancheSend_Start(void){
    if(Log.Serial_Sendstate == SERIAL_LOG_AVALANCHE) return; 
    else if(Log.Ringbuffer.count == 0) return; 

    Log.Serial_Sendstate = SERIAL_LOG_AVALANCHE;
    unsigned char data = 0;
    RingBuffer_ReadByte(&Log.Ringbuffer, &data);
    HAL_UART_Transmit_DMA(Log.Serial_port_huart, &data, 1);
    
}
