/** @file stm32f4_mapping.c @brief NRF24L01_demo的HAL映射；独立前缀允许组合使用。 */
#include "stm32f4_mapping.h"
static Nrf24l01MappingStatus ConvertStatus(HAL_StatusTypeDef status)
{
    switch (status) {
    case HAL_OK: return NRF24L01_MAPPING_OK;
    case HAL_BUSY: return NRF24L01_MAPPING_BUSY;
    case HAL_TIMEOUT: return NRF24L01_MAPPING_TIMEOUT;
    default: return NRF24L01_MAPPING_ERROR;
    }
}
static Nrf24l01MappingStatus ExchangeSpi(void *opaque, const uint8_t *outgoing, uint8_t *incoming, size_t length)
{
    Nrf24l01Stm32f4SpiContext *context = opaque;
    if (length > UINT16_MAX) return NRF24L01_MAPPING_ERROR;
    return ConvertStatus(HAL_SPI_TransmitReceive(context->handle, (uint8_t *)outgoing, incoming, (uint16_t)length, context->timeout_milliseconds));
}
Nrf24l01MappingSpiBus Nrf24l01Stm32f4_MakeSpiBus(Nrf24l01Stm32f4SpiContext *context) { return (Nrf24l01MappingSpiBus){context, ExchangeSpi}; }
static void WritePin(void *opaque, bool high)
{
    Nrf24l01Stm32f4PinContext *context = opaque;
    HAL_GPIO_WritePin(context->port, context->pin, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
static bool ReadPin(void *opaque)
{
    Nrf24l01Stm32f4PinContext *context = opaque;
    return HAL_GPIO_ReadPin(context->port, context->pin) == GPIO_PIN_SET;
}
Nrf24l01MappingDigitalPin Nrf24l01Stm32f4_MakeDigitalPin(Nrf24l01Stm32f4PinContext *context) { return (Nrf24l01MappingDigitalPin){context, WritePin, ReadPin}; }
static uint32_t Milliseconds(void *context) { (void)context; return HAL_GetTick(); }
static void DelayMilliseconds(void *context, uint32_t duration) { (void)context; HAL_Delay(duration); }
Nrf24l01MappingClock Nrf24l01Stm32f4_MakeClock(void) { return (Nrf24l01MappingClock){NULL, Milliseconds, DelayMilliseconds}; }
