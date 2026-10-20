/** @file test_regression.c @brief 修复后的边界与错误路径回归测试；不访问真实硬件。 */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "../Middleware/pid_controller.h"
int main(void)
{
    PositionPidController pid = {.proportional_gain=0.25f,.target=1,.maximum_output=100000};
    PositionPid_Update(&pid); assert(pid.output == 0.25f && pid.accumulated_error == 0);
    pid.proportional_gain=0; pid.integral_gain=-2; pid.target=100;
    pid.maximum_output=10; PositionPid_Update(&pid);
    assert(pid.output == -10 && pid.accumulated_error == 5);
    pid.integral_gain=0; pid.proportional_gain=100000; pid.target=100;
    pid.maximum_output=100000; PositionPid_Update(&pid); assert(pid.output == 100000);
    pid.target=NAN; PositionPid_Update(&pid); assert(pid.output == 0);
    IncrementalPidController inc = {.proportional_gain=1,.maximum_output=5,.minimum_output=-5};
    assert(IncrementalPid_Update(&inc,10,0) == 5);
    IncrementalPid_Reset(&inc); assert(inc.previous_error == 0 && inc.output == 0);
    assert(IncrementalPid_Update(&inc,INFINITY,0) == 0);
    return 0;
}
