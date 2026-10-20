/** @file log_service.c @brief 生产者批量入队，单消费者逐字节确认发送。 */
#include "log_service.h"
#include <stdarg.h>
#include <stdio.h>
void LogService_Initialize(LogService *service, uint8_t *storage, uint16_t capacity, LogMappingByteWriter writer)
{
    *service = (LogService){.writer=writer};
    RingBuffer_Initialize(&service->buffer, storage, capacity);
}
LogPrintResult LogService_Print(LogService *service, const char *format, ...)
{
    if (service == NULL) return LOG_PRINT_INVALID;
    char text[256];
    int length = -1;
    if (format != NULL) {
        va_list arguments;
        va_start(arguments, format);
        length = vsnprintf(text, sizeof(text), format, arguments);
        va_end(arguments);
    }
    uintptr_t previous = RingBuffer_Enter(&service->buffer);
    LogPrintResult result = LOG_PRINT_OK;
    if (length < 0) result = LOG_PRINT_INVALID;
    else if ((size_t)length >= sizeof(text)) result = LOG_PRINT_TOO_LONG;
    else if (RingBuffer_Write(&service->buffer, (const uint8_t *)text, (size_t)length) != (size_t)length)
        result = LOG_PRINT_QUEUE_FULL;
    if (result != LOG_PRINT_OK) ++service->rejected_messages;
    RingBuffer_Leave(&service->buffer, previous);
    return result;
}
size_t LogService_Process(LogService *service)
{
    if (service == NULL || service->writer.write == NULL) return 0;
    size_t count = 0, limit = (size_t)service->buffer.capacity + 1;
    while (count < limit) {
        if (!service->pending_valid) {
            if (!RingBuffer_ReadByte(&service->buffer, &service->pending_byte)) break;
            service->pending_valid = true;
        }
        if (service->writer.write(service->writer.context, &service->pending_byte, 1) != LOG_MAPPING_OK) {
            ++service->send_failures;
            break;
        }
        service->pending_valid = false;
        ++count;
    }
    return count;
}
