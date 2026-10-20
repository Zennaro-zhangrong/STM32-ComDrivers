/** @file spi_frame_receiver.h
 * @brief command,length,payload双缓冲接收；length为包含头部的总长度，范围3～36。
 * @note ISR与任务访问同一对象须由调用方串行化；没有帧头标记，片选结束/接收超时必须Abort。
 */
#ifndef COMDRIVERS_SPI_FRAME_RECEIVER_H
#define COMDRIVERS_SPI_FRAME_RECEIVER_H
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { SPI_FRAME_EMPTY, SPI_FRAME_FULL, SPI_FRAME_READING, SPI_FRAME_WRITING } SpiFrameState;
typedef struct { uint8_t bytes[36], cursor; SpiFrameState state; } SpiReceiveFrame;
typedef struct {
    SpiReceiveFrame frames[2];
    int8_t active_frame;
    bool address_pending;
    uint8_t ready_order[2], ready_count; /**< 按完成顺序获取，避免低编号缓冲抢先。 */
} SpiFrameReceiver;
/** @brief 清空状态；接收前调用，不能在DMA访问缓冲期间重置。 */
void SpiFrameReceiver_Initialize(SpiFrameReceiver *receiver);
/** @brief 申请一个字节写地址；尚未完成时重复调用返回同一地址，不提前发布FULL。 */
uint8_t *SpiFrameReceiver_NextAddress(SpiFrameReceiver *receiver);
/** @brief 仅在该字节实际接收完成后调用；返回true表示刚完成整帧。 */
bool SpiFrameReceiver_CommitByte(SpiFrameReceiver *receiver);
/** @brief 放弃当前半帧；调用方须先停止接收DMA，不影响已完成帧。 */
void SpiFrameReceiver_Abort(SpiFrameReceiver *receiver);
/** @brief 获取已完成帧并标为READING；使用结束必须Release，返回NULL表示无帧。 */
const SpiReceiveFrame *SpiFrameReceiver_Acquire(SpiFrameReceiver *receiver);
void SpiFrameReceiver_Release(SpiFrameReceiver *receiver, const SpiReceiveFrame *frame);
#ifdef __cplusplus
}
#endif
#endif
