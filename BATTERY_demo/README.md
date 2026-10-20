# BATTERY_demo

电池采样滤波、负载补偿与电量估算。

**独立复制整个本目录即可，无其他demo依赖。** 本目录内部三层为`Mapping/`、`Middleware/`、`Service/`。没有实际代码的层仅用README说明边界，不制造空实现。

## 文件

- `Middleware/sample_filter.c`
- `Middleware/sample_filter.h`
- `Service/battery_service.c`
- `Service/battery_service.h`

## 加入工程

1. 复制`BATTERY_demo/`，保持内部相对路径。
2. 将其`Middleware/`、`Service/`中的`.c`加入编译。
3. 选择映射实现，或按接口连接自己的硬件。不要同时编译两份相同demo。
4. 使用`#include "BATTERY_demo/Service/...h"`或对应中间层头文件时，将demo所在父目录加入包含路径。
5. CMake项目可使用`add_subdirectory(BATTERY_demo)`和`target_link_libraries(your_target PRIVATE comdrivers_battery)`。本demo自己的CMakeLists不依赖库根目录文件。

已按问题清单修复可由代码确认的缺陷，并加入边界/错误路径回归测试；仍不是完整烧录工程。

问题跟踪编号：BUG-010、BUG-011。详见本目录[修复记录](KNOWN_ISSUES.md)。

## 代码来源

源工程只作为读取来源，本 demo 不依赖这些工程目录。下表路径相对于对应源项目根目录。

| 来源项目 | 源文件 | 当前归属 | 抽取边界 |
| --- | --- | --- | --- |
| LVGL-_stm32f407vet6 | `Hardware/BatteryVoltage.c` | `BATTERY_demo/Middleware` + `BATTERY_demo/Service` | 中值/均值、补偿、电量换算；传入负载电流，移除LCD和GUI全局依赖 |

## 回归测试

`Tests/test_regression.c` 只使用模拟数据及回调，不访问真实外设。将本 demo 的可移植源码与该文件一起编译即可运行。CMake 可设置 `COMDRIVERS_BUILD_TESTS=ON`；主机编译后运行 CTest，ARM 交叉编译只能检查编译/链接。

`Tests/` 不需要复制到固件工程。STM32F4 及 LVGL 适配仍由外部依赖提供，实板结果不包含在本机回归结论内。

## 本轮接口迁移

- 首次有效样本用于初始化均值历史，不再让全零历史影响预热电压。
- 电量先限幅再转换，范围0～100%；补偿电压仍用源工程的固定点历史分辨率，上限6.5535V。
- 滤波窗口支持1～255，零窗口返回0；电路和电量模型仍需按实际分压、偏置和电池曲线校准。
