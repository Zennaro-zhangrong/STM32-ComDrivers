# NAVIGATION_demo

定位与导航：GPS行解析、航点、位置上报、航向控制、IMU航向读取。

**独立复制整个本目录即可，无其他demo依赖。** 本目录内部三层为`Mapping/`、`Middleware/`、`Service/`。没有实际代码的层仅用README说明边界，不制造空实现。

## 文件

- `Mapping/mapping.h`
- `Mapping/STM32F4/stm32f4_mapping.c`
- `Mapping/STM32F4/stm32f4_mapping.h`
- `Middleware/navigation_math.c`
- `Middleware/navigation_math.h`
- `Middleware/navigation_protocol.c`
- `Middleware/navigation_protocol.h`
- `Service/heading_control_service.c`
- `Service/heading_control_service.h`
- `Service/imu_heading_service.c`
- `Service/imu_heading_service.h`
- `Service/navigation_service.c`
- `Service/navigation_service.h`

## 加入工程

1. 复制`NAVIGATION_demo/`，保持内部相对路径。
2. 将其`Middleware/`、`Service/`中的`.c`加入编译。
3. 选择映射实现，或按接口连接自己的硬件。不要同时编译两份相同demo。
4. 使用`#include "NAVIGATION_demo/Service/...h"`或对应中间层头文件时，将demo所在父目录加入包含路径。
5. CMake项目可使用`add_subdirectory(NAVIGATION_demo)`和`target_link_libraries(your_target PRIVATE comdrivers_navigation)`。本demo自己的CMakeLists不依赖库根目录文件。

已按问题清单修复可由代码确认的缺陷，并加入边界/错误路径回归测试；仍不是完整烧录工程。

硬件合同使用`NavigationMapping...`类型，STM32适配使用`NavigationStm32f4_...`前缀，多个demo可同时链接。

问题跟踪编号：BUG-012、BUG-013、BUG-014、BUG-020、BUG-026。详见本目录[修复记录](KNOWN_ISSUES.md)。

## 代码来源

源工程只作为读取来源，本 demo 不依赖这些工程目录。下表路径相对于对应源项目根目录。

| 来源项目 | 源文件 | 当前归属 | 抽取边界 |
| --- | --- | --- | --- |
| CarMainControlBoard | `DriverLayer/Src/Motion.c` | `NAVIGATION_demo/Middleware` + `NAVIGATION_demo/Service` | 度分转换、距离方位、航向PI、到达判定；车辆启停/模式选择交还应用 |
| CarMainControlBoard | `DriverLayer/Src/gps.c` | `NAVIGATION_demo/Middleware/navigation_protocol` + `NAVIGATION_demo/Service/navigation_service` | RMC解析、行组装、位置上报和路线申请；ISR双缓冲/信号量由板级接收入口代替 |
| CarMainControlBoard | `DriverLayer/Src/g4.c` | `NAVIGATION_demo/Middleware/navigation_protocol` 中的 `WaypointDecoder` | 字节状态机与航点存储；零长度命令通过回调交付，语音/车灯不进入解析器 |
| CarMainControlBoard | `DriverLayer/Src/IMU.c` | `NAVIGATION_demo/Middleware/navigation_math` + `NAVIGATION_demo/Service/imu_heading_service` | 标定插值、航向读取；实车37点标定表由使用者按传感器提供 |

## 构建映射层

手动集成时，将 Mapping/STM32F4 下的 C 文件加入工程，并提供 HAL、CMSIS、配置头文件及芯片型号宏。使用本 demo 的 CMake 时，在 add_subdirectory 前设置 COMDRIVERS_ENABLE_STM32F4=ON、COMDRIVERS_STM32_INCLUDE_DIRS 为相应头文件目录列表；COMDRIVERS_STM32_DEVICE 默认为 STM32F407xx。目标固件本身也需配置这些外部依赖并链接 HAL 实现。

## 回归测试

`Tests/test_regression.c` 只使用模拟数据及回调，不访问真实外设。将本 demo 的可移植源码与该文件一起编译即可运行。CMake 可设置 `COMDRIVERS_BUILD_TESTS=ON`；主机编译后运行 CTest，ARM 交叉编译只能检查编译/链接。

`Tests/` 不需要复制到固件工程。STM32F4 及 LVGL 适配仍由外部依赖提供，实板结果不包含在本机回归结论内。

## 本轮接口迁移

- WaypointDecoder_Feed新增毫秒时间戳，字节间隔>=500ms丢弃半帧；可显式ResetFrame。不带帧头的旧协议无法从任意噪声自动找回边界，接收端必须在超时后从新的command字节开始。
- RMC必须带正确校验和；无效定位清position_valid，手动注入位置时也必须设置该标志。
- target_valid取代零坐标哨兵，0/0可以是合法航点。完整航点帧到达前不公开其中的点。
- IMU错误返回NAN，推荐ImuHeading_TryRead并检查bool；线上的浮点仍按源模块的小端IEEE754弧度解释。
- 航向输出包含机械偏置后的最终限幅；目标板需重新确认PI参数和运动方向。
