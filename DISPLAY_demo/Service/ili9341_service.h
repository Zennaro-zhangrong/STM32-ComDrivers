/** @file ili9341_service.h @brief ILI9341 命令序列与像素发送接口，图形库不进入设备层。 */
#ifndef COMDRIVERS_ILI9341_SERVICE_H
#define COMDRIVERS_ILI9341_SERVICE_H
#include "../Mapping/mapping.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    DisplayMappingByteWriter writer;     /**< 同步命令/参数写入。 */
    DisplayMappingDigitalPin chip_select, data_command, reset;
    DisplayMappingClock clock;
    void *pixel_context;
    DisplayMappingStatus (*start_pixels)(void *context, const uint8_t *pixels, uint16_t bytes);
    uint16_t width, height;
    DisplayMappingStatus last_status;
    const uint8_t *pending_pixels;
    size_t remaining_bytes;
    uint16_t active_bytes;
    bool pixels_active;
} Ili9341Service;
/** @brief 执行原屏幕复位及初始化序列，最终为270度横屏；背光 PWM 由板级单独启动。 */
void Ili9341_Initialize(Ili9341Service *service);
/** @brief 方向0/1/2/3分别为0/90/180/270度，其他值回退到0。 */
void Ili9341_SetDirection(Ili9341Service *service, uint8_t direction);
/** @brief 设置闭区间窗口，拒绝反向/越界窗口；调用前必须初始化宽高。 */
DisplayMappingStatus Ili9341_SetWindow(Ili9341Service *service, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom);
/** @brief 启动 RGB565 字节流；数据按屏幕字节序提供，缓冲保持至完成通知。
 * @note 按最多65534字节分块，每次DMA完成调用OnPixelsComplete；失败返回状态并释放片选。 */
DisplayMappingStatus Ili9341_StartPixels(Ili9341Service *service, const uint8_t *pixels, size_t byte_count);
/** @brief 单次DMA完成时调用；BUSY表示已续传下一块，OK表示整帧结束，其他值为失败。 */
DisplayMappingStatus Ili9341_OnPixelsComplete(Ili9341Service *service);
/** @brief DMA错误且已停止访问缓冲后调用，释放片选和内部状态。 */
void Ili9341_AbortPixels(Ili9341Service *service);
/** @brief 兼容清理入口；调用前必须确认DMA已结束或中止。 */
void Ili9341_FinishPixels(Ili9341Service *service);
#ifdef __cplusplus
}
#endif
#endif
