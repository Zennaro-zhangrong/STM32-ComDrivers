/** @file ring_buffer.h
 * @brief 外部存储的字节环形缓冲；支持可注入临界区。
 * @note 默认仅限单执行上下文。跨任务/中断必须设置成对enter/leave，禁止阻塞式ISR锁。
 *       enter返回进入前状态，leave恢复该状态；回调须支持嵌套，不能盲目开中断。
 */
#ifndef COMDRIVERS_RING_BUFFER_H
#define COMDRIVERS_RING_BUFFER_H
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    void *context;
    uintptr_t (*enter)(void *context);
    void (*leave)(void *context, uintptr_t previous_state);
} RingBufferCriticalSection;
typedef struct {
    uint8_t *storage;
    uint16_t capacity, write_index, read_index, count;
    RingBufferCriticalSection critical;
} RingBuffer;
void RingBuffer_Initialize(RingBuffer *buffer, uint8_t *storage, uint16_t capacity);
/** @brief 启用并发使用前设置；传入不成对回调返回0且不修改原配置。 */
uint8_t RingBuffer_SetCriticalSection(RingBuffer *buffer, RingBufferCriticalSection critical);
uintptr_t RingBuffer_Enter(RingBuffer *buffer);
void RingBuffer_Leave(RingBuffer *buffer, uintptr_t previous_state);
/** @brief 整批写入，空间不足则不写入任何字节；返回实际写入数。 */
size_t RingBuffer_Write(RingBuffer *buffer, const uint8_t *data, size_t length);
uint8_t RingBuffer_WriteByte(RingBuffer *buffer, uint8_t value);
uint8_t RingBuffer_ReadByte(RingBuffer *buffer, uint8_t *value);
#ifdef __cplusplus
}
#endif
#endif
