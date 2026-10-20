/** @file stm32f4_mapping.c @brief DISPLAY_demo的HAL映射；独立前缀允许组合使用。 */
#include "stm32f4_mapping.h"
static DisplayMappingStatus ConvertStatus(HAL_StatusTypeDef status)
{
    switch (status) {
    case HAL_OK: return DISPLAY_MAPPING_OK;
    case HAL_BUSY: return DISPLAY_MAPPING_BUSY;
    case HAL_TIMEOUT: return DISPLAY_MAPPING_TIMEOUT;
    default: return DISPLAY_MAPPING_ERROR;
    }
}
static DisplayMappingStatus WriteSpi(void *opaque, const uint8_t *data, size_t length)
{
    DisplayStm32f4SpiContext *context = opaque;
    if (length > UINT16_MAX) return DISPLAY_MAPPING_ERROR;
    return ConvertStatus(HAL_SPI_Transmit(context->handle, (uint8_t *)data, (uint16_t)length, context->timeout_milliseconds));
}
DisplayMappingByteWriter DisplayStm32f4_MakeSpiWriter(DisplayStm32f4SpiContext *context) { return (DisplayMappingByteWriter){context, WriteSpi}; }
static DisplayMappingStatus ReadRegister(void *opaque, uint8_t address, uint8_t reg, uint8_t *data, size_t length)
{
    DisplayStm32f4I2cContext *context = opaque;
    if (length > UINT16_MAX || address > 0x7F) return DISPLAY_MAPPING_ERROR;
    return ConvertStatus(HAL_I2C_Mem_Read(context->handle, (uint16_t)address << 1, reg,
        I2C_MEMADD_SIZE_8BIT, data, (uint16_t)length, context->timeout_milliseconds));
}
static DisplayMappingStatus WriteRegister(void *opaque, uint8_t address, uint8_t reg, const uint8_t *data, size_t length)
{
    DisplayStm32f4I2cContext *context = opaque;
    if (length > UINT16_MAX || address > 0x7F) return DISPLAY_MAPPING_ERROR;
    return ConvertStatus(HAL_I2C_Mem_Write(context->handle, (uint16_t)address << 1, reg,
        I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, (uint16_t)length, context->timeout_milliseconds));
}
DisplayMappingRegisterBus DisplayStm32f4_MakeRegisterBus(DisplayStm32f4I2cContext *context) { return (DisplayMappingRegisterBus){context, ReadRegister, WriteRegister}; }
static void WritePin(void *opaque, bool high)
{
    DisplayStm32f4PinContext *context = opaque;
    HAL_GPIO_WritePin(context->port, context->pin, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
static bool ReadPin(void *opaque)
{
    DisplayStm32f4PinContext *context = opaque;
    return HAL_GPIO_ReadPin(context->port, context->pin) == GPIO_PIN_SET;
}
DisplayMappingDigitalPin DisplayStm32f4_MakeDigitalPin(DisplayStm32f4PinContext *context) { return (DisplayMappingDigitalPin){context, WritePin, ReadPin}; }
static uint32_t Milliseconds(void *context) { (void)context; return HAL_GetTick(); }
static void DelayMilliseconds(void *context, uint32_t duration) { (void)context; HAL_Delay(duration); }
DisplayMappingClock DisplayStm32f4_MakeClock(void) { return (DisplayMappingClock){NULL, Milliseconds, DelayMilliseconds}; }
DisplayMappingStatus DisplayStm32f4_StartSpiPixels(void *opaque, const uint8_t *pixels, uint16_t bytes)
{
    DisplayStm32f4SpiContext *context = opaque;
    return ConvertStatus(HAL_SPI_Transmit_DMA(context->handle, (uint8_t *)pixels, bytes));
}
