/** @file stm32f4_mapping.h
 * @brief FOC_demo专用STM32F4适配；仅包含本demo实际需要的外设。
 * @note 不定义HAL总回调，不初始化时钟/引脚；句柄和上下文由调用方持有。
 */
#ifndef COMDRIVERS_FOC_STM32F4_MAPPING_H
#define COMDRIVERS_FOC_STM32F4_MAPPING_H
#include "stm32f4xx_hal.h"
#include "../mapping.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    TIM_HandleTypeDef *handle; /**< 已初始化的高级定时器，启动互补输出时须支持相应通道。 */
    uint32_t phase_a_channel, phase_b_channel, phase_c_channel; /**< 三相对应的TIM_CHANNEL_x宏。 */
} FocStm32f4ThreePhaseContext;
typedef struct { GPIO_TypeDef *port; uint16_t pin; } FocStm32f4PinContext;
/** @brief 只更新比较寄存器，不自动启动输出或配置死区。 */
FocMappingThreePhasePwm FocStm32f4_MakeThreePhasePwm(FocStm32f4ThreePhaseContext *context);
/** @brief 启动三相主通道和互补通道；仅在板级初始化完成后显式调用。 */
FocMappingStatus FocStm32f4_StartThreePhasePwm(FocStm32f4ThreePhaseContext *context);
/** @brief 返回物理高低电平GPIO接口，不隐含有效极性。 */
FocMappingDigitalPin FocStm32f4_MakeDigitalPin(FocStm32f4PinContext *context);
/** @brief HAL_GetTick/HAL_Delay 包装；阻塞延时的使用上下文由集成方决定。 */
FocMappingClock FocStm32f4_MakeClock(void);
#ifdef __cplusplus
}
#endif
#endif
