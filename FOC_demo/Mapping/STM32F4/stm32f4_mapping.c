/** @file stm32f4_mapping.c @brief FOC_demo的HAL映射；独立前缀允许组合使用。 */
#include "stm32f4_mapping.h"
static FocMappingStatus ConvertStatus(HAL_StatusTypeDef status)
{
    switch (status) {
    case HAL_OK: return FOC_MAPPING_OK;
    case HAL_BUSY: return FOC_MAPPING_BUSY;
    case HAL_TIMEOUT: return FOC_MAPPING_TIMEOUT;
    default: return FOC_MAPPING_ERROR;
    }
}
static void WriteThreePhase(void *opaque, uint16_t phase_a, uint16_t phase_b, uint16_t phase_c)
{
    FocStm32f4ThreePhaseContext *context = opaque;
    __HAL_TIM_SET_COMPARE(context->handle, context->phase_a_channel, phase_a);
    __HAL_TIM_SET_COMPARE(context->handle, context->phase_b_channel, phase_b);
    __HAL_TIM_SET_COMPARE(context->handle, context->phase_c_channel, phase_c);
}
FocMappingThreePhasePwm FocStm32f4_MakeThreePhasePwm(FocStm32f4ThreePhaseContext *context) { return (FocMappingThreePhasePwm){context, WriteThreePhase}; }
FocMappingStatus FocStm32f4_StartThreePhasePwm(FocStm32f4ThreePhaseContext *context)
{
    uint32_t channels[3] = {context->phase_a_channel, context->phase_b_channel, context->phase_c_channel};
    for (unsigned index = 0; index < 3; ++index) {
        HAL_StatusTypeDef status = HAL_TIM_PWM_Start(context->handle, channels[index]);
        if (status != HAL_OK) return ConvertStatus(status);
    }
    for (unsigned index = 0; index < 3; ++index) {
        HAL_StatusTypeDef status = HAL_TIMEx_PWMN_Start(context->handle, channels[index]);
        if (status != HAL_OK) return ConvertStatus(status);
    }
    return FOC_MAPPING_OK;
}
static void WritePin(void *opaque, bool high)
{
    FocStm32f4PinContext *context = opaque;
    HAL_GPIO_WritePin(context->port, context->pin, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
static bool ReadPin(void *opaque)
{
    FocStm32f4PinContext *context = opaque;
    return HAL_GPIO_ReadPin(context->port, context->pin) == GPIO_PIN_SET;
}
FocMappingDigitalPin FocStm32f4_MakeDigitalPin(FocStm32f4PinContext *context) { return (FocMappingDigitalPin){context, WritePin, ReadPin}; }
static uint32_t Milliseconds(void *context) { (void)context; return HAL_GetTick(); }
static void DelayMilliseconds(void *context, uint32_t duration) { (void)context; HAL_Delay(duration); }
FocMappingClock FocStm32f4_MakeClock(void) { return (FocMappingClock){NULL, Milliseconds, DelayMilliseconds}; }
