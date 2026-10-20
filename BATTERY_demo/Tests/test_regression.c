/** @file test_regression.c @brief 修复后的边界与错误路径回归测试；不访问真实硬件。 */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "../Middleware/sample_filter.h"
#include "../Service/battery_service.h"
int main(void)
{
    uint16_t history[255]; for (unsigned i=0;i<255;++i) history[i]=65535;
    assert(SampleFilter_Average(history,255,65535)==65535);
    assert(SampleFilter_Average(history,128,0)==65023);
    assert(SampleFilter_Average(history,0,99)==0);
    assert(SampleFilter_Median(history,history,0)==0);
    BatteryService battery; BatteryService_Initialize(&battery);
    BatteryService_Update(&battery,0); assert(battery.percentage==0);
    BatteryService_AddSample(&battery,0); BatteryService_Update(&battery,0);
    assert(battery.percentage==0 && fabs(battery.compensated_voltage-3.3f)<0.001f);
    for (unsigned i=0;i<11;++i) BatteryService_AddSample(&battery,4095);
    for (unsigned i=0;i<11;++i) BatteryService_Update(&battery,1000000);
    assert(battery.percentage==100 && battery.compensated_voltage<=6.5535f);
    float previous=battery.compensated_voltage;
    BatteryService_Update(&battery,NAN); assert(battery.compensated_voltage==previous);
    return 0;
}
