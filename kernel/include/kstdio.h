/*
 * @Description: 内核printf实现
 * @Author: QIUFUYU
 * @Date: 2021-10-03 19:07:57
 * @LastEditTime: 2021-12-19 12:40:23
 */
#ifndef _H_STDIO
#define _H_STDIO

#include"types.h"
typedef char* va_list;
uint32 printf(const char* str, ...);

//此printf_null无实际作用
//一般用于控制是否调试输出
//例如
// #ifndef _debug_
//#define printk printf_null
//#endif
uint32 printf_null(const char* str, ...);
uint32 vsprintf(char* str, const char* format, va_list ap);
void printbins(char *ptr,uint32 len);
uint32 sprintf(char* buf, const char* format, ...);
#define printk printf
#endif