/** @file battery_service.c @brief 来源：BatteryVoltage.c；调度周期和界面回调由集成方管理。 */
#include "battery_service.h"
#include "../Middleware/sample_filter.h"
#include <math.h>
void BatteryService_Initialize(BatteryService *service)
{
    *service = (BatteryService){0};
    service->internal_resistance_ohms = 0.7f;
}
void BatteryService_AddSample(BatteryService *service, uint16_t sample)
{
    service->samples[service->write_index++] = sample;
    if (service->write_index == 11) service->write_index = 0;
    if (service->sample_count < 11) ++service->sample_count;
}
void BatteryService_Update(BatteryService *service, float load_milliamperes)
{
    if (service->sample_count == 0 || !isfinite(load_milliamperes) ||
        !isfinite(service->fixed_drop_volts) || !isfinite(service->internal_resistance_ohms)) return;
    uint16_t median = SampleFilter_Median(service->samples, service->scratch, service->sample_count);
    float drop = service->fixed_drop_volts + service->internal_resistance_ohms * load_milliamperes * 0.001f;
    float voltage = ((float)median * 0.0008056f / 3.4f + 3.3f + drop) * 10000.0f;
    if (!isfinite(voltage)) return;
    if (voltage < 0) voltage = 0;
    if (voltage > UINT16_MAX) voltage = UINT16_MAX;
    if (!service->history_initialized) {
        for (unsigned index = 0; index < 11; ++index) {
            service->output_history[index] = median;
            service->compensated_history[index] = (uint16_t)voltage;
        }
        service->history_initialized = 1;
    }
    float measured = (float)SampleFilter_Average(service->output_history, 11, median) / 4096.0f;
    service->output_voltage = measured * 3.3f / 3.4f + 3.3f;
    service->compensated_voltage = (float)SampleFilter_Average(service->compensated_history, 11, (uint16_t)voltage) / 10000.0f;
    float percentage = (service->compensated_voltage - 3.7f) * 200.0f;
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;
    service->percentage = (uint8_t)percentage;
}
