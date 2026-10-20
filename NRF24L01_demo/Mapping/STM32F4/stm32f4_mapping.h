/** @file stm32f4_mapping.h
 * @brief NRF24L01_demo专用STM32F4适配；仅包含本demo实际需要的外设。
 * @note 不定义HAL总回调，不初始化时钟/引脚；句柄和上下文由调用方持有。
 */
#ifndef COMDRIVERS_NRF24L01_STM32F4_MAPPING_H
#define COMDRIVERS_NRF24L01_STM32F4_MAPPING_H
#include "stm32f4xx_hal.h"
#include "../mapping.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { SPI_HandleTypeDef *handle; uint32_t timeout_milliseconds; } Nrf24l01Stm32f4SpiContext;
typedef struct { GPIO_TypeDef *port; uint16_t pin; } Nrf24l01Stm32f4PinContext;
/** @brief 返回同步 SPI 全双工接口，收发数组不可为空。 */
Nrf24l01MappingSpiBus Nrf24l01Stm32f4_MakeSpiBus(Nrf24l01Stm32f4SpiContext *context);
/** @brief 返回物理高低电平GPIO接口，不隐含有效极性。 */
Nrf24l01MappingDigitalPin Nrf24l01Stm32f4_MakeDigitalPin(Nrf24l01Stm32f4PinContext *context);
/** @brief HAL_GetTick/HAL_Delay 包装；阻塞延时的使用上下文由集成方决定。 */
Nrf24l01MappingClock Nrf24l01Stm32f4_MakeClock(void);
#ifdef __cplusplus
}
#endif
#endif
