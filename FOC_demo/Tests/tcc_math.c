/** @file tcc_math.c
 * @brief 仅供Windows TinyCC回归测试：旧MSVCRT没有这三个C99浮点导出。
 * @note 用系统双精度数学函数计算后转换为float；固件和其他主机编译器不链接此文件。
 */
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
float fmodf(float value, float divisor) { return (float)fmod(value, divisor); }
float sinf(float value) { return (float)sin(value); }
float cosf(float value) { return (float)cos(value); }
int TestC99_Vsnprintf(char *buffer, size_t capacity, const char *format, va_list arguments)
{
    va_list copy;
    va_copy(copy, arguments);
    int required = _vscprintf(format, copy);
    va_end(copy);
    if (required < 0) return required;
    if (capacity != 0) {
        _vsnprintf(buffer, capacity, format, arguments);
        buffer[capacity - 1] = '\0';
    }
    return required;
}
