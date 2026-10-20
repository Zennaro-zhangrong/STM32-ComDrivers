/** @file motor_link_protocol.h @brief 双数据线32位协议；片选拉高后的时钟提交一帧。 */
#ifndef COMDRIVERS_MOTOR_LINK_PROTOCOL_H
#define COMDRIVERS_MOTOR_LINK_PROTOCOL_H
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    uint32_t state_word, joystick_word;
    uint8_t bit_index;
    uint32_t previous_receive_milliseconds;
    bool received_frame;
    void *context;
    void (*on_frame)(void *context, uint32_t state, int16_t forward, int16_t turn);
} MotorLinkDecoder;
/** @brief 对象先清零；每个时钟传入当前毫秒数，仅完整32位且状态为1/2/4的帧被提交。 */
void MotorLink_OnClock(MotorLinkDecoder *decoder, bool chip_select_high, bool state_bit,
                       bool joystick_bit, uint32_t milliseconds);
/** @brief 尚未收到合法帧或距最后合法帧>=500毫秒返回true，支持32位回绕。 */
bool MotorLink_HasTimedOut(const MotorLinkDecoder *decoder, uint32_t milliseconds);
uint32_t MotorLink_PackJoystick(int16_t forward, int16_t turn);
#ifdef __cplusplus
}
#endif
#endif
