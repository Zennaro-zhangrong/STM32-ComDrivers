/** @file mapping.h
 * @brief WS2812_demo的硬件接口，仅声明本功能需要的操作。
 * @note 回调及context由集成工程提供；同步函数返回前完成访问。
 *       DMA缓冲保持至完成通知；只允许映射实现解释HAL句柄。
 */
#ifndef COMDRIVERS_WS2812_MAPPING_H
#define COMDRIVERS_WS2812_MAPPING_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    WS2812_MAPPING_OK = 0,             /**< 本次操作完成。 */
    WS2812_MAPPING_ERROR,              /**< 底层操作失败。 */
    WS2812_MAPPING_BUSY,               /**< 资源正在使用。 */
    WS2812_MAPPING_TIMEOUT            /**< 超过调用方指定的等待时间。 */
} Ws2812MappingStatus;
#ifdef __cplusplus
}
#endif
#endif
