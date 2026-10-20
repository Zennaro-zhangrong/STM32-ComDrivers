/** @file log_service.h
 * @brief 格式化日志、整条入队和发送失败重试。
 * @note Print可多生产者调用，但须配置buffer临界区；Process仅允许一个消费者。
 *       格式化不在临界区内执行；UART发送也不持有临界区。
 */
#ifndef COMDRIVERS_LOG_SERVICE_H
#define COMDRIVERS_LOG_SERVICE_H
#include "../Middleware/ring_buffer.h"
#include "../Mapping/mapping.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { LOG_PRINT_OK, LOG_PRINT_TOO_LONG, LOG_PRINT_QUEUE_FULL, LOG_PRINT_INVALID } LogPrintResult;
typedef struct {
    RingBuffer buffer;
    LogMappingByteWriter writer;
    uint32_t rejected_messages; /**< 过长、无效或队满时整条拒绝；并发读取需同一临界区。 */
    uint32_t send_failures;
    uint8_t pending_byte;
    bool pending_valid;
} LogService;
void LogService_Initialize(LogService *service, uint8_t *storage, uint16_t capacity, LogMappingByteWriter writer);
/** @brief 最多255字节，过长/队满整条拒绝并返回原因，不再静默截断。 */
LogPrintResult LogService_Print(LogService *service, const char *format, ...);
/** @brief 返回成功发送的字节数；失败字节留待下次重试，每次最多处理capacity+1字节。
 * @note writer非OK必须表示该字节未发送；不确定是否已发送的底层需自行解决重复交付语义。
 */
size_t LogService_Process(LogService *service);
#ifdef __cplusplus
}
#endif
#endif
