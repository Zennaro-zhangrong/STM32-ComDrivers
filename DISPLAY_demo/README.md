# DISPLAY_demo

ILI9341显示、FT6336G触摸、显示队列及可选LVGL8适配。

**独立复制整个本目录即可，无其他demo依赖。** 本目录内部三层为`Mapping/`、`Middleware/`、`Service/`。没有实际代码的层仅用README说明边界，不制造空实现。

## 文件

- `Mapping/mapping.h`
- `Mapping/STM32F4/stm32f4_mapping.c`
- `Mapping/STM32F4/stm32f4_mapping.h`
- `Service/display_transfer_service.c`
- `Service/display_transfer_service.h`
- `Service/ft6336g_service.c`
- `Service/ft6336g_service.h`
- `Service/ili9341_service.c`
- `Service/ili9341_service.h`
- `Service/Lvgl8/lvgl8_adapter.c`
- `Service/Lvgl8/lvgl8_adapter.h`

## 加入工程

1. 复制`DISPLAY_demo/`，保持内部相对路径。
2. 将其`Middleware/`、`Service/`中的`.c`加入编译；`Service/Lvgl8/`仅在使用LVGL8时加入。
3. 选择映射实现，或按接口连接自己的硬件。不要同时编译两份相同demo。
4. 使用`#include "DISPLAY_demo/Service/...h"`或对应中间层头文件时，将demo所在父目录加入包含路径。
5. CMake项目可使用`add_subdirectory(DISPLAY_demo)`和`target_link_libraries(your_target PRIVATE comdrivers_display)`。本demo自己的CMakeLists不依赖库根目录文件。

已按问题清单修复可由代码确认的缺陷，并加入边界/错误路径回归测试；仍不是完整烧录工程。

硬件合同使用`DisplayMapping...`类型，STM32适配使用`DisplayStm32f4_...`前缀，多个demo可同时链接。

问题跟踪编号：BUG-019、BUG-024、BUG-025。详见本目录[修复记录](KNOWN_ISSUES.md)。

## 代码来源

源工程只作为读取来源，本 demo 不依赖这些工程目录。下表路径相对于对应源项目根目录。

| 来源项目 | 源文件 | 当前归属 | 抽取边界 |
| --- | --- | --- | --- |
| LVGL-_stm32f407vet6 | `Screen_Driver/FT6336G.c` | `DISPLAY_demo/Service/ft6336g_service` | 原寄存器设置、单次快照读取、横屏边界校验；I2C注入 |
| LVGL-_stm32f407vet6 | `Screen_Driver/LCD.c` | `DISPLAY_demo/Service/ili9341_service` + `DISPLAY_demo/Service/display_transfer_service` | 原86项初始化命令/参数/延时序列，窗口、像素传输、双队列 |
| LVGL-_stm32f407vet6 | `lvgl/examples/porting/lv_port_disp.c`、`lv_port_indev.c` | `DISPLAY_demo/Service/Lvgl8` | 仅适配刷新/触摸接口；LVGL注册、缓冲分配仍由应用持有 |

## 构建映射层

手动集成时，将 Mapping/STM32F4 下的 C 文件加入工程，并提供 HAL、CMSIS、配置头文件及芯片型号宏。使用本 demo 的 CMake 时，在 add_subdirectory 前设置 COMDRIVERS_ENABLE_STM32F4=ON、COMDRIVERS_STM32_INCLUDE_DIRS 为相应头文件目录列表；COMDRIVERS_STM32_DEVICE 默认为 STM32F407xx。目标固件本身也需配置这些外部依赖并链接 HAL 实现。

## 显示与触摸

队列的 start 回调现在返回 bool；complete 回调新增 success 参数。入队返回 true 表示已接受，返回 false 时缓冲仍由调用方持有。所有队列入口由同一任务串行调用，DMA 中断只传递事件。

```c
static bool StartDisplayTransfer(void *context, const DisplayTransfer *transfer)
{
    Ili9341Service *screen = context;
    if (Ili9341_SetWindow(screen, transfer->left, transfer->top,
                         transfer->right, transfer->bottom) != DISPLAY_MAPPING_OK)
        return false;
    return Ili9341_StartPixels(screen, transfer->pixels, transfer->byte_count) == DISPLAY_MAPPING_OK;
}

/* 每一块DMA完成后在同一任务调用；只有整次请求结束才通知队列。 */
static void OnDisplayDmaComplete(Ili9341Service *screen, DisplayTransferService *queue)
{
    DisplayMappingStatus status = Ili9341_OnPixelsComplete(screen);
    if (status == DISPLAY_MAPPING_OK) DisplayTransfer_OnComplete(queue);
    else if (status != DISPLAY_MAPPING_BUSY) DisplayTransfer_OnError(queue);
}

/* 板级已确认DMA停止访问缓冲后调用。 */
static void OnDisplayDmaError(Ili9341Service *screen, DisplayTransferService *queue)
{
    Ili9341_AbortPixels(screen);
    DisplayTransfer_OnError(queue);
}
```

设置 `start_pixels=DisplayStm32f4_StartSpiPixels`、`pixel_context` 为SPI上下文，单块限制65534字节，整屏会自动分块。RGB565字节序由应用与屏幕配置统一。队列完成回调可设置 `Lvgl8Adapter_Complete`，成功和失败都会归还LVGL刷新缓冲；应用如需显示错误，应包装该回调记录success。

触摸必须配置 width/height，横屏通常320/240。Ft6336g_ReadCoordinates 返回0～2表示有效触点数，255表示读取失败；错误不能当作松开事件，读取失败时坐标保留。初始化也会检查每次寄存器写入结果。

## 回归测试

`Tests/test_regression.c` 只使用模拟数据及回调，不访问真实外设。将本 demo 的可移植源码与该文件一起编译即可运行。CMake 可设置 `COMDRIVERS_BUILD_TESTS=ON`；主机编译后运行 CTest，ARM 交叉编译只能检查编译/链接。

`Tests/` 不需要复制到固件工程。STM32F4 及 LVGL 适配仍由外部依赖提供，实板结果不包含在本机回归结论内。

## 本轮接口迁移

- DisplayTransfer.byte_count改为size_t，Enqueue返回bool；start回调返回bool，complete回调增加bool success。
- Ili9341_SetWindow/StartPixels返回状态；每次DMA完成都调用OnPixelsComplete，只有返回OK时才能结束队列请求；BUSY表示已续传下一块。
- 总线或DMA错误必须先确认DMA停止，再AbortPixels和DisplayTransfer_OnError。操作队列与图形库统一放在任务中。
- Ft6336gService新增height；ReadCoordinates返回255代表错误，0才代表释放。
