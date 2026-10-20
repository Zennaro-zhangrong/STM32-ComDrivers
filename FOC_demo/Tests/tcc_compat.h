/** @file tcc_compat.h @brief 仅供旧Windows TinyCC运行库的C99格式化语义适配。 */
#ifndef COMDRIVERS_TEST_TCC_COMPAT_H
#define COMDRIVERS_TEST_TCC_COMPAT_H
#include <stdio.h>
#include <stdarg.h>
int TestC99_Vsnprintf(char *buffer, size_t capacity, const char *format, va_list arguments);
#define vsnprintf TestC99_Vsnprintf
#endif
