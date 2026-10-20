# 回归测试

本目录保存 FOC 回归及仓库全库检查入口。复制功能到固件时不需要 Tests。

```powershell
python FOC_demo/Tests/run_regressions.py --all --source-root ..
```

`--all` 检查所有已有 demo；省略时只检查 FOC。ARM GCC 必须在 PATH 中；`--source-root` 可选，仅借用三个原项目中的 HAL/LVGL 头文件，既不修改也不复制它们。产物分别在各 demo 的 `Tests/build`。

`--host-compiler` 指定本机 GCC、Clang 或 TinyCC。没有主机编译器时只做 ARM 编译/链接，脚本会明确标记未执行；不会自动安装软件。也可以使用各 demo 自带 CMake 的 `COMDRIVERS_BUILD_TESTS=ON`，在主机工具链下运行 CTest。

本次使用 ARM GCC 15.2.1 做 C11、Cortex-M4F、硬浮点、`-Wall -Wextra -Wpedantic -Werror` 检查，并使用 TinyCC 0.9.27 在 Windows 运行测试。TinyCC 使用的旧 MSVCRT 缺少部分 C99 浮点导出，且截断时的 vsnprintf 返回值不同；tcc_math.c 和 tcc_compat.h 仅在该主机组合中补齐这些运行库语义，不参与 ARM 固件构建。编译器本体放在被 Git 忽略的 build 中，不是库依赖。

测试覆盖模拟输入、故障返回、状态推进、内存边界、数值限幅及时间回绕。物理引脚极性、实际 PWM/CE/DMA 波形、抢占压力、电池模型及电机控制参数仍需目标板验证。
