/** @file sample_filter.h @brief 电池采样所用中值和滑动均值，缓冲区由调用方管理。 */
#ifndef COMDRIVERS_SAMPLE_FILTER_H
#define COMDRIVERS_SAMPLE_FILTER_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/** @brief 求中值（偶数取较小中值）；scratch至少count项且不得与samples重叠，无效参数返回0。 */
uint16_t SampleFilter_Median(const uint16_t *samples, uint16_t *scratch, uint8_t count);
/** @brief 移动窗口并返回均值；count范围1～255，空指针或零窗口返回0。 */
uint16_t SampleFilter_Average(uint16_t *history, uint8_t count, uint16_t sample);
#ifdef __cplusplus
}
#endif
#endif
