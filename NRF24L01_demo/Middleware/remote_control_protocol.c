/** @file remote_control_protocol.c @brief 来源：nrf24l01_mainSend 与 NRF24L01_ProcessRxData。 */
#include "remote_control_protocol.h"
void RemoteControl_Encode(const RemoteControlInput *input, uint8_t *payload)
{
    const int16_t axes[4] = {input->left_x, input->left_y, input->right_x, input->right_y};
    for (unsigned index = 0; index < 4; ++index) {
        payload[index * 2] = (uint8_t)((uint16_t)axes[index] >> 8);
        payload[index * 2 + 1] = (uint8_t)axes[index];
    }
    payload[8] = input->switches;
}
bool RemoteControl_Decode(RemoteControlInput *input, const uint8_t *payload, size_t length)
{
    if (input == NULL || payload == NULL || length != 9) return false;
    input->left_x = (int16_t)(((uint16_t)payload[0] << 8) | payload[1]);
    input->left_y = (int16_t)(((uint16_t)payload[2] << 8) | payload[3]);
    input->right_x = (int16_t)(((uint16_t)payload[4] << 8) | payload[5]);
    input->right_y = (int16_t)(((uint16_t)payload[6] << 8) | payload[7]);
    input->switches = payload[8];
    return true;
}

bool RemoteControl_DecodeFrame(RemoteControlInput *input, uint8_t command, uint8_t expected_command,
    const uint8_t *payload, size_t length)
{
    return command == expected_command && RemoteControl_Decode(input, payload, length);
}
