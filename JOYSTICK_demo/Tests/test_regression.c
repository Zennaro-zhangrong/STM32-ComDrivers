/** @file test_regression.c @brief 修复后的边界与错误路径回归测试；不访问真实硬件。 */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "../Service/joystick_service.h"
int main(void)
{
    JoystickAxis axis={.center_sample=2048,.direction=1,.positive_gain=2,.negative_gain=3};
    Joystick_Update(&axis,2048); assert(axis.value==0);
    Joystick_Update(&axis,2056); assert(axis.value==2);
    Joystick_Update(&axis,2040); assert(axis.value==-3);
    axis.positive_gain=1e30f; Joystick_Update(&axis,4095); assert(axis.value==INT16_MAX);
    axis.negative_gain=1e30f; Joystick_Update(&axis,0); assert(axis.value==INT16_MIN);
    axis.direction=0; Joystick_Update(&axis,4095); assert(axis.value==0);
    axis.direction=1; axis.positive_gain=NAN; Joystick_Update(&axis,4095); assert(axis.value==0);
    return 0;
}
