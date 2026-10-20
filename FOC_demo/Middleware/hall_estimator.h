/** @file hall_estimator.h
 * @brief 六扇区霍尔测速与插值；同一对象的边沿和监督调用必须串行化。
 * @note 计数器必须为同一32位单调时基，间隔小于2^31；接线顺序和电角零偏需实板标定。
 */
#ifndef COMDRIVERS_HALL_ESTIMATOR_H
#define COMDRIVERS_HALL_ESTIMATOR_H
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    uint32_t previous_tick, pulse_period; /**< 无符号边沿周期，单位为外部计数。 */
    float frequency; /**< 带方向的机械转数/秒。 */
    uint8_t average_length, history_count;
    float *frequency_history;
    uint8_t sector, previous_sector;
    int8_t direction;
    float tick_frequency_hz, electrical_zero_offset_degrees;
    uint16_t pole_pairs;
} HallEstimator;
/** @brief 初始化默认20kHz时基、1极对；必须按实际硬件覆盖配置后再使用速度反馈。
 * @note history=NULL或length=0禁用估算，接口不会访问空窗口。
 */
void HallEstimator_Initialize(HallEstimator *estimator, float *history, uint8_t length,
                              uint8_t pin_bits, uint32_t initial_tick);
bool HallEstimator_ConfigureTimebase(HallEstimator *estimator, float tick_frequency_hz, uint16_t pole_pairs);
void HallEstimator_OnEdge(HallEstimator *estimator, uint8_t pin_bits, uint32_t tick);
/** @brief 超过上个边沿周期的两倍视为停转，清空整个历史窗口。 */
void HallEstimator_Supervise(HallEstimator *estimator, uint32_t tick);
/** @brief 返回[0,359]电角度；无速度时保持当前扇区基准，插值最多一个扇区。 */
int16_t HallEstimator_Angle(const HallEstimator *estimator, uint32_t tick);
#ifdef __cplusplus
}
#endif
#endif
