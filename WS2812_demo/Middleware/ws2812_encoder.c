/** @file ws2812_encoder.c @brief 有容量检查及可配置复位时间的波形编码。 */
#include "ws2812_encoder.h"
size_t Ws2812_RequiredSamples(size_t count, size_t reset_cycles)
{
    if (reset_cycles == 0 || reset_cycles > SIZE_MAX / 2 || count > (SIZE_MAX - reset_cycles * 2) / 24) return 0;
    return reset_cycles * 2 + count * 24;
}
size_t Ws2812_ResetCycles(uint32_t reset_nanoseconds, uint32_t period_nanoseconds)
{
    if (period_nanoseconds == 0) return 0;
    return reset_nanoseconds / period_nanoseconds + (reset_nanoseconds % period_nanoseconds != 0);
}
size_t Ws2812_Encode(uint16_t *output, size_t capacity, const uint32_t *colors, size_t count,
    uint8_t brightness, uint16_t logic_zero, uint16_t logic_one, size_t reset_cycles)
{
    size_t required = Ws2812_RequiredSamples(count, reset_cycles);
    if (output == NULL || (count != 0 && colors == NULL) || required == 0 || capacity < required) return 0;
    for (size_t index = 0; index < reset_cycles; ++index) output[index] = 0;
    for (size_t led = 0; led < count; ++led) {
        uint32_t color = 0;
        for (unsigned channel = 0; channel < 3; ++channel) {
            uint32_t value = ((colors[led] >> (channel * 8)) & 255u) * brightness;
            color |= ((value + 127u) / 255u) << (channel * 8);
        }
        for (size_t bit = 0; bit < 24; ++bit)
            output[reset_cycles + led * 24 + bit] = color & (0x00800000u >> bit) ? logic_one : logic_zero;
    }
    for (size_t index = required - reset_cycles; index < required; ++index) output[index] = 0;
    return required;
}
