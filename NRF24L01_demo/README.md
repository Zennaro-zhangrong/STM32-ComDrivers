# NRF24L01_demo

无线收发与遥控载荷编解码。

**独立复制整个本目录即可，无其他demo依赖。** 本目录内部三层为`Mapping/`、`Middleware/`、`Service/`。没有实际代码的层仅用README说明边界，不制造空实现。

## 文件

- `Mapping/mapping.h`
- `Mapping/STM32F4/stm32f4_mapping.c`
- `Mapping/STM32F4/stm32f4_mapping.h`
- `Middleware/remote_control_protocol.c`
- `Middleware/remote_control_protocol.h`
- `Service/nrf24l01_service.c`
- `Service/nrf24l01_service.h`

## 加入工程

1. 复制`NRF24L01_demo/`，保持内部相对路径。
2. 将其`Middleware/`、`Service/`中的`.c`加入编译。
3. 选择映射实现，或按接口连接自己的硬件。不要同时编译两份相同demo。
4. 使用`#include "NRF24L01_demo/Service/...h"`或对应中间层头文件时，将demo所在父目录加入包含路径。
5. CMake项目可使用`add_subdirectory(NRF24L01_demo)`和`target_link_libraries(your_target PRIVATE comdrivers_nrf24l01)`。本demo自己的CMakeLists不依赖库根目录文件。

已按问题清单修复可由代码确认的缺陷，并加入边界/错误路径回归测试；仍不是完整烧录工程。

硬件合同使用`Nrf24l01Mapping...`类型，STM32适配使用`Nrf24l01Stm32f4_...`前缀，多个demo可同时链接。

问题跟踪编号：BUG-017、BUG-018、BUG-023、BUG-030。详见本目录[修复记录](KNOWN_ISSUES.md)。

## 代码来源

源工程只作为读取来源，本 demo 不依赖这些工程目录。下表路径相对于对应源项目根目录。

| 来源项目 | 源文件 | 当前归属 | 抽取边界 |
| --- | --- | --- | --- |
| CarMainControlBoard | `DriverLayer/Src/NRF24L01.c` | `NRF24L01_demo/Service/nrf24l01_service` + `NRF24L01_demo/Middleware/remote_control_protocol` | 寄存器与收发、遥控载荷分离；接收回调交付数据，车辆动作不写进设备服务 |
| LVGL-_stm32f407vet6 | `Hardware/NRF24L01.c` | NRF24L01_demo 无线服务 + `NRF24L01_demo/Middleware/remote_control_protocol` | 保留固定帧；采用该工程1ms CE延时接口，主控空循环延时留作源问题记录 |

## 构建映射层

手动集成时，将 Mapping/STM32F4 下的 C 文件加入工程，并提供 HAL、CMSIS、配置头文件及芯片型号宏。使用本 demo 的 CMake 时，在 add_subdirectory 前设置 COMDRIVERS_ENABLE_STM32F4=ON、COMDRIVERS_STM32_INCLUDE_DIRS 为相应头文件目录列表；COMDRIVERS_STM32_DEVICE 默认为 STM32F407xx。目标固件本身也需配置这些外部依赖并链接 HAL 实现。

## 回归测试

`Tests/test_regression.c` 只使用模拟数据及回调，不访问真实外设。将本 demo 的可移植源码与该文件一起编译即可运行。CMake 可设置 `COMDRIVERS_BUILD_TESTS=ON`；主机编译后运行 CTest，ARM 交叉编译只能检查编译/链接。

`Tests/` 不需要复制到固件工程。STM32F4 及 LVGL 适配仍由外部依赖提供，实板结果不包含在本机回归结论内。

## 本轮接口迁移

- Initialize/Send返回Nrf24l01Result；Send的OK表示受理，查询transmit_state获取送达结果，忙时返回BUSY。
- 时钟接口须提供milliseconds，周期调用Supervise处理无IRQ超时；默认发送超时100ms，可在初始化前配置。
- ProcessInterrupt按FIFO状态读取最多3帧，允许周期调用。SPI错误记录在last_bus_status；总线恢复后按应用策略重新初始化或重新提交请求。
- RemoteControl_Decode新增长度，必须恰好9字节；无线回调推荐DecodeFrame，传入期望命令码，同时验证命令和长度。

寄存器行为依据：[Nordic nRF24L01 产品规范](https://devzone.nordicsemi.com/cfs-file/__key/support-attachments/beef5d1b77644c448dabff31668f3a47-aad1a46f307945a7b0204fd969e86bdf/content.pdf)。
