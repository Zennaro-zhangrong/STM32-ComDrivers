/** @file lvgl8_adapter.h
 * @brief 可选 LVGL 8 适配，不复制 LVGL 源码；使用者提供 lvgl.h 与配置。
 * @note 仅支持源工程 RGB565 配置。图形库生命周期由应用拥有，不含页面布局。
 */
#ifndef COMDRIVERS_LVGL8_ADAPTER_H
#define COMDRIVERS_LVGL8_ADAPTER_H
#include "lvgl.h"
#include "../display_transfer_service.h"
#ifdef __cplusplus
extern "C" {
#endif
/** @brief 在应用的 flush_cb 中调用；service.complete 应设置为 Lvgl8Adapter_Complete。 */
void Lvgl8Adapter_Flush(DisplayTransferService *service, lv_disp_drv_t *driver,
                         const lv_area_t *area, lv_color_t *pixels);
/** @brief 作为显示服务的成功/失败回调，两种结果都释放LVGL刷新缓冲；owner 必须是对应 lv_disp_drv_t。 */
void Lvgl8Adapter_Complete(void *owner, bool success);
/** @brief 把输入状态转换为 LVGL 指针数据；释放时由调用方传入上次坐标。 */
void Lvgl8Adapter_SetTouch(lv_indev_data_t *data, bool pressed, uint16_t x, uint16_t y);
#ifdef __cplusplus
}
#endif
#endif
