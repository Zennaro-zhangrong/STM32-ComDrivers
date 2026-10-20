/** @file ft6336g_service.h
 * @brief FT6336G 初始化与坐标读取。横屏坐标转换包含宽高边界校验，分离 I2C 和界面对象。
 * @note 对象先清零，再设置 bus、address（原 0x38）、width/height（横屏320/240）。
 *       读取失败返回FT6336G_READ_ERROR并保留上次坐标，不能把失败当作释放。
 */
#ifndef COMDRIVERS_FT6336G_SERVICE_H
#define COMDRIVERS_FT6336G_SERVICE_H
#include "../Mapping/mapping.h"
#ifdef __cplusplus
extern "C" {
#endif
enum { FT6336G_READ_ERROR = 255 };
typedef struct {
    DisplayMappingRegisterBus bus;
    uint8_t address;
    uint16_t width, height;
    DisplayMappingStatus last_status;
    uint16_t x[2], y[2];
} Ft6336gService;
/** @return 读到厂商 ID 0x11 返回 1，且所有配置写入成功返回1，否则0。 */
uint8_t Ft6336g_Initialize(Ft6336gService *service);
/** @brief 读取并更新坐标；返回0～2触点数，错误返回255；释放和失败时保留上次坐标。 */
uint8_t Ft6336g_ReadCoordinates(Ft6336gService *service);
#ifdef __cplusplus
}
#endif
#endif
