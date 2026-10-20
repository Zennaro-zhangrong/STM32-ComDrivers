/** @file stm32f4_mapping.c @brief LOG_demo的HAL映射；独立前缀允许组合使用。 */
#include "stm32f4_mapping.h"
static LogMappingStatus ConvertStatus(HAL_StatusTypeDef status)
{
    switch (status) {
    case HAL_OK: return LOG_MAPPING_OK;
    case HAL_BUSY: return LOG_MAPPING_BUSY;
    case HAL_TIMEOUT: return LOG_MAPPING_TIMEOUT;
    default: return LOG_MAPPING_ERROR;
    }
}
static LogMappingStatus WriteUart(void *opaque, const uint8_t *data, size_t length)
{
    LogStm32f4UartContext *context = opaque;
    if (length > UINT16_MAX) return LOG_MAPPING_ERROR;
    return ConvertStatus(HAL_UART_Transmit(context->handle, (uint8_t *)data, (uint16_t)length, context->timeout_milliseconds));
}
LogMappingByteWriter LogStm32f4_MakeUartWriter(LogStm32f4UartContext *context) { return (LogMappingByteWriter){context, WriteUart}; }
uintptr_t LogStm32f4_EnterCritical(void *context)
{
    (void)context;
    uint32_t previous = __get_PRIMASK();
    __disable_irq();
    __DMB();
    return previous;
}
void LogStm32f4_LeaveCritical(void *context, uintptr_t previous_state)
{
    (void)context;
    __DMB();
    __set_PRIMASK((uint32_t)previous_state);
}
