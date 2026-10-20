/** @file sample_filter.c @brief 来源：LVGL 工程 Hardware/BatteryVoltage.c。 */
#include "sample_filter.h"
#include <stdlib.h>
#include <string.h>
static int CompareSamples(const void *left, const void *right)
{
    uint16_t left_value = *(const uint16_t *)left;
    uint16_t right_value = *(const uint16_t *)right;
    return left_value < right_value ? -1 : left_value > right_value ? 1 : 0;
}
uint16_t SampleFilter_Median(const uint16_t *samples, uint16_t *scratch, uint8_t count)
{
    if (samples == NULL || scratch == NULL || count == 0) return 0;
    memcpy(scratch, samples, count * sizeof(uint16_t));
    qsort(scratch, count, sizeof(uint16_t), CompareSamples);
    return scratch[(count - 1) / 2];
}
uint16_t SampleFilter_Average(uint16_t *history, uint8_t count, uint16_t sample)
{
    if (history == NULL || count == 0) return 0;
    unsigned window = count;
    uint32_t sum = 0;
    for (unsigned index = 0; index + 1 < window; ++index) {
        history[index] = history[index + 1];
        sum += history[index];
    }
    sum += sample;
    history[window - 1] = sample;
    return (uint16_t)(sum / window);
}
