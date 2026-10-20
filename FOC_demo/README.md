# FOC_demo

三相电机FOC控制：坐标变换、空间矢量调制、霍尔估算、电流/速度环、双线电机协议。

**独立复制整个本目录即可，无其他demo依赖。** 本目录内部三层为`Mapping/`、`Middleware/`、`Service/`。没有实际代码的层仅用README说明边界，不制造空实现。

## 文件

- `Mapping/mapping.h`
- `Mapping/STM32F4/stm32f4_mapping.c`
- `Mapping/STM32F4/stm32f4_mapping.h`
- `Middleware/coordinate_transform.c`
- `Middleware/coordinate_transform.h`
- `Middleware/hall_estimator.c`
- `Middleware/hall_estimator.h`
- `Middleware/incremental_pid.c`
- `Middleware/incremental_pid.h`
- `Middleware/motor_link_protocol.c`
- `Middleware/motor_link_protocol.h`
- `Middleware/space_vector_pwm.c`
- `Middleware/space_vector_pwm.h`
- `Service/motor_control_service.c`
- `Service/motor_control_service.h`

## 加入工程

1. 复制`FOC_demo/`，保持内部相对路径。
2. 将其`Middleware/`、`Service/`中的`.c`加入编译。
3. 选择映射实现，或按接口连接自己的硬件。不要同时编译两份相同demo。
4. 使用`#include "FOC_demo/Service/...h"`或对应中间层头文件时，将demo所在父目录加入包含路径。
5. CMake项目可使用`add_subdirectory(FOC_demo)`和`target_link_libraries(your_target PRIVATE comdrivers_foc)`。本demo自己的CMakeLists不依赖库根目录文件。

已按问题清单修复可由代码确认的缺陷，并加入边界/错误路径回归测试；仍不是完整烧录工程。

硬件合同使用`FocMapping...`类型，STM32适配使用`FocStm32f4_...`前缀，多个demo可同时链接。

问题跟踪编号：BUG-003、BUG-006、BUG-007、BUG-008、BUG-009、BUG-022、BUG-030。详见本目录[修复记录](KNOWN_ISSUES.md)。

## STM32F4 接入示例

以下片段放在使用者的板级代码中。TIM1、ADC 同步触发、引脚、互补输出、死区及故障关断须由板级工程配置。此初始化函数只绑定接口，不启动 PWM。

```c
#include "FOC_demo/Mapping/STM32F4/stm32f4_mapping.h"
#include "FOC_demo/Service/motor_control_service.h"

extern TIM_HandleTypeDef htim1;
static FocStm32f4ThreePhaseContext motor_pwm = {
    &htim1, TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3
};
static MotorControlService motor;

/* configuration 应先清零，再填写三个 PID 的增益/限幅、
 * 两相采样偏置/增益、modulator.maximum_duty 和独立电流目标限幅 maximum_current_target。
 * 参数由实际电机、采样电路、PWM 周期和控制周期确定。
 */
void BoardMotor_Initialize(const MotorControlService *configuration)
{
    motor = *configuration;
    MotorControl_SetMode(&motor, MOTOR_BRAKE);
    motor.output = FocStm32f4_MakeThreePhasePwm(&motor_pwm);
}

/* 固定电流环周期调用；采样应来自同一控制周期。
 * electrical_angle_degrees 为电角度，支持小数角度。
 */
void BoardMotor_UpdateCurrent(int16_t sample_b, int16_t sample_c,
                              float electrical_angle_degrees)
{
    MotorControl_SetSamples(&motor, sample_b, sample_c);
    motor.electrical_angle_degrees = electrical_angle_degrees;
    MotorControl_UpdateCurrent(&motor);
}
```

板级准备完成后显式调用 `FocStm32f4_StartThreePhasePwm(&motor_pwm)` 并检查返回值；失败时可能已启动部分通道，由板级代码处理停机。通过 MotorControl_SetMode 切换模式后再设置新目标，避免旧目标在模式间残留。速度环需更新 `speed_feedback`，然后按独立固定周期调用 `MotorControl_UpdateSpeed`，不可与电流环并发写同一对象。

每台电机分别分配服务及映射上下文。霍尔估算时间戳必须统一时基；霍尔反馈、模式切换等问题仍见本目录问题清单。上述片段展示接口连接，不是经过电机调试的参数配置。

