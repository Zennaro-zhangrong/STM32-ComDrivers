/** @file spi_frame_receiver.c @brief 将接收地址申请、字节提交、帧消费分离。 */
#include "spi_frame_receiver.h"
#include <stddef.h>
void SpiFrameReceiver_Initialize(SpiFrameReceiver *receiver)
{
    *receiver = (SpiFrameReceiver){.active_frame = -1};
}
uint8_t *SpiFrameReceiver_NextAddress(SpiFrameReceiver *receiver)
{
    if (receiver == NULL) return NULL;
    int selected = -1;
    for (int index = 0; index < 2; ++index)
        if (receiver->frames[index].state == SPI_FRAME_WRITING) selected = index;
    if (selected < 0) {
        for (int index = 0; index < 2; ++index) {
            if (receiver->frames[index].state == SPI_FRAME_EMPTY) { selected = index; break; }
        }
        if (selected < 0) return NULL;
        receiver->frames[selected].cursor = 0;
        receiver->frames[selected].state = SPI_FRAME_WRITING;
    }
    receiver->active_frame = (int8_t)selected;
    receiver->address_pending = true;
    return &receiver->frames[selected].bytes[receiver->frames[selected].cursor];
}
void SpiFrameReceiver_Abort(SpiFrameReceiver *receiver)
{
    for (unsigned index = 0; index < 2; ++index) {
        if (receiver->frames[index].state == SPI_FRAME_WRITING) {
            receiver->frames[index].state = SPI_FRAME_EMPTY;
            receiver->frames[index].cursor = 0;
        }
    }
    receiver->active_frame = -1;
    receiver->address_pending = false;
}
bool SpiFrameReceiver_CommitByte(SpiFrameReceiver *receiver)
{
    if (!receiver->address_pending || receiver->active_frame < 0 || receiver->active_frame > 1) return false;
    SpiReceiveFrame *frame = &receiver->frames[(unsigned)receiver->active_frame];
    receiver->address_pending = false;
    if (frame->state != SPI_FRAME_WRITING) return false;
    ++frame->cursor;
    if (frame->cursor < 2) return false;
    if (frame->bytes[1] < 3 || frame->bytes[1] > sizeof(frame->bytes)) {
        SpiFrameReceiver_Abort(receiver);
        return false;
    }
    if (frame->cursor == frame->bytes[1]) {
        frame->state = SPI_FRAME_FULL;
        receiver->ready_order[receiver->ready_count++] = (uint8_t)receiver->active_frame;
        receiver->active_frame = -1;
        return true;
    }
    return false;
}
const SpiReceiveFrame *SpiFrameReceiver_Acquire(SpiFrameReceiver *receiver)
{
    if (receiver->ready_count == 0) return NULL;
    uint8_t index = receiver->ready_order[0];
    --receiver->ready_count;
    if (receiver->ready_count != 0) receiver->ready_order[0] = receiver->ready_order[1];
    receiver->frames[index].state = SPI_FRAME_READING;
    return &receiver->frames[index];
}
void SpiFrameReceiver_Release(SpiFrameReceiver *receiver, const SpiReceiveFrame *frame)
{
    for (unsigned index = 0; index < 2; ++index) {
        if (frame == &receiver->frames[index] && frame->state == SPI_FRAME_READING) {
            receiver->frames[index].state = SPI_FRAME_EMPTY;
            receiver->frames[index].cursor = 0;
        }
    }
}
