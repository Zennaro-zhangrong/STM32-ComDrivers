/** @file heading_control_service.c @brief 来源：Motion_Control；车辆模式、航点来源由调用方决定。 */
#include "heading_control_service.h"
#include "../Middleware/navigation_math.h"
#include <math.h>
void HeadingControl_Initialize(HeadingControlService *service)
{
    *service = (HeadingControlService){.proportional_gain=8.0f, .integral_gain=0.5f,
        .sample_seconds=0.05f, .integral_limit=200, .base_speed=40,
        .maximum_speed=80, .maximum_turn=156, .steering_offset=30};
}
void HeadingControl_Update(HeadingControlService *service, bool enabled, float target, float feedback)
{
    if (!enabled || !isfinite(target) || !isfinite(feedback) || !isfinite(service->sample_seconds) ||
        service->sample_seconds <= 0 || !isfinite(service->integral_limit) || service->integral_limit < 0 ||
        !isfinite(service->proportional_gain) || !isfinite(service->integral_gain) ||
        service->maximum_turn < 0 || service->maximum_speed < 0) {
        service->forward = service->turn = 0; service->integral = 0; return;
    }
    float error = Navigation_NormalizeError(fmodf(target, 360) - fmodf(feedback, 360));
    double candidate = fmax(-service->integral_limit, fmin(service->integral_limit,
        service->integral + (double)error * service->sample_seconds));
    double output = service->proportional_gain * (double)error + service->integral_gain * candidate + service->steering_offset;
    double integral_change = service->integral_gain * (candidate - service->integral);
    if ((output > service->maximum_turn && integral_change > 0) ||
        (output < -service->maximum_turn && integral_change < 0)) {
        output = service->proportional_gain * (double)error + service->integral_gain * (double)service->integral + service->steering_offset;
    } else service->integral = (float)candidate;
    if (output > service->maximum_turn) output = service->maximum_turn;
    if (output < -service->maximum_turn) output = -service->maximum_turn;
    service->turn = (int16_t)output;
    float factor = fmaxf(0, cosf(error * 0.017453292519943295f));
    service->forward = (int16_t)fmaxf(0, fminf(service->maximum_speed, service->base_speed * factor));
}
