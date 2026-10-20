/** @file stm32f4_mapping.h
 * @brief DISPLAY_demo专用STM32F4适配；仅包含本demo实际需要的外设。
 * @note 不定义HAL总回调，不初始化时钟/引脚；句柄和上下文由调用方持有。
 */
#ifndef COMDRIVERS_DISPLAY_STM32F4_MAPPING_H
#define COMDRIVERS_DISPLAY_STM32F4_MAPPING_H
#include "stm32f4xx_hal.h"
#include "../mapping.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { SPI_HandleTypeDef *handle; uint32_t timeout_milliseconds; } DisplayStm32f4SpiContext;
typedef struct { I2C_HandleTypeDef *handle; uint32_t timeout_milliseconds; } DisplayStm32f4I2cContext;
typedef struct { GPIO_TypeDef *port; uint16_t pin; } DisplayStm32f4PinContext;
/** @brief 返回同步 SPI 发送接口，供屏幕命令与参数使用。 */
DisplayMappingByteWriter DisplayStm32f4_MakeSpiWriter(DisplayStm32f4SpiContext *context);
/** @brief 返回8位寄存器I2C接口，内部将7位设备地址左移一次。 */
DisplayMappingRegisterBus DisplayStm32f4_MakeRegisterBus(DisplayStm32f4I2cContext *context);
/** @brief 返回物理高低电平GPIO接口，不隐含有效极性。 */
DisplayMappingDigitalPin DisplayStm32f4_MakeDigitalPin(DisplayStm32f4PinContext *context);
/** @brief HAL_GetTick/HAL_Delay 包装；阻塞延时的使用上下文由集成方决定。 */
DisplayMappingClock DisplayStm32f4_MakeClock(void);
/** @brief SPI DMA发送，适配 Ili9341.start_pixels；完成/错误由板级HAL回调转交。 */
DisplayMappingStatus DisplayStm32f4_StartSpiPixels(void *context, const uint8_t *pixels, uint16_t bytes);
#ifdef __cplusplus
}
#endif
#endif
