# LOG_demo

独立日志：格式化、字节环形缓冲、UART发送映射。

**独立复制整个本目录即可，无其他demo依赖。** 本目录内部三层为`Mapping/`、`Middleware/`、`Service/`。没有实际代码的层仅用README说明边界，不制造空实现。

## 文件

- `Mapping/mapping.h`
- `Mapping/STM32F4/stm32f4_mapping.c`
- `Mapping/STM32F4/stm32f4_mapping.h`
- `Middleware/ring_buffer.c`
- `Middleware/ring_buffer.h`
- `Service/log_service.c`
- `Service/log_service.h`

## 加入工程

1. 复制`LOG_demo/`，保持内部相对路径。
2. 将其`Middleware/`、`Service/`中的`.c`加入编译。
3. 选择映射实现，或按接口连接自己的硬件。不要同时编译两份相同demo。
4. 使用`#include "LOG_demo/Service/...h"`或对应中间层头文件时，将demo所在父目录加入包含路径。
5. CMake项目可使用`add_subdirectory(LOG_demo)`和`target_link_libraries(your_target PRIVATE comdrivers_log)`。本demo自己的CMakeLists不依赖库根目录文件。

已按问题清单修复可由代码确认的缺陷，并加入边界/错误路径回归测试；仍不是完整烧录工程。

硬件合同使用`LogMapping...`类型，STM32适配使用`LogStm32f4_...`前缀，多个demo可同时链接。

问题跟踪编号：BUG-001、BUG-005。详见本目录[修复记录](KNOWN_ISSUES.md)。

## STM32F4 UART 接入示例

以下片段放在使用者板级工程，UART 必须已初始化；映射上下文和缓冲区须在整个日志服务使用期间保持有效。

```c
#include "LOG_demo/Mapping/STM32F4/stm32f4_mapping.h"
#include "LOG_demo/Service/log_service.h"

extern UART_HandleTypeDef huart2;
static LogStm32f4UartContext log_uart = { &huart2, 10 };
static uint8_t log_storage[512];
static LogService log_service;

void BoardLog_Initialize(void)
{
    LogService_Initialize(&log_service, log_storage, sizeof(log_storage),
                          LogStm32f4_MakeUartWriter(&log_uart));
    RingBuffer_SetCriticalSection(&log_service.buffer,
        (RingBufferCriticalSection){ NULL, LogStm32f4_EnterCritical, LogStm32f4_LeaveCritical });
    LogPrintResult result = LogService_Print(&log_service, "初始化完成\r\n");
    (void)result; /* 应用可根据返回值统计或重试。 */
}

/* 从主循环或任务调用；同一实例的打印与发送需要串行化。 */
void BoardLog_Process(void)
{
    LogService_Process(&log_service);
}
```

当前实现逐字节阻塞输出，timeout_milliseconds 为每次 UART 发送的超时值。过长/队满会整条拒绝并返回原因，失败字节保留重试。跨上下文打印必须设置临界区；Process 只允许单消费者，格式化和发送均不持有临界区。更换平台时实现 LogMappingByteWriter.write 回调即可；无需修改日志服务或环形缓冲。

## 代码来源

源工程只作为读取来源，本 demo 不依赖这些工程目录。下表路径相对于对应源项目根目录。

| 来源项目 | 源文件 | 当前归属 | 抽取边界 |
| --- | --- | --- | --- |
| CarMainControlBoard | `Middleware/RingBuffer.c/.h` | `LOG_demo/Middleware/ring_buffer` | 合并重复实现，保留count算法 |
| CarMainControlBoard | `DriverLayer/Src/log.c` | `LOG_demo/Service` + `LOG_demo/Mapping/STM32F4` | 格式化/缓冲属于服务；HAL发送属于映射。RTOS DMA日志任务不复制 |
| FOC_EG2134 | `RingBuffer.c`、`UART2.c` | LOG_demo 的缓冲区和日志服务 | 不再重复复制 |
| LVGL-_stm32f407vet6 | `Hardware/UART.c`、`Middleware/RingBuffer.c` | LOG_demo 的日志/缓冲区 | 合并同功能模块 |
| 目标库原有文件 | 三份`Middleware/ring_buffer` | 一份`LOG_demo/Middleware` | 归属调整 |

## 构建映射层

手动集成时，将 Mapping/STM32F4 下的 C 文件加入工程，并提供 HAL、CMSIS、配置头文件及芯片型号宏。使用本 demo 的 CMake 时，在 add_subdirectory 前设置 COMDRIVERS_ENABLE_STM32F4=ON、COMDRIVERS_STM32_INCLUDE_DIRS 为相应头文件目录列表；COMDRIVERS_STM32_DEVICE 默认为 STM32F407xx。目标固件本身也需配置这些外部依赖并链接 HAL 实现。

## 回归测试

`Tests/test_regression.c` 只使用模拟数据及回调，不访问真实外设。将本 demo 的可移植源码与该文件一起编译即可运行。CMake 可设置 `COMDRIVERS_BUILD_TESTS=ON`；主机编译后运行 CTest，ARM 交叉编译只能检查编译/链接。

`Tests/` 不需要复制到固件工程。STM32F4 及 LVGL 适配仍由外部依赖提供，实板结果不包含在本机回归结论内。

## 本轮接口迁移

- LogService_Print返回LogPrintResult；超过255字节或空间不足时整条拒绝，不输出截断的半条消息。
- 跨任务/中断使用前调用RingBuffer_SetCriticalSection；STM32F4可注入LogStm32f4_EnterCritical/LeaveCritical，保存并恢复PRIMASK。默认仍是单上下文模式，不可把未配置回调的对象并发使用。
- Process只允许一个消费者；失败字节保留，返回值现在表示成功发送数。底层非OK必须表示未发送，若硬件错误结果无法确认是否已经送出，仍可能存在重复交付，需要上层协议处理。
- rejected_messages和send_failures可用于诊断；读取并发统计时使用同一临界区。
