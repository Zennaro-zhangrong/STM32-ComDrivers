/** @file stm32f4_mapping.c
 * @brief WS2812_demo的HAL映射；独立前缀允许组合使用。
 **/
#include "stm32f4_mapping.h"
static Ws2812MappingStatus ConvertStatus(HAL_StatusTypeDef status)
{
    switch (status) {
    case HAL_OK: return WS2812_MAPPING_OK;
    case HAL_BUSY: return WS2812_MAPPING_BUSY;
    case HAL_TIMEOUT: return WS2812_MAPPING_TIMEOUT;
    default: return WS2812_MAPPING_ERROR;
    }
}
Ws2812MappingStatus Ws2812Stm32f4_StartPwmSamples(Ws2812Stm32f4PwmContext *context, uint16_t *samples, uint16_t count)
{
    return ConvertStatus(HAL_TIM_PWM_Start_DMA(context->handle, context->channel, (uint32_t *)samples, count));
}
