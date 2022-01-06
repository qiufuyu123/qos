/*
 * @Description: 用户系统调用接口
 * @Author: QIUFUYU
 * @Date: 2021-10-16 11:14:51
 * @LastEditTime: 2021-10-16 11:52:19
 */
#ifndef _SYSCALL_H
#define _SYSCALL_H
extern int __syscall0(int num);
extern int __syscall1(int num,int a1);
extern int __syscall2(int num,int a1,int a2);
extern int __syscall3(int num,int a1,int a2,int a3);
#define _syscall0_(num) __syscall0 
#define _syscall1_(num,arg1) __syscall1
#define _syscall2_(num,arg1,arg2) __syscall2
#define _syscall3_(num,arg1,arg2,arg3) __syscall3 
#endif