/** @file stm32f4_mapping.h
 * @brief LOG_demo专用STM32F4适配；仅包含本demo实际需要的外设。
 * @note 不定义HAL总回调，不初始化时钟/引脚；句柄和上下文由调用方持有。
 */
#ifndef COMDRIVERS_LOG_STM32F4_MAPPING_H
#define COMDRIVERS_LOG_STM32F4_MAPPING_H
#include "stm32f4xx_hal.h"
#include "../mapping.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { UART_HandleTypeDef *handle; uint32_t timeout_milliseconds; } LogStm32f4UartContext;
/** @brief 返回同步 UART 输出接口；每次最多65535字节。 */
LogMappingByteWriter LogStm32f4_MakeUartWriter(LogStm32f4UartContext *context);
/** @brief 保存PRIMASK并关中断，用作环形缓冲enter；支持嵌套，仅适合短临界区。 */
uintptr_t LogStm32f4_EnterCritical(void *context);
/** @brief 恢复先前PRIMASK，用作leave；不得无条件开启中断。 */
void LogStm32f4_LeaveCritical(void *context, uintptr_t previous_state);
#ifdef __cplusplus
}
#endif
#endif
