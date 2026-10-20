# LD14P_demo

LD14P雷达批次解析与网格转换。

**独立复制整个本目录即可，无其他demo依赖。** 本目录内部三层为`Mapping/`、`Middleware/`、`Service/`。没有实际代码的层仅用README说明边界，不制造空实现。

## 文件

- `Middleware/ld14p_parser.c`
- `Middleware/ld14p_parser.h`

## 加入工程

1. 复制`LD14P_demo/`，保持内部相对路径。
2. 将其`Middleware/`、`Service/`中的`.c`加入编译。
3. 选择映射实现，或按接口连接自己的硬件。不要同时编译两份相同demo。
4. 使用`#include "LD14P_demo/Service/...h"`或对应中间层头文件时，将demo所在父目录加入包含路径。
5. CMake项目可使用`add_subdirectory(LD14P_demo)`和`target_link_libraries(your_target PRIVATE comdrivers_ld14p)`。本demo自己的CMakeLists不依赖库根目录文件。

已按问题清单修复可由代码确认的缺陷，并加入边界/错误路径回归测试；仍不是完整烧录工程。

问题跟踪编号：BUG-015、BUG-016。详见本目录[修复记录](KNOWN_ISSUES.md)。

## 代码来源

源工程只作为读取来源，本 demo 不依赖这些工程目录。下表路径相对于对应源项目根目录。

| 来源项目 | 源文件 | 当前归属 | 抽取边界 |
| --- | --- | --- | --- |
| CarMainControlBoard | `DriverLayer/Src/ld14p.c` | `LD14P_demo/Middleware/ld14p_parser` | 2632字节批次解析和网格转换；串口节拍、雷达转发任务由应用调度 |

## 回归测试

`Tests/test_regression.c` 只使用模拟数据及回调，不访问真实外设。将本 demo 的可移植源码与该文件一起编译即可运行。CMake 可设置 `COMDRIVERS_BUILD_TESTS=ON`；主机编译后运行 CTest，ARM 交叉编译只能检查编译/链接。

`Tests/` 不需要复制到固件工程。STM32F4 及 LVGL 适配仍由外部依赖提供，实板结果不包含在本机回归结论内。

## 本轮接口迁移

- CheckHeaders新增输入字节数，只检查头部；真正CRC检查由ParseBatch逐包执行。
- ParseBatch新增输入字节数和输出容量，最多56包；输出尾部清零，返回有效点数。
- ConvertGrid新增有效点数和输出容量，仅处理有效点，返回bool；偏置改为int32_t，单位0.01度。
- 校验采用厂家SDK对应CRC-8多项式0x4D；实际雷达帧和串口接收时序仍需实机确认。

协议依据：[厂家 LD14P SDK 的帧校验实现](https://github.com/ldrobotSensorTeam/ldlidar_sl_sdk/blob/master/ldlidar_driver/src/lipkg.cpp)。本库使用独立逐位CRC实现，不复制厂家的查表源码。
