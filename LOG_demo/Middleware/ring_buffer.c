/** @file ring_buffer.c @brief 在同一临界区内更新缓冲内容、索引及计数。 */
#include "ring_buffer.h"
void RingBuffer_Initialize(RingBuffer *buffer, uint8_t *storage, uint16_t capacity)
{
    *buffer = (RingBuffer){.storage=storage, .capacity=storage != NULL ? capacity : 0};
}
uint8_t RingBuffer_SetCriticalSection(RingBuffer *buffer, RingBufferCriticalSection critical)
{
    if ((critical.enter == NULL) != (critical.leave == NULL)) return 0;
    buffer->critical = critical;
    return 1;
}
uintptr_t RingBuffer_Enter(RingBuffer *buffer)
{
    return buffer->critical.enter != NULL ? buffer->critical.enter(buffer->critical.context) : 0;
}
void RingBuffer_Leave(RingBuffer *buffer, uintptr_t previous_state)
{
    if (buffer->critical.leave != NULL) buffer->critical.leave(buffer->critical.context, previous_state);
}
size_t RingBuffer_Write(RingBuffer *buffer, const uint8_t *data, size_t length)
{
    if (buffer == NULL || (data == NULL && length != 0)) return 0;
    uintptr_t previous = RingBuffer_Enter(buffer);
    if (length > (size_t)(buffer->capacity - buffer->count)) { RingBuffer_Leave(buffer, previous); return 0; }
    for (size_t index = 0; index < length; ++index) {
        buffer->storage[buffer->write_index] = data[index];
        if (++buffer->write_index == buffer->capacity) buffer->write_index = 0;
    }
    buffer->count += (uint16_t)length;
    RingBuffer_Leave(buffer, previous);
    return length;
}
uint8_t RingBuffer_WriteByte(RingBuffer *buffer, uint8_t value)
{
    return (uint8_t)RingBuffer_Write(buffer, &value, 1);
}
uint8_t RingBuffer_ReadByte(RingBuffer *buffer, uint8_t *value)
{
    if (buffer == NULL || value == NULL) return 0;
    uintptr_t previous = RingBuffer_Enter(buffer);
    if (buffer->count == 0) { RingBuffer_Leave(buffer, previous); return 0; }
    *value = buffer->storage[buffer->read_index];
    if (++buffer->read_index == buffer->capacity) buffer->read_index = 0;
    --buffer->count;
    RingBuffer_Leave(buffer, previous);
    return 1;
}
