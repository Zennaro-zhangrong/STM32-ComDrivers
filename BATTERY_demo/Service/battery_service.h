/** @file battery_service.h
 * @brief 11 点中值、均值与负载补偿；电流由调用方提供，不依赖 LCD 亮度对象。
 * @note 电路公式沿用源硬件；电量先在浮点域限幅，不是通用电量计模型。
 */
#ifndef COMDRIVERS_BATTERY_SERVICE_H
#define COMDRIVERS_BATTERY_SERVICE_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    uint16_t samples[11], output_history[11], compensated_history[11], scratch[11];
    uint8_t write_index, percentage;
    float output_voltage, compensated_voltage;
    float fixed_drop_volts, internal_resistance_ohms;
    uint8_t sample_count; /**< 当前有效采样数；未收到样本时Update不更新结果。 */
    uint8_t history_initialized; /**< 首次更新用当前值填充均值窗口，消除零值预热偏差。 */
} BatteryService;
/** @brief 清零状态并采用源工程内阻0.7欧姆；首个有效计算会填充均值历史。 */
void BatteryService_Initialize(BatteryService *service);
/** @brief 写入一次电池 ADC 采样；与 Update 必须由调用方串行化。 */
void BatteryService_AddSample(BatteryService *service, uint16_t sample);
/** @brief 更新电压、电量；load_milliamperes 为估计负载电流，原模型为 150+亮度*7.14。 */
void BatteryService_Update(BatteryService *service, float load_milliamperes);
#ifdef __cplusplus
}
#endif
#endif
