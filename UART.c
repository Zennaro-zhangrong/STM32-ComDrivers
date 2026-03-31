/*
 * UART.c
 *
 *  Created on: Mar 27, 2026
 *      Author: 吴梦佳
 */
#include "Hardware.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

extern UART_HandleTypeDef huart2;//使用哪个通道就配置哪个

/**
 * @brief  串口格式化发送函数（支持printf格式）
 * @param  fmt: 格式化字符串
 * @retval 无
 */
void UART_Printf(const char *fmt, ...)
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
    HAL_UART_Transmit(&huart2, (uint8_t *)buf, strlen(buf), HAL_MAX_DELAY);
}
