/*
 * @Description: 标准输入输出
 * @Author: QIUFUYU
 * @Date: 2021-10-16 11:19:43
 * @LastEditTime: 2021-10-16 12:10:41
 */
#ifndef _STDIO_H
#define _STDIO_H
#include"stdint.h"
#include"sys/syscall.h"
typedef char* va_list;
uint32_t printf(const char* str, ...);
uint32_t vsprintf(char* str, const char* format, va_list ap);
uint32_t sprintf(char* buf, const char* format, ...);

#endif