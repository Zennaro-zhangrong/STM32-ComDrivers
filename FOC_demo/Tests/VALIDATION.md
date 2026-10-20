# 本轮验证记录

全部11个demo的回归测试已在本机执行通过，独立复制的FOC+LOG组合也已运行通过，共12个运行入口。

- ARM GCC 15.2.1：C11、Cortex-M4F、硬浮点、Wall/Wextra/Wpedantic/Werror；86个严格编译检查、13次目标链接/组合检查全部通过。
- 可选STM32F4与LVGL8适配已编译；仅借用源工程现有头文件，源工程不作修改。
- 所有生产头文件可一起包含，生产对象组合链接没有重复符号；FOC+LOG在仅包含两者源码的独立副本中验证。
- Windows TinyCC 0.9.27：11个demo测试与FOC+LOG组合实际执行，所有断言通过。旧MSVCRT适配说明见[测试说明](README.md)。
- FOC自带CMake的回归目标也已完成ARM静态库及测试程序链接。

详细命令输出位于被忽略的 `build/regression_report.json`。交叉链接使用nosys系统调用桩，其系统调用警告不表示生成了完整板级固件。

本机测试未覆盖真实中断抢占、总线信号、DMA压力、电角标定、无线通信距离及电机闭环运行。BUG-021的实际灯带复位时间、BUG-030的GPIO/CE波形仍需目标板验证；其他板级参数要求见各demo的KNOWN_ISSUES.md和README接口迁移说明。
