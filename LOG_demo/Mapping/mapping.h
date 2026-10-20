/** @file mapping.h
 * @brief LOG_demo的硬件接口，仅声明本功能需要的操作。
 * @note 回调及context由集成工程提供；同步函数返回前完成访问。
 *       本接口为同步输出，不接管缓冲区；只允许映射实现解释HAL句柄。
 */
#ifndef COMDRIVERS_LOG_MAPPING_H
#define COMDRIVERS_LOG_MAPPING_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    LOG_MAPPING_OK = 0,             /**< 本次操作完成。 */
    LOG_MAPPING_ERROR,              /**< 底层操作失败。 */
    LOG_MAPPING_BUSY,               /**< 资源正在使用。 */
    LOG_MAPPING_TIMEOUT            /**< 超过调用方指定的等待时间。 */
} LogMappingStatus;

typedef struct {
    void *context; /**< 调用方持有的输出上下文，服务使用期间必须有效。 */
    /** @brief 同步发送length个字节；返回前结束对data的访问，失败返回对应状态。 */
    LogMappingStatus (*write)(void *context, const uint8_t *data, size_t length);
} LogMappingByteWriter;
#ifdef __cplusplus
}
#endif
#endif
