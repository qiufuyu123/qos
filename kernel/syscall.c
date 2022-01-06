/*
 * @Description: 系统调用实现
 * @Author: QIUFUYU
 * @Date: 2021-10-03 12:01:46
 * @LastEditTime: 2021-10-16 12:22:02
 */
#include"syscall.h"
#include"isr.h"
#include"console.h"
#include"kstdio.h"
int sys_printf(syscall_args_t args)
{
    console_print_str(args.arg1);
    return 0;
}
syscall_args_t build_args(registers_t reg)
{
    syscall_args_t re;
    re.num=reg.eax;
    re.arg1=reg.ebx;
    re.arg2=reg.ecx;
    re.arg3=reg.edx;
    return re;
}
void syscall_handle(registers_t *reg)
{
    console_print_str("hello syscall!\n");
    syscall_args_t args=build_args(*reg);
    printf("arg_num:%d ,1:%s\n",args.num,args.arg1);
    if(args.num<SYSCALL_NR)
    {
        int(*func)(syscall_args_t a)=syscall_table[args.num];
        uint32 ret= func(args);
        reg->eax=ret;
    }
    return;
}
void init_syscall()
{
    syscall_table[0]=sys_printf;
    register_interrupt_handler(0x80,syscall_handle);
    //初始化系统调用
}
