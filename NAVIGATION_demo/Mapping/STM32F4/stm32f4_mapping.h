/** @file stm32f4_mapping.h
 * @brief NAVIGATION_demo专用STM32F4适配；仅包含本demo实际需要的外设。
 * @note 不定义HAL总回调，不初始化时钟/引脚；句柄和上下文由调用方持有。
 */
#ifndef COMDRIVERS_NAVIGATION_STM32F4_MAPPING_H
#define COMDRIVERS_NAVIGATION_STM32F4_MAPPING_H
#include "stm32f4xx_hal.h"
#include "../mapping.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { UART_HandleTypeDef *handle; uint32_t timeout_milliseconds; } NavigationStm32f4UartContext;
typedef struct { I2C_HandleTypeDef *handle; uint32_t timeout_milliseconds; } NavigationStm32f4I2cContext;
/** @brief 返回同步 UART 输出接口；每次最多65535字节。 */
NavigationMappingByteWriter NavigationStm32f4_MakeUartWriter(NavigationStm32f4UartContext *context);
/** @brief 返回8位寄存器I2C接口，内部将7位设备地址左移一次。 */
NavigationMappingRegisterBus NavigationStm32f4_MakeRegisterBus(NavigationStm32f4I2cContext *context);
/** @brief HAL_GetTick/HAL_Delay 包装；阻塞延时的使用上下文由集成方决定。 */
NavigationMappingClock NavigationStm32f4_MakeClock(void);
#ifdef __cplusplus
}
#endif
#endif
