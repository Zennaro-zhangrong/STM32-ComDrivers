/** @file space_vector_pwm.c @brief 保留七段矢量序列，限幅后才转换为整数比较值。 */
#include "space_vector_pwm.h"
#include <math.h>
static int CompareValue(double value, uint16_t maximum)
{
    if (value < 0) return 0;
    if (value > maximum) return maximum;
    return (int)lround(value);
}
void SpaceVectorPwm_FromPolar(SpaceVectorPwm *modulator, float amplitude, float angle_degrees)
{
    if (!isfinite(amplitude) || !isfinite(angle_degrees) || amplitude < 0) {
        modulator->duty_a = modulator->duty_b = modulator->duty_c = 0;
        modulator->sector = 0; return;
    }
    double angle = fmod((double)angle_degrees, 360.0);
    if (angle < 0) angle += 360;
    if (angle >= 360) angle = 0; /* 极小负角度加360可能舍入到360，避免扇区下标越界。 */
    unsigned sector = (unsigned)(angle / 60.0);
    modulator->sector = (uint8_t)(sector + 1);
    modulator->angle_degrees = (float)angle;
    double local_angle = (angle - sector * 60.0) * 0.017453292519943295;
    double first = amplitude * sin(1.047197551196597746 - local_angle) / 0.866025403784438646;
    double second = amplitude * sin(local_angle) / 0.866025403784438646;
    if (first + second > modulator->maximum_duty) {
        double scale = modulator->maximum_duty / (first + second);
        first *= scale; second *= scale;
    }
    double zero = fmax(0, (modulator->maximum_duty - first - second) * 0.5);
    static const uint8_t first_vectors[6] = {1,3,2,6,4,5};
    static const uint8_t second_vectors[6] = {3,2,6,4,5,1};
    uint8_t a = first_vectors[sector], b = second_vectors[sector];
    modulator->duty_a = CompareValue(zero + ((a & 1) ? first : 0) + ((b & 1) ? second : 0), modulator->maximum_duty);
    modulator->duty_b = CompareValue(zero + ((a & 2) ? first : 0) + ((b & 2) ? second : 0), modulator->maximum_duty);
    modulator->duty_c = CompareValue(zero + ((a & 4) ? first : 0) + ((b & 4) ? second : 0), modulator->maximum_duty);
}
void SpaceVectorPwm_Update(SpaceVectorPwm *modulator)
{
    double magnitude = hypot((double)modulator->alpha, (double)modulator->beta);
    /* 超大输入只改变方向，幅值限制在浮点范围内；后续按PWM周期缩放。 */
    if (magnitude > 3.402823466e38) magnitude = 3.402823466e38;
    float angle = (float)(atan2((double)modulator->beta, (double)modulator->alpha) * 57.29577951308232);
    if (!isfinite(modulator->alpha) || !isfinite(modulator->beta)) magnitude = NAN;
    SpaceVectorPwm_FromPolar(modulator, (float)magnitude, angle);
}
