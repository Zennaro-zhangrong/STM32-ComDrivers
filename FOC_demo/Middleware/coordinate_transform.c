/** @file coordinate_transform.c @brief 与 HAL、PWM 和共享全局对象无关的坐标变换。 */
#include "coordinate_transform.h"
#include <math.h>
void Clarke_Forward(ClarkeTransform *transform)
{
    float phase_a = (float)transform->phase_a;
    float phase_b = (float)transform->phase_b;
    float phase_c = (float)transform->phase_c;
    transform->alpha = (0.6666666667f * (phase_a - phase_b * 0.5f - phase_c * 0.5f));
    transform->beta = (0.6666666667f * (phase_b - phase_c) * 0.8660254038f);
}
void Clarke_Inverse(ClarkeTransform *transform)
{
    float alpha = (float)transform->alpha, beta = (float)transform->beta;
    transform->phase_a = alpha;
    transform->phase_b = (-0.5f * alpha + 0.8660254038f * beta);
    transform->phase_c = (-0.5f * alpha - 0.8660254038f * beta);
}
void Park_Forward(ParkTransform *transform)
{
    float alpha = (float)transform->alpha, beta = (float)transform->beta;
    float angle = fmodf(transform->angle_degrees, 360.0f) * 0.01745329252f;
    transform->direct = (alpha * cosf(angle) + beta * sinf(angle));
    transform->quadrature = (-alpha * sinf(angle) + beta * cosf(angle));
}
void Park_Inverse(ParkTransform *transform)
{
    float direct = (float)transform->direct, quadrature = (float)transform->quadrature;
    float angle = fmodf(transform->angle_degrees, 360.0f) * 0.01745329252f;
    transform->alpha = (direct * cosf(angle) - quadrature * sinf(angle));
    transform->beta = (direct * sinf(angle) + quadrature * cosf(angle));
}
