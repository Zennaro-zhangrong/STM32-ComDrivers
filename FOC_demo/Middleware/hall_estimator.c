/** @file hall_estimator.c @brief 无符号时间差、完整六扇区方向判断及停转清理。 */
#include "hall_estimator.h"
#include <stddef.h>
#include <math.h>
static uint8_t DecodeSector(uint8_t bits)
{
    static const uint8_t sectors[8] = {0,4,6,5,2,3,1,0};
    return bits < 8 ? sectors[bits] : 0;
}
static void ClearSpeed(HallEstimator *estimator)
{
    estimator->frequency = 0;
    estimator->pulse_period = 0;
    estimator->history_count = 0;
    estimator->direction = 0;
    if (estimator->frequency_history != NULL)
        for (unsigned index = 0; index < estimator->average_length; ++index) estimator->frequency_history[index] = 0;
}
void HallEstimator_Initialize(HallEstimator *estimator, float *history, uint8_t length,
                              uint8_t pin_bits, uint32_t initial_tick)
{
    *estimator = (HallEstimator){.frequency_history=history, .average_length=history != NULL ? length : 0,
        .previous_tick=initial_tick, .sector=DecodeSector(pin_bits), .tick_frequency_hz=20000, .pole_pairs=1};
    ClearSpeed(estimator);
}
bool HallEstimator_ConfigureTimebase(HallEstimator *estimator, float tick_frequency_hz, uint16_t pole_pairs)
{
    if (!isfinite(tick_frequency_hz) || tick_frequency_hz <= 0 || pole_pairs == 0) return false;
    estimator->tick_frequency_hz = tick_frequency_hz;
    estimator->pole_pairs = pole_pairs;
    ClearSpeed(estimator);
    return true;
}
void HallEstimator_OnEdge(HallEstimator *estimator, uint8_t pin_bits, uint32_t tick)
{
    uint8_t sector = DecodeSector(pin_bits);
    if (sector == 0) { estimator->sector = 0; ClearSpeed(estimator); return; }
    if (sector == estimator->sector) return;
    uint32_t elapsed = tick - estimator->previous_tick;
    uint8_t difference = (uint8_t)((sector + 6 - estimator->sector) % 6);
    if (estimator->sector == 0 || (difference != 1 && difference != 5)) {
        estimator->sector = sector;
        estimator->previous_tick = tick;
        ClearSpeed(estimator);
        return;
    }
    if (elapsed == 0 || elapsed >= UINT32_C(0x80000000)) return;
    estimator->previous_sector = estimator->sector;
    estimator->sector = sector;
    estimator->previous_tick = tick;
    if (estimator->average_length == 0 || estimator->frequency_history == NULL ||
        estimator->pole_pairs == 0 || !isfinite(estimator->tick_frequency_hz) || estimator->tick_frequency_hz <= 0) {
        ClearSpeed(estimator); return;
    }
    int8_t direction = difference == 1 ? 1 : -1;
    if (direction != estimator->direction) ClearSpeed(estimator);
    estimator->direction = direction;
    estimator->pulse_period = elapsed;
    for (unsigned index = 1; index < estimator->average_length; ++index)
        estimator->frequency_history[index - 1] = estimator->frequency_history[index];
    estimator->frequency_history[estimator->average_length - 1] =
        direction * (estimator->tick_frequency_hz / 6.0f / estimator->pole_pairs / elapsed);
    if (estimator->history_count < estimator->average_length) ++estimator->history_count;
    double sum = 0;
    for (unsigned index = 0; index < estimator->average_length; ++index) sum += estimator->frequency_history[index];
    estimator->frequency = (float)(sum / estimator->history_count);
}
void HallEstimator_Supervise(HallEstimator *estimator, uint32_t tick)
{
    if (estimator->pulse_period != 0 && (uint64_t)(tick - estimator->previous_tick) > (uint64_t)estimator->pulse_period * 2)
        ClearSpeed(estimator);
}
int16_t HallEstimator_Angle(const HallEstimator *estimator, uint32_t tick)
{
    if (estimator->sector == 0 || !isfinite(estimator->electrical_zero_offset_degrees)) return 0;
    double angle = (estimator->sector - 1) * 60.0;
    if (estimator->direction < 0) angle += 60;
    if (estimator->pulse_period != 0) {
        double fraction = (double)(tick - estimator->previous_tick) / estimator->pulse_period;
        if (fraction > 1) fraction = 1;
        angle += estimator->direction * 60.0 * fraction;
    }
    angle = fmod(angle + estimator->electrical_zero_offset_degrees, 360.0);
    if (angle < 0) angle += 360;
    if (angle >= 360) angle = 0;
    return (int16_t)angle;
}
