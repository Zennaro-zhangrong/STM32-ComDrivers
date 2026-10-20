# WS2812_demo

GRB灯带编码与PWM DMA映射。

**独立复制整个本目录即可，无其他demo依赖。** 本目录内部三层为`Mapping/`、`Middleware/`、`Service/`。没有实际代码的层仅用README说明边界，不制造空实现。

## 文件

- `Mapping/mapping.h`
- `Mapping/STM32F4/stm32f4_mapping.c`
- `Mapping/STM32F4/stm32f4_mapping.h`
- `Middleware/ws2812_encoder.c`
- `Middleware/ws2812_encoder.h`

## 加入工程

1. 复制`WS2812_demo/`，保持内部相对路径。
2. 将其`Middleware/`、`Service/`中的`.c`加入编译。
3. 选择映射实现，或按接口连接自己的硬件。不要同时编译两份相同demo。
4. 使用`#include "WS2812_demo/Service/...h"`或对应中间层头文件时，将demo所在父目录加入包含路径。
5. CMake项目可使用`add_subdirectory(WS2812_demo)`和`target_link_libraries(your_target PRIVATE comdrivers_ws2812)`。本demo自己的CMakeLists不依赖库根目录文件。

已按问题清单修复可由代码确认的缺陷，并加入边界/错误路径回归测试；仍不是完整烧录工程。

硬件合同使用`Ws2812Mapping...`类型，STM32适配使用`Ws2812Stm32f4_...`前缀，多个demo可同时链接。

问题跟踪编号：BUG-021。详见本目录[修复记录](KNOWN_ISSUES.md)。

## 代码来源

源工程只作为读取来源，本 demo 不依赖这些工程目录。下表路径相对于对应源项目根目录。

| 来源项目 | 源文件 | 当前归属 | 抽取边界 |
| --- | --- | --- | --- |
| CarMainControlBoard | `DriverLayer/Src/ws2812.c` | `WS2812_demo/Middleware/ws2812_encoder` + PWM DMA映射 | 保留GRB、亮度和波形编码；四块4×4灯板位置/图案/车辆灯语属于应用资源 |

## 构建映射层

手动集成时，将 Mapping/STM32F4 下的 C 文件加入工程，并提供 HAL、CMSIS、配置头文件及芯片型号宏。使用本 demo 的 CMake 时，在 add_subdirectory 前设置 COMDRIVERS_ENABLE_STM32F4=ON、COMDRIVERS_STM32_INCLUDE_DIRS 为相应头文件目录列表；COMDRIVERS_STM32_DEVICE 默认为 STM32F407xx。目标固件本身也需配置这些外部依赖并链接 HAL 实现。

## 回归测试

`Tests/test_regression.c` 只使用模拟数据及回调，不访问真实外设。将本 demo 的可移植源码与该文件一起编译即可运行。CMake 可设置 `COMDRIVERS_BUILD_TESTS=ON`；主机编译后运行 CTest，ARM 交叉编译只能检查编译/链接。

`Tests/` 不需要复制到固件工程。STM32F4 及 LVGL 适配仍由外部依赖提供，实板结果不包含在本机回归结论内。

## 本轮接口迁移

- Encode新增capacity和reset_cycles并返回实际样本数，参数错误或容量不足返回0。
- 用ResetCycles(reset_nanoseconds,period_nanoseconds)计算前后复位周期，用RequiredSamples分配容量；例如周期1250ns、复位300000ns得到前后各240周期。这个示例不替代所用灯珠型号的时序要求。
- 亮度按0～255线性缩放；DMA缓冲区必须保存到实际发送完成，复位期间输出须保持低电平。
