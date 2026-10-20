/** @file navigation_service.h
 * @brief 文本定位入口、位置上报及航点目标推进；不创建接收中断和操作系统任务。
 * @note Feed/Upload/Advance 由同一执行上下文调用；字节队列交给板级管理。
 */
#ifndef COMDRIVERS_NAVIGATION_SERVICE_H
#define COMDRIVERS_NAVIGATION_SERVICE_H
#include "../Middleware/navigation_protocol.h"
#include "../Mapping/mapping.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    char line[128];
    uint16_t line_length;
    bool line_overflow, position_valid, target_valid;
    NavigationPosition position, target;
    float target_distance_meters, target_bearing_degrees;
    uint32_t previous_upload_milliseconds;
    NavigationMappingByteWriter uplink;
} NavigationService;
/** @brief 对象先清零；逐字节接收，遇换行同步解析。成功解析返回1，否则0。
 * @note 溢出整行丢弃；失锁时清position_valid；不在中断中执行文本解析。
 */
uint8_t NavigationService_Feed(NavigationService *service, uint8_t value);
/** @brief 每3000ms尝试上报一次位置；显式小端编码，无有效定位时不上传。 */
void NavigationService_Upload(NavigationService *service, uint32_t milliseconds);
/** @brief 发出0x1A轨迹申请并清目标，保留源协议。 */
void NavigationService_RequestRoute(NavigationService *service);
/** @brief 计算目标方位，进入到达半径则取下一航点。
 * @return 1 表示仍有目标，0 表示目标到达且已无下一点。
 * @note target_valid表示目标有效，零经纬度也是合法目标；无定位时返回0且不推进航点。
 */
uint8_t NavigationService_Advance(NavigationService *service, WaypointDecoder *route, float arrival_meters);
#ifdef __cplusplus
}
#endif
#endif
