/** @file mapping.h
 * @brief NAVIGATION_demo的硬件接口，仅声明本功能需要的操作。
 * @note 回调及context由集成工程提供；同步函数返回前完成访问。
 *       DMA缓冲保持至完成通知；只允许映射实现解释HAL句柄。
 */
#ifndef COMDRIVERS_NAVIGATION_MAPPING_H
#define COMDRIVERS_NAVIGATION_MAPPING_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    NAVIGATION_MAPPING_OK = 0,             /**< 本次操作完成。 */
    NAVIGATION_MAPPING_ERROR,              /**< 底层操作失败。 */
    NAVIGATION_MAPPING_BUSY,               /**< 资源正在使用。 */
    NAVIGATION_MAPPING_TIMEOUT            /**< 超过调用方指定的等待时间。 */
} NavigationMappingStatus;

typedef struct {
    void *context;
    NavigationMappingStatus (*write)(void *context, const uint8_t *data, size_t length);
} NavigationMappingByteWriter;

typedef struct {
    void *context;
    NavigationMappingStatus (*read)(void *context, uint8_t address, uint8_t register_address,
                         uint8_t *data, size_t length);
    NavigationMappingStatus (*write)(void *context, uint8_t address, uint8_t register_address,
                          const uint8_t *data, size_t length);
} NavigationMappingRegisterBus;

typedef struct {
    void *context;
    uint32_t (*milliseconds)(void *context);
    void (*delay_milliseconds)(void *context, uint32_t duration);
} NavigationMappingClock;
#ifdef __cplusplus
}
#endif
#endif
