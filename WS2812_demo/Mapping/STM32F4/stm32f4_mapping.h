/** @file stm32f4_mapping.h
 * @brief WS2812_demo专用STM32F4适配；仅包含本demo实际需要的外设。
 * @note 不定义HAL总回调，不初始化时钟/引脚；句柄和上下文由调用方持有。
 */
#ifndef COMDRIVERS_WS2812_STM32F4_MAPPING_H
#define COMDRIVERS_WS2812_STM32F4_MAPPING_H
#include "stm32f4xx_hal.h"
#include "../mapping.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { TIM_HandleTypeDef *handle; uint32_t channel; } Ws2812Stm32f4PwmContext;
/** @brief 定时器DMA发送灯带序列；samples需位于DMA可访问RAM且存活至完成。 */
Ws2812MappingStatus Ws2812Stm32f4_StartPwmSamples(Ws2812Stm32f4PwmContext *context, uint16_t *samples, uint16_t count);
#ifdef __cplusplus
}
#endif
#endif
