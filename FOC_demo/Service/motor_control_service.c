/** @file motor_control_service.c @brief 来源：FOC_EG2134/Motor.c、DataReception.c。 */
#include "motor_control_service.h"
#include "../Middleware/coordinate_transform.h"
#include <math.h>
#include <stddef.h>

void MotorControl_SetMode(MotorControlService *service, MotorControlMode mode)
{
    if (mode != MOTOR_SPEED_CONTROL && mode != MOTOR_CURRENT_CONTROL) mode = MOTOR_BRAKE;
    if (!service->mode_initialized || service->previous_mode != mode || mode == MOTOR_BRAKE) {
        FocIncrementalPid_Reset(&service->direct_current);
        FocIncrementalPid_Reset(&service->quadrature_current);
        FocIncrementalPid_Reset(&service->speed);
        service->direct_current.target = service->quadrature_current.target = service->speed.target = 0;
        service->previous_mode = mode;
        service->mode_initialized = 1;
    }
    service->mode = mode;
}
static void SynchronizeMode(MotorControlService *service)
{
    if (!service->mode_initialized) {
        FocIncrementalPid_Reset(&service->direct_current);
        FocIncrementalPid_Reset(&service->quadrature_current);
        FocIncrementalPid_Reset(&service->speed);
        service->previous_mode = service->mode;
        service->mode_initialized = 1;
    }
    if (service->previous_mode != service->mode || service->mode == MOTOR_BRAKE ||
        (service->mode != MOTOR_SPEED_CONTROL && service->mode != MOTOR_CURRENT_CONTROL))
        MotorControl_SetMode(service, service->mode);
}

void MotorControl_SetSamples(MotorControlService *service, int16_t sample_b, int16_t sample_c)
{
    service->current_phase_b = (service->current_gain_b * (sample_b - service->current_bias_b));
    service->current_phase_c = (service->current_gain_c * (sample_c - service->current_bias_c));
    service->current_phase_a = -service->current_phase_b - service->current_phase_c;
}
void MotorControl_UpdateCurrent(MotorControlService *service)
{
    SynchronizeMode(service);
    if (service->mode == MOTOR_BRAKE || !isfinite(service->current_phase_a) ||
        !isfinite(service->current_phase_b) || !isfinite(service->current_phase_c) ||
        !isfinite(service->electrical_angle_degrees)) {
        service->quadrature_current.previous_output = service->quadrature_current.output = 0;
        service->direct_current.previous_output = service->direct_current.output = 0;
        if (service->output.write != NULL) service->output.write(service->output.context, 0, 0, 0);
        return;
    }
    ClarkeTransform clarke = {service->current_phase_a, service->current_phase_b, service->current_phase_c, 0, 0};
    Clarke_Forward(&clarke);
    ParkTransform park = {clarke.alpha, clarke.beta, 0, 0, service->electrical_angle_degrees};
    Park_Forward(&park);
    FocIncrementalPid_Update(&service->quadrature_current, service->quadrature_current.target, (float)park.quadrature);
    FocIncrementalPid_Update(&service->direct_current, service->direct_current.target, (float)park.direct);
    park.direct = service->direct_current.output;
    park.quadrature = service->quadrature_current.output;
    Park_Inverse(&park);
    service->modulator.alpha = park.alpha;
    service->modulator.beta = park.beta;
    SpaceVectorPwm_Update(&service->modulator);
    if (service->output.write != NULL) service->output.write(service->output.context, (uint16_t)service->modulator.duty_a,
        (uint16_t)service->modulator.duty_b, (uint16_t)service->modulator.duty_c);
}
void MotorControl_UpdateSpeed(MotorControlService *service)
{
    SynchronizeMode(service);
    if (service->mode == MOTOR_BRAKE) {
        FocIncrementalPid_Reset(&service->speed);
        return;
    }
    if (service->mode != MOTOR_SPEED_CONTROL) return;
    if (!isfinite(service->speed.target) || !isfinite(service->speed_feedback)) {
        FocIncrementalPid_Reset(&service->speed);
        service->quadrature_current.target = 0;
        return;
    }
    if (service->speed.target > -1 && service->speed.target < 1) {
        FocIncrementalPid_Reset(&service->speed);
        service->quadrature_current.target = service->speed.output;
        return;
    }
    FocIncrementalPid_Update(&service->speed, service->speed.target, service->speed_feedback);
    service->direct_current.target = 0;
    float correction = service->speed.target * 0.8f;
    correction += correction > 0 ? 50 : -50;
    service->speed.output += correction;
    if (service->speed.output > service->speed.maximum_output) service->speed.output = service->speed.maximum_output;
    if (service->speed.output < service->speed.minimum_output) service->speed.output = service->speed.minimum_output;
    service->quadrature_current.target = service->speed.output;
    if (!isfinite(service->maximum_current_target) || service->maximum_current_target <= 0)
        service->quadrature_current.target = 0;
    else {
        if (service->quadrature_current.target > service->maximum_current_target)
            service->quadrature_current.target = service->maximum_current_target;
        if (service->quadrature_current.target < -service->maximum_current_target)
            service->quadrature_current.target = -service->maximum_current_target;
    }
}
static float LimitTarget(float target, float maximum)
{
    if (!isfinite(maximum) || maximum <= 0) return 0;
    return fmaxf(-maximum, fminf(maximum, target));
}
void MotorControl_SetDifferentialTarget(MotorControlService *left, MotorControlService *right,
                                        MotorControlMode mode, int16_t forward, int16_t turn)
{
    MotorControl_SetMode(left, mode);
    MotorControl_SetMode(right, mode);
    if (mode == MOTOR_SPEED_CONTROL) {
        left->speed.target = -(forward / 3 + turn / 28);
        right->speed.target = forward / 3 - turn / 28;
    } else if (mode == MOTOR_CURRENT_CONTROL) {
        left->quadrature_current.target = LimitTarget((float)-(forward + turn), left->maximum_current_target);
        right->quadrature_current.target = LimitTarget((float)(forward - turn), right->maximum_current_target);
    }
}