## 添加日志

同时复制 LOG_demo，并在应用代码中持有 LogService。FOC 不包含日志头文件；应用可在任务中取得一致的电机状态快照后调用 LogService_Print，再周期调用 LogService_Process。不要在电流环中执行阻塞 UART 输出或格式化日志。

## 代码来源

源工程只作为读取来源，本 demo 不依赖这些工程目录。下表路径相对于对应源项目根目录。

| 来源项目 | 源文件 | 当前归属 | 抽取边界 |
| --- | --- | --- | --- |
| CarMainControlBoard | `DriverLayer/Src/FOC_communication.c` | `FOC_demo/Middleware/motor_link_protocol` + GPIO映射 | 抽取32位字编码；双数据线时序需按原协议由板级驱动 |
| FOC_EG2134 | `MiddleLayer/Src/PID.c` | `FOC_demo/Middleware/incremental_pid`；通用版本见 `PID_demo/Middleware/pid_controller` | 增量式PID，与目标库位置式PID分开命名 |
| FOC_EG2134 | `clark_transformation.c`、`park_transformation.c` | `FOC_demo/Middleware/coordinate_transform` | Clarke拼写修正，去掉全局对象和无用HAL依赖 |
| FOC_EG2134 | `svpwm.c` | `FOC_demo/Middleware/space_vector_pwm` | 保留极坐标与直角坐标算法；开环测试时基属于应用 |
| FOC_EG2134 | `DriveLayer/Src/Hall_Sensor.c` | `FOC_demo/Middleware/hall_estimator` | 输入霍尔位图和时间戳；引脚读取留给映射 |
| FOC_EG2134 | `DriveLayer/Src/Motor.c` | `FOC_demo/Service/motor_control_service` | 采样换算、电流环、速度环、三相输出；参数/偏置/定时器/ADC配置由应用设置 |
| FOC_EG2134 | `DriveLayer/Src/DataReception.c` | `FOC_demo/Middleware/motor_link_protocol` + `MotorControl_SetDifferentialTarget` | 字节/位接收状态与双电机目标解耦；完整帧校验及回绕安全超时 |
| FOC_EG2134 | `PWM_EG2134.c` | `FOC_demo/Mapping/STM32F4` | 三相比较、主/互补输出启动；不复制写死TIM1/TIM8和开环测试 |

## 构建映射层

手动集成时，将 Mapping/STM32F4 下的 C 文件加入工程，并提供 HAL、CMSIS、配置头文件及芯片型号宏。使用本 demo 的 CMake 时，在 add_subdirectory 前设置 COMDRIVERS_ENABLE_STM32F4=ON、COMDRIVERS_STM32_INCLUDE_DIRS 为相应头文件目录列表；COMDRIVERS_STM32_DEVICE 默认为 STM32F407xx。目标固件本身也需配置这些外部依赖并链接 HAL 实现。

## 回归测试

`Tests/test_regression.c` 只使用模拟数据及回调，不访问真实外设。将本 demo 的可移植源码与该文件一起编译即可运行。CMake 可设置 `COMDRIVERS_BUILD_TESTS=ON`；主机编译后运行 CTest，ARM 交叉编译只能检查编译/链接。

`Tests/` 不需要复制到固件工程。STM32F4 及 LVGL 适配仍由外部依赖提供，实板结果不包含在本机回归结论内。

## 本轮接口迁移

- Clarke/Park及空间矢量输入输出改为浮点；最终PWM比较值仍为整数。
- HallEstimator的pulse_period为32位无符号周期，direction单独表示方向。初始化后用HallEstimator_ConfigureTimebase设置真实时钟频率和极对数；frequency单位变为机械转数/秒，旧速度目标和PID增益不能直接沿用未定义尺度。
- MotorLink_OnClock新增milliseconds参数；首次有效帧之前HasTimedOut返回true。
- maximum_current_target为必填的独立电流目标上限。切换模式先调用MotorControl_SetMode，再设置新目标。速度环前馈后再次限幅，电流目标也限制在该独立上限内。
- 电流/速度更新入口需串行化；电角零偏和霍尔接线须在目标电机上标定。
