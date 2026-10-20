/** @file navigation_protocol.h
 * @brief 从 GPS 与 4G 接收逻辑抽取的文本定位和二进制航点解析。
 * @note 无 UART、RTOS、车辆全局对象；固定长协议没有同步帧头，半帧超时后重新从命令字节开始。
 */
#ifndef COMDRIVERS_NAVIGATION_PROTOCOL_H
#define COMDRIVERS_NAVIGATION_PROTOCOL_H
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    int32_t longitude, latitude; /**< NMEA 度分原值乘 100000，不是十进制度。 */
} NavigationPosition;
/** @brief 解析完整的零结尾 GNRMC/GPRMC 行；成功返回 1，校验/格式/定位无效时清零position并返回0。 */
uint8_t Navigation_ParseRmc(const char *line, NavigationPosition *position);
typedef struct {
    uint8_t state, command;
    uint16_t payload_length, received_length;
    uint8_t coordinate_bytes[8];
    uint16_t staged_index;
    bool accept_payload;
    uint32_t previous_byte_milliseconds;
    NavigationPosition *waypoints; /**< 外部数组，至少 capacity 项。 */
    uint16_t capacity, write_index, read_index;
    void *event_context;
    void (*on_command)(void *context, uint8_t command); /**< 零长度业务命令通知，可为空。 */
} WaypointDecoder;
/** @brief 初始化航点解析；0x0A 为航点数据，0x0E 为清空，沿用源协议。 */
void WaypointDecoder_Initialize(WaypointDecoder *decoder, NavigationPosition *storage, uint16_t capacity);
/** @brief 顺序喂入一个字节，帧为command,length_high,length_low,payload；字节间隔>=500ms丢弃半帧。 */
void WaypointDecoder_Feed(WaypointDecoder *decoder, uint8_t value, uint32_t milliseconds);
/** @brief 丢弃半帧；保留之前完整接收的航点。 */
void WaypointDecoder_ResetFrame(WaypointDecoder *decoder);
/** @return 读到航点返回 1，耗尽返回 0；线性数组不会自动回收已读空间。 */
uint8_t WaypointDecoder_Read(WaypointDecoder *decoder, NavigationPosition *position);
#ifdef __cplusplus
}
#endif
#endif
