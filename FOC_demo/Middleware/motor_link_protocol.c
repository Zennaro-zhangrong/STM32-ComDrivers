/** @file motor_link_protocol.c @brief 限定帧位数，仅在验证完成后刷新接收时刻。 */
#include "motor_link_protocol.h"
#include <stddef.h>
static int16_t DecodeSigned(uint16_t value)
{
    return (int16_t)(value <= INT16_MAX ? (int32_t)value : (int32_t)value - 65536);
}
void MotorLink_OnClock(MotorLinkDecoder *decoder, bool chip_select_high, bool state_bit,
                       bool joystick_bit, uint32_t milliseconds)
{
    if (chip_select_high) {
        uint32_t state = decoder->state_word, joystick = decoder->joystick_word;
        bool valid = decoder->bit_index == 32 && (state == 1 || state == 2 || state == 4);
        decoder->bit_index = 0;
        decoder->state_word = decoder->joystick_word = 0;
        if (valid) {
            decoder->received_frame = true;
            decoder->previous_receive_milliseconds = milliseconds;
            if (decoder->on_frame != NULL)
                decoder->on_frame(decoder->context, state, DecodeSigned((uint16_t)(joystick >> 16)), DecodeSigned((uint16_t)joystick));
        }
        return;
    }
    if (decoder->bit_index >= 32) { decoder->bit_index = 33; return; }
    if (state_bit) decoder->state_word |= UINT32_C(0x80000000) >> decoder->bit_index;
    if (joystick_bit) decoder->joystick_word |= UINT32_C(0x80000000) >> decoder->bit_index;
    ++decoder->bit_index;
}
bool MotorLink_HasTimedOut(const MotorLinkDecoder *decoder, uint32_t milliseconds)
{
    return !decoder->received_frame || (uint32_t)(milliseconds - decoder->previous_receive_milliseconds) >= 500;
}
uint32_t MotorLink_PackJoystick(int16_t forward, int16_t turn)
{
    return ((uint32_t)(uint16_t)forward << 16) | (uint16_t)turn;
}
