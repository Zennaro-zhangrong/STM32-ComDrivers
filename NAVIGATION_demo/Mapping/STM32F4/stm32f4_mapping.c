/** @file stm32f4_mapping.c @brief NAVIGATION_demo的HAL映射；独立前缀允许组合使用。 */
#include "stm32f4_mapping.h"
static NavigationMappingStatus ConvertStatus(HAL_StatusTypeDef status)
{
    switch (status) {
    case HAL_OK: return NAVIGATION_MAPPING_OK;
    case HAL_BUSY: return NAVIGATION_MAPPING_BUSY;
    case HAL_TIMEOUT: return NAVIGATION_MAPPING_TIMEOUT;
    default: return NAVIGATION_MAPPING_ERROR;
    }
}
static NavigationMappingStatus WriteUart(void *opaque, const uint8_t *data, size_t length)
{
    NavigationStm32f4UartContext *context = opaque;
    if (length > UINT16_MAX) return NAVIGATION_MAPPING_ERROR;
    return ConvertStatus(HAL_UART_Transmit(context->handle, (uint8_t *)data, (uint16_t)length, context->timeout_milliseconds));
}
NavigationMappingByteWriter NavigationStm32f4_MakeUartWriter(NavigationStm32f4UartContext *context) { return (NavigationMappingByteWriter){context, WriteUart}; }
static NavigationMappingStatus ReadRegister(void *opaque, uint8_t address, uint8_t reg, uint8_t *data, size_t length)
{
    NavigationStm32f4I2cContext *context = opaque;
    if (length > UINT16_MAX || address > 0x7F) return NAVIGATION_MAPPING_ERROR;
    return ConvertStatus(HAL_I2C_Mem_Read(context->handle, (uint16_t)address << 1, reg,
        I2C_MEMADD_SIZE_8BIT, data, (uint16_t)length, context->timeout_milliseconds));
}
static NavigationMappingStatus WriteRegister(void *opaque, uint8_t address, uint8_t reg, const uint8_t *data, size_t length)
{
    NavigationStm32f4I2cContext *context = opaque;
    if (length > UINT16_MAX || address > 0x7F) return NAVIGATION_MAPPING_ERROR;
    return ConvertStatus(HAL_I2C_Mem_Write(context->handle, (uint16_t)address << 1, reg,
        I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, (uint16_t)length, context->timeout_milliseconds));
}
NavigationMappingRegisterBus NavigationStm32f4_MakeRegisterBus(NavigationStm32f4I2cContext *context) { return (NavigationMappingRegisterBus){context, ReadRegister, WriteRegister}; }
static uint32_t Milliseconds(void *context) { (void)context; return HAL_GetTick(); }
static void DelayMilliseconds(void *context, uint32_t duration) { (void)context; HAL_Delay(duration); }
NavigationMappingClock NavigationStm32f4_MakeClock(void) { return (NavigationMappingClock){NULL, Milliseconds, DelayMilliseconds}; }
