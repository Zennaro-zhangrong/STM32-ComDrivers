/** @file incremental_pid.c @brief FOC自带增量式PID，计算规则未改动。 */
#include "incremental_pid.h"
#include <math.h>
void FocIncrementalPid_Reset(FocIncrementalPidController *controller)
{
    controller->previous_output = controller->output = 0;
    controller->previous_error = controller->second_previous_error = 0;
}
float FocIncrementalPid_Update(FocIncrementalPidController *controller, float target, float feedback)
{
    if (!isfinite(target) || !isfinite(feedback) || !isfinite(controller->proportional_gain) ||
        !isfinite(controller->integral_gain) || !isfinite(controller->derivative_gain) ||
        !isfinite(controller->maximum_output) || !isfinite(controller->minimum_output) ||
        controller->minimum_output > controller->maximum_output) { FocIncrementalPid_Reset(controller); return 0; }
    controller->target = target; controller->feedback = feedback;
    double error = (double)target - feedback;
    if (error > 3.402823466e38 || error < -3.402823466e38) { FocIncrementalPid_Reset(controller); return 0; }
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
