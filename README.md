# STM32-ComDrivers

按功能选择、复制即用的嵌入式组件库。**开发 FOC 并输出日志时，只需复制 `FOC_demo/` 和 `LOG_demo/`。** 每个 demo 内部分为映射层、中间层、服务层，所需代码随目录一起带走。

这里的 demo 是可移植功能包，不是完整烧录工程。主函数、任务调度、CubeMX 配置、启动文件和具体业务由使用者的工程提供。

## 目录与选择

| 目录 | 功能 | 其他 demo 依赖 |
| --- | --- | --- |
| [FOC_demo](FOC_demo/README.md) | 坐标变换、空间矢量调制、增量式 PID、霍尔估算、电流/速度环、双线电机协议 | 无 |
| [LOG_demo](LOG_demo/README.md) | 环形缓冲、格式化日志、UART 输出映射 | 无 |
| [PID_demo](PID_demo/README.md) | 通用位置式和增量式 PID | 无 |
| [NAVIGATION_demo](NAVIGATION_demo/README.md) | GPS、航点、距离方位、航向控制、IMU 航向 | 无 |
| [NRF24L01_demo](NRF24L01_demo/README.md) | 无线收发及遥控载荷解析 | 无 |
| [DISPLAY_demo](DISPLAY_demo/README.md) | ILI9341、FT6336G、显示传输、可选 LVGL 8 适配 | 无 |
| [JOYSTICK_demo](JOYSTICK_demo/README.md) | 摇杆校准与单轴换算 | 无 |
| [BATTERY_demo](BATTERY_demo/README.md) | 采样滤波、电压补偿与电量换算 | 无 |
| [WS2812_demo](WS2812_demo/README.md) | 灯带波形编码及 PWM DMA 映射 | 无 |
| [LD14P_demo](LD14P_demo/README.md) | 雷达批次解析与网格转换 | 无 |
| [COMMAND_demo](COMMAND_demo/README.md) | 命令派发、PWM 命令、SPI 双缓冲帧接收器 | 无 |

```text
STM32-ComDrivers/
  FOC_demo/
    Mapping/       硬件接口、STM32F4 实现
    Middleware/    本功能需要的算法和协议
    Service/       电机控制服务
    README.md      复制和接入说明
    KNOWN_ISSUES.md 本功能待修复问题
    CMakeLists.txt 独立构建入口
  LOG_demo/        同样按上述三层组织
  ...其他 demo/
```

纯算法功能没有实际映射或服务代码时，用层内说明文件解释边界。FOC 自带所需增量式 PID，不必再复制 PID_demo。日志只保存在 LOG_demo，由应用决定何时记录，电机算法不直接打印日志。各 demo 的硬件接口具有独立名称，可组合编译。

## 开发 FOC 并使用日志

1. 将 `FOC_demo/`、`LOG_demo/` 复制到目标工程同一父目录。
2. 编译两个目录的 `Middleware/`、`Service/` 下的 `.c`，将其父目录加入头文件搜索路径。
3. 使用 STM32F4 HAL 时，再编译各自 `Mapping/STM32F4/stm32f4_mapping.c`，由目标工程提供 HAL、CMSIS 和配置头文件；也可自行实现 `Mapping/mapping.h` 中的回调。
4. 在板级代码中连接 PWM、采样、角度和 UART，按固定周期更新电机，在主循环或任务中处理日志。

CMake 工程可直接使用两个 demo 自带的构建入口，无需复制本库根目录构建文件：

```cmake
add_subdirectory(FOC_demo)
add_subdirectory(LOG_demo)
target_link_libraries(firmware PRIVATE comdrivers_foc comdrivers_log)
```

具体配置与调用见 [FOC 接入](FOC_demo/README.md)和[日志接入](LOG_demo/README.md)。每个 demo 保留自己的 CMakeLists.txt；根目录不再设置统一构建入口。

## 当前范围

代码来源为 CarMainControlBoard、FOC_EG2134、LVGL-_stm32f407vet6 及本库原有模块。三个源项目只读；使用这些 demo 不需要保留源项目。

本轮已修复各 demo 中可由代码确认的缺陷，并加入回归测试。硬件时序、霍尔零偏、控制参数及传感器标定仍需实板验证。各 demo 的 KNOWN_ISSUES.md 随功能一起复制，保留原问题编号。编译、链接通过不等同于运行或实板验证。

## 组织约定

顶层业务文件夹只保留各个 `*_demo`。说明、接入方法及问题清单放在对应 demo 内，仓库根目录只保留导航说明、Git 忽略规则和历史备份文件。

- Mapping 负责硬件操作接口及平台实现；硬件句柄由调用方提供。
- Middleware 负责算法、数据结构和协议，不依赖 HAL、任务调度或服务层。
- Service 组合本 demo 的中间层与映射接口；不创建全局任务或应用主循环。
- 各实例状态由调用方持有，同一实例的并发访问由集成工程协调；DMA 缓冲在完成通知前必须保持有效。
- 使用含义明确的英文名称和中文接口注释，说明单位、范围、所有权和调用前提。

## 验证及历史资料

11 个 demo 的边界/错误路径回归测试已在本机执行通过，同时通过严格 ARM 编译与链接。HAL/LVGL 适配和全部公共头文件已编译检查，组合链接无重复符号。尚未进行实板验证。

测试位于各 demo 的 Tests 内，生产代码仍仅依赖所属 demo。部分接口增加了长度、时间戳或错误返回值，接入前请阅读对应 README 的“本轮接口迁移”。全库检查入口为 `python FOC_demo/Tests/run_regressions.py --all --source-root ..`，源项目仅读取，产物写入各 demo 的 Tests/build。主机执行需要本地 C 编译器，可通过 `--host-compiler` 指定。

原架构说明、迁移总表、完整问题清单、验证脚本和历史目录备份已收进 [migration_backup.zip](migration_backup.zip)。该文件只用于追溯和恢复，不参与移植或构建；压缩包内文档记录的是整理前路径。

未移入 demo 的源代码问题仍需记录：BUG-028 是未完成观察器的饱和函数边界问题；BUG-029 是源应用中的接收缓冲、任务创建和图形库调用上下文问题。具体触发条件保存在备份的完整问题清单中，未修复也未作为可用功能引入。

本轮逐项验证结果见 [回归验证记录](FOC_demo/Tests/VALIDATION.md)。
