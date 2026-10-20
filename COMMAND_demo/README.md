# COMMAND_demo

命令派发、PWM命令服务以及双缓冲SPI帧接收器。

**独立复制整个本目录即可，无其他demo依赖。** 本目录内部三层为`Mapping/`、`Middleware/`、`Service/`。没有实际代码的层仅用README说明边界，不制造空实现。

## 文件

- `Middleware/command_dispatcher.c`
- `Middleware/command_dispatcher.h`
- `Middleware/spi_frame_receiver.c`
- `Middleware/spi_frame_receiver.h`
- `Service/pwm_command_service.c`
- `Service/pwm_command_service.h`

## 加入工程

1. 复制`COMMAND_demo/`，保持内部相对路径。
2. 将其`Middleware/`、`Service/`中的`.c`加入编译。
3. 选择映射实现，或按接口连接自己的硬件。不要同时编译两份相同demo。
4. 使用`#include "COMMAND_demo/Service/...h"`或对应中间层头文件时，将demo所在父目录加入包含路径。
5. CMake项目可使用`add_subdirectory(COMMAND_demo)`和`target_link_libraries(your_target PRIVATE comdrivers_command)`。本demo自己的CMakeLists不依赖库根目录文件。

已按问题清单修复可由代码确认的缺陷，并加入边界/错误路径回归测试；仍不是完整烧录工程。

问题跟踪编号：BUG-004、BUG-027。详见本目录[修复记录](KNOWN_ISSUES.md)。

## 代码来源

源工程只作为读取来源，本 demo 不依赖这些工程目录。下表路径相对于对应源项目根目录。

| 来源项目 | 源文件 | 当前归属 | 抽取边界 |
| --- | --- | --- | --- |
| CarMainControlBoard | `CMD_Layer` | `COMMAND_demo/Middleware/command_dispatcher` 的表/回调模式 | 源命令函数为空，不虚构停车等实现 |
| 目标库原有文件 | `FOC_demo/Application/spi_cmd_table` | `COMMAND_demo/Service/pwm_command_service` | 归属调整 |
| 目标库原有文件 | `FOC_demo/Mapping/mapping_spi` | `COMMAND_demo/Middleware/spi_frame_receiver`，使用显式字节提交和帧交接 | 归属调整 |

## 回归测试

`Tests/test_regression.c` 只使用模拟数据及回调，不访问真实外设。将本 demo 的可移植源码与该文件一起编译即可运行。CMake 可设置 `COMDRIVERS_BUILD_TESTS=ON`；主机编译后运行 CTest，ARM 交叉编译只能检查编译/链接。

`Tests/` 不需要复制到固件工程。STM32F4 及 LVGL 适配仍由外部依赖提供，实板结果不包含在本机回归结论内。

## 本轮接口迁移

- CommandDispatcher_Run只有实际调用有效handler才返回1。
- SPI接收先Initialize；NextAddress申请字节地址，硬件确实收到该字节后调用CommitByte。不得在申请地址时提前提交。
- Acquire取得完整帧，消费后Release；Abort丢弃半帧前必须先停止DMA。所有入口由应用串行化，超时/片选结束由板级触发Abort。
- 旧帧没有同步标志；错误长度后必须从已知帧边界重新接收，不保证从任意字节流自动恢复。
