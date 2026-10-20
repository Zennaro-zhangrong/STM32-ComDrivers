/** @file ws2812_encoder.h @brief GRB颜色编码；复位低电平长度由实际PWM周期计算。 */
#ifndef COMDRIVERS_WS2812_ENCODER_H
#define COMDRIVERS_WS2812_ENCODER_H
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
/** @brief 返回前后复位周期加颜色周期所需容量；溢出或reset_cycles=0返回0。 */
size_t Ws2812_RequiredSamples(size_t count, size_t reset_cycles);
/** @brief 计算ceil(reset_nanoseconds/period_nanoseconds)；无效周期返回0。
 * @note 按实际灯珠手册选择复位时间，不能假定所有WS2812型号只需50微秒。
 */
size_t Ws2812_ResetCycles(uint32_t reset_nanoseconds, uint32_t period_nanoseconds);
/** @brief 返回实际样本数；容量不足或参数无效返回0且不写入。
 * @param colors 每灯珠0x00GGRRBB；brightness按0～255线性缩放，255保持原色。
 * @param reset_cycles 前后各输出的低电平周期数。
 */
size_t Ws2812_Encode(uint16_t *output, size_t capacity, const uint32_t *colors, size_t count,
    uint8_t brightness, uint16_t logic_zero, uint16_t logic_one, size_t reset_cycles);
#ifdef __cplusplus
}
#endif
#endif
