/** @file heading_control_service.h @brief 可独立复用的航向 PI 与弯道减速服务。 */
#ifndef COMDRIVERS_HEADING_CONTROL_SERVICE_H
#define COMDRIVERS_HEADING_CONTROL_SERVICE_H
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    float proportional_gain, integral_gain, sample_seconds;
    float integral_limit, integral;
    int16_t base_speed, maximum_speed, maximum_turn, steering_offset;
    int16_t forward, turn;
} HeadingControlService;
/** @brief 使用源工程参数：8/0.5/50ms，速度40，转向156，机械偏置30。可在初始化后覆盖。 */
void HeadingControl_Initialize(HeadingControlService *service);
/** @brief enabled 为 false 时清积分并停车；航向单位为度。最终转向包含机械偏置且受限幅；饱和时仅允许积分向解除饱和的方向变化。 */
void HeadingControl_Update(HeadingControlService *service, bool enabled, float target, float feedback);
#ifdef __cplusplus
}
#endif
#endif
