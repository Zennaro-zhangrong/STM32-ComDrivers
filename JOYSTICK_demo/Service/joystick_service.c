/** @file joystick_service.c @brief 来源：Hardware/ADC.c 的 Joystick_ADC_Get。 */
#include "joystick_service.h"
#include <math.h>
void Joystick_Update(JoystickAxis *axis, uint16_t sample)
{
    if (axis->direction != 1 && axis->direction != -1) { axis->value = 0; return; }
    float value = (float)(axis->direction * ((int32_t)sample - axis->center_sample)) / 8.0f;
    float gain = value < 0 ? axis->negative_gain : axis->positive_gain;
    if (!isfinite(gain) || gain < 0) { axis->value = 0; return; }
    value *= gain;
    if (value > INT16_MAX) value = INT16_MAX;
    if (value < INT16_MIN) value = INT16_MIN;
    axis->value = (int16_t)value;
}
