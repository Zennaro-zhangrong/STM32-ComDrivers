/** @file mapping.h
 * @brief FOC_demo的硬件接口，仅声明本功能需要的操作。
 * @note 回调及context由集成工程提供；同步函数返回前完成访问。
 *       只允许映射实现解释HAL句柄；同一硬件的并发访问由调用方协调。
 */
#ifndef COMDRIVERS_FOC_MAPPING_H
#define COMDRIVERS_FOC_MAPPING_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    FOC_MAPPING_OK = 0,             /**< 本次操作完成。 */
    FOC_MAPPING_ERROR,              /**< 底层操作失败。 */
    FOC_MAPPING_BUSY,               /**< 资源正在使用。 */
    FOC_MAPPING_TIMEOUT            /**< 超过调用方指定的等待时间。 */
} FocMappingStatus;

typedef struct {
    void *context; /**< 调用方持有的硬件上下文，使用期间必须有效。 */
    /** @brief 写入三相比较值；单位为定时器计数，范围由板级周期配置确定。 */
    void (*write)(void *context, uint16_t phase_a, uint16_t phase_b, uint16_t phase_c);
} FocMappingThreePhasePwm;

typedef struct {
    void *context;
    void (*write)(void *context, bool high); /**< high 表示物理高电平。 */
    bool (*read)(void *context);            /**< 返回物理电平。 */
} FocMappingDigitalPin;

typedef struct {
    void *context;
    uint32_t (*milliseconds)(void *context); /**< 返回毫秒计数，允许32位自然回绕。 */
    void (*delay_milliseconds)(void *context, uint32_t duration); /**< 阻塞指定毫秒数。 */
} FocMappingClock;
#ifdef __cplusplus
}
#endif
#endif
