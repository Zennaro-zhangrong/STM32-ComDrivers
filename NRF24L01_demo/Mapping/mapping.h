/** @file mapping.h
 * @brief NRF24L01_demo的硬件接口，仅声明本功能需要的操作。
 * @note 回调及context由集成工程提供；同步函数返回前完成访问。
 *       DMA缓冲保持至完成通知；只允许映射实现解释HAL句柄。
 */
#ifndef COMDRIVERS_NRF24L01_MAPPING_H
#define COMDRIVERS_NRF24L01_MAPPING_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    NRF24L01_MAPPING_OK = 0,             /**< 本次操作完成。 */
    NRF24L01_MAPPING_ERROR,              /**< 底层操作失败。 */
    NRF24L01_MAPPING_BUSY,               /**< 资源正在使用。 */
    NRF24L01_MAPPING_TIMEOUT            /**< 超过调用方指定的等待时间。 */
} Nrf24l01MappingStatus;

typedef struct {
    void *context;
    Nrf24l01MappingStatus (*exchange)(void *context, const uint8_t *transmit,
                              uint8_t *receive, size_t length);
} Nrf24l01MappingSpiBus;

typedef struct {
    void *context;
    void (*write)(void *context, bool high); /**< high 表示物理高电平。 */
    bool (*read)(void *context);            /**< 返回物理电平。 */
} Nrf24l01MappingDigitalPin;

typedef struct {
    void *context;
    uint32_t (*milliseconds)(void *context);
    void (*delay_milliseconds)(void *context, uint32_t duration);
} Nrf24l01MappingClock;
#ifdef __cplusplus
}
#endif
#endif
