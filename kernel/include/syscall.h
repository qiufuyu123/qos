/*
 * @Description: 系统调用实现
 * @Author: QIUFUYU
 * @Date: 2021-10-03 12:02:05
 * @LastEditTime: 2021-10-09 20:06:15
 */
#ifndef _H_SYSCALL
#define _H_SYSCALL

#include"types.h"
#define SYS_PRINTF 0
#define SYSCALL_NR 32

typedef struct syscall_args
{
    uint32 num;
    uint32 arg1;
    uint32 arg2;
    uint32 arg3;
}syscall_args_t;
typedef int call_func(syscall_args_t args) ;
void* syscall_table[SYSCALL_NR];
void init_syscall();
#endif