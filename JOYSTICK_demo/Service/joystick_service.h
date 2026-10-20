/** @file joystick_service.h @brief 摇杆去偏置、方向和分段增益；不绑定 ADC 通道或界面。 */
#ifndef COMDRIVERS_JOYSTICK_SERVICE_H
#define COMDRIVERS_JOYSTICK_SERVICE_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    uint16_t center_sample; /**< 校准时的 ADC 值。 */
    int8_t direction;       /**< 仅允许 +1 或 -1。 */
    float negative_gain, positive_gain;
    int16_t value;          /**< ADC差值/8后乘增益，最终饱和至int16_t范围；不附加死区。 */
} JoystickAxis;
/** @brief 更新单轴，仅在最终输出时截断；无效方向或非有限/负增益输出0。 */
void Joystick_Update(JoystickAxis *axis, uint16_t sample);
#ifdef __cplusplus
}
#endif
#endif
