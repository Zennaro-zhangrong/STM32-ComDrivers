/** @file pid_controller.c @brief 从目标库 PID 和 FOC_EG2134/PID.c 抽取。 */
#include "pid_controller.h"
#include <math.h>
void PositionPid_Reset(PositionPidController *controller)
{
    controller->error = 0;
    controller->previous_error = 0;
    controller->accumulated_error = 0;
    controller->output = 0;
}
void PositionPid_Update(PositionPidController *controller)
{
    if (!isfinite(controller->target) || !isfinite(controller->feedback) ||
        !isfinite(controller->maximum_output) || controller->maximum_output < 0 ||
        !isfinite(controller->proportional_gain) || !isfinite(controller->integral_gain) ||
        !isfinite(controller->derivative_gain)) { PositionPid_Reset(controller); return; }
    controller->error = controller->target - controller->feedback;
    if (!isfinite(controller->error)) { PositionPid_Reset(controller); return; }
    if (controller->integral_gain == 0) controller->accumulated_error = 0;
    else {
        double integral_limit = (double)controller->maximum_output / fabs(controller->integral_gain);
        double accumulated = (double)controller->accumulated_error + controller->error;
        if (accumulated > integral_limit) accumulated = integral_limit;
        if (accumulated < -integral_limit) accumulated = -integral_limit;
        if (accumulated > 3.402823466e38) accumulated = 3.402823466e38;
        if (accumulated < -3.402823466e38) accumulated = -3.402823466e38;
        controller->accumulated_error = (float)accumulated;
    }
    double output = (double)controller->proportional_gain * controller->error +
        (double)controller->integral_gain * controller->accumulated_error +
        (double)controller->derivative_gain * ((double)controller->error - controller->previous_error);
    if (output > controller->maximum_output) output = controller->maximum_output;
    if (output < -controller->maximum_output) output = -controller->maximum_output;
    controller->output = (float)output;
    controller->previous_error = controller->error;
}

void IncrementalPid_Reset(IncrementalPidController *controller)
{
    controller->previous_output = controller->output = 0;
    controller->previous_error = controller->second_previous_error = 0;
}
float IncrementalPid_Update(IncrementalPidController *controller, float target, float feedback)
{
    if (!isfinite(target) || !isfinite(feedback) || !isfinite(controller->proportional_gain) ||
        !isfinite(controller->integral_gain) || !isfinite(controller->derivative_gain) ||
        !isfinite(controller->maximum_output) || !isfinite(controller->minimum_output) ||
        controller->minimum_output > controller->maximum_output) { IncrementalPid_Reset(controller); return 0; }
    controller->target = target; controller->feedback = feedback;
    double error = (double)target - feedback;
    if (error > 3.402823466e38 || error < -3.402823466e38) { IncrementalPid_Reset(controller); return 0; }
    double increment = controller->proportional_gain * (error - controller->previous_error) +
        controller->integral_gain * error + controller->derivative_gain *
        (error - 2.0 * controller->previous_error + controller->second_previous_error);
    double output = controller->previous_output + increment;
    if (output > controller->maximum_output) output = controller->maximum_output;
    if (output < controller->minimum_output) output = controller->minimum_output;
    controller->second_previous_error = controller->previous_error;
    controller->previous_error = (float)error;
    controller->previous_output = controller->output = (float)output;
    return controller->output;
}
