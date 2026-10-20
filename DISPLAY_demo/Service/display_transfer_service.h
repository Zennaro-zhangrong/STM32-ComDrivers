/** @file display_transfer_service.h
 * @brief 双描述符发送队列；失败和完成都归还对应owner的缓冲所有权。
 * @note 所有入口由同一任务串行调用；DMA中断只投递事件。start只能启动异步传输，
 *       不能在返回前调用OnComplete/OnError；错误通知前必须确认DMA停止访问缓冲。
 */
#ifndef COMDRIVERS_DISPLAY_TRANSFER_SERVICE_H
#define COMDRIVERS_DISPLAY_TRANSFER_SERVICE_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    uint16_t left, top, right, bottom;
    const uint8_t *pixels;
    size_t byte_count;
    void *owner;
} DisplayTransfer;
typedef struct {
    DisplayTransfer pending[2];
    uint8_t count;
    bool active, dispatching;
    void *context;
    bool (*start)(void *context, const DisplayTransfer *transfer); /**< true为已启动，false为未启动且不持有缓冲。 */
    void (*complete)(void *owner, bool success); /**< 对每个已接受请求恰好通知一次。 */
} DisplayTransferService;
/** @brief 清零对象并设置回调；返回true表示请求已接受，false表示满队列/无效参数且不持有缓冲。 */
bool DisplayTransfer_Enqueue(DisplayTransferService *service, void *owner,
    uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, const uint8_t *pixels);
/** @brief 启动尚未运行的请求，允许主循环周期调用。 */
void DisplayTransfer_Process(DisplayTransferService *service);
void DisplayTransfer_OnComplete(DisplayTransferService *service);
void DisplayTransfer_OnError(DisplayTransferService *service);
#ifdef __cplusplus
}
#endif
#endif
