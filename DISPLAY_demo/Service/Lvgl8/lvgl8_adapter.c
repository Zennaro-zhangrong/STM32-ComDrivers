/** @file lvgl8_adapter.c @brief 来源：lv_port_disp.c 和 lv_port_indev.c 的接口转换部分。 */
#include "lvgl8_adapter.h"
#if LV_COLOR_DEPTH != 16
#error "Lvgl8Adapter requires RGB565 (LV_COLOR_DEPTH == 16)"
#endif
void Lvgl8Adapter_Flush(DisplayTransferService *service, lv_disp_drv_t *driver,
                         const lv_area_t *area, lv_color_t *pixels)
{
    if (area->x1 < 0 || area->y1 < 0 || area->x2 < area->x1 || area->y2 < area->y1) {
        lv_disp_flush_ready(driver); return;
    }
    if (!DisplayTransfer_Enqueue(service, driver, (uint16_t)area->x1, (uint16_t)area->y1,
        (uint16_t)area->x2, (uint16_t)area->y2, (const uint8_t *)pixels)) lv_disp_flush_ready(driver);
}
void Lvgl8Adapter_Complete(void *owner, bool success) { (void)success; lv_disp_flush_ready((lv_disp_drv_t *)owner); }
void Lvgl8Adapter_SetTouch(lv_indev_data_t *data, bool pressed, uint16_t x, uint16_t y)
{
    data->state = pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
    data->point.x = (lv_coord_t)x;
    data->point.y = (lv_coord_t)y;
}
