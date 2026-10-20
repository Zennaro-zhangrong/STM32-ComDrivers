/** @file remote_control_protocol.h @brief 遥控器9字节载荷；板端模式与灯光策略留给集成应用。 */
#ifndef COMDRIVERS_REMOTE_CONTROL_PROTOCOL_H
#define COMDRIVERS_REMOTE_CONTROL_PROTOCOL_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    int16_t left_x, left_y, right_x, right_y;
    uint8_t switches; /**< bit4左开关、bit3右开关、bit2闭环选择、bit1:0速度档位。 */
} RemoteControlInput;
/** @brief payload 必须至少9字节；大端写入四轴值，再写开关位。 */
void RemoteControl_Encode(const RemoteControlInput *input, uint8_t *payload);
/** @brief 仅接收恰好9字节载荷；无效输入返回false且保持input不变。 */
bool RemoteControl_Decode(RemoteControlInput *input, const uint8_t *payload, size_t length);
/** @brief 无线回调使用此入口；expected_command由应用的协议命令表提供。 */
bool RemoteControl_DecodeFrame(RemoteControlInput *input, uint8_t command, uint8_t expected_command,
    const uint8_t *payload, size_t length);
#ifdef __cplusplus
}
#endif
#endif
