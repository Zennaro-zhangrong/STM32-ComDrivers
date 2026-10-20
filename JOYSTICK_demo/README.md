# JOYSTICK_demo

摇杆去偏置、方向与分段增益。

**独立复制整个本目录即可，无其他demo依赖。** 本目录内部三层为`Mapping/`、`Middleware/`、`Service/`。没有实际代码的层仅用README说明边界，不制造空实现。

## 文件

- `Service/joystick_service.c`
- `Service/joystick_service.h`

## 加入工程

1. 复制`JOYSTICK_demo/`，保持内部相对路径。
2. 将其`Middleware/`、`Service/`中的`.c`加入编译。
3. 选择映射实现，或按接口连接自己的硬件。不要同时编译两份相同demo。
4. 使用`#include "JOYSTICK_demo/Service/...h"`或对应中间层头文件时，将demo所在父目录加入包含路径。
5. CMake项目可使用`add_subdirectory(JOYSTICK_demo)`和`target_link_libraries(your_target PRIVATE comdrivers_joystick)`。本demo自己的CMakeLists不依赖库根目录文件。

已按问题清单修复可由代码确认的缺陷，并加入边界/错误路径回归测试；仍不是完整烧录工程。

## 代码来源

源工程只作为读取来源，本 demo 不依赖这些工程目录。下表路径相对于对应源项目根目录。

| 来源项目 | 源文件 | 当前归属 | 抽取边界 |
| --- | --- | --- | --- |
| LVGL-_stm32f407vet6 | `Hardware/ADC.c` | `JOYSTICK_demo/Service/joystick_service` | 单轴中心校准、方向和正负增益；ADC通道/启动由应用管理 |

## 回归测试

`Tests/test_regression.c` 只使用模拟数据及回调，不访问真实外设。将本 demo 的可移植源码与该文件一起编译即可运行。CMake 可设置 `COMDRIVERS_BUILD_TESTS=ON`；主机编译后运行 CTest，ARM 交叉编译只能检查编译/链接。

`Tests/` 不需要复制到固件工程。STM32F4 及 LVGL 适配仍由外部依赖提供，实板结果不包含在本机回归结论内。

## 本轮接口迁移

- 运算只在最终输出时转换为int16_t，超范围饱和；无效direction、负增益或非有限增益输出0。
- direction仍只接受+1/-1；校准中心、增益及应用死区由板级提供。
