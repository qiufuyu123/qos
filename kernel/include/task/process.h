/*
 * @Description: 用户进程实现
 * @Author: QIUFUYU
 * @Date: 2021-10-02 19:05:51
 * @LastEditTime: 2021-11-29 20:33:28
 */
#ifndef _H_PROCESS
#define _H_PROCESS

#include"task/kthread.h"
#define EFLAGS_MBS	(1 << 1)	// 此项必须要设置
#define EFLAGS_IF_1	(1 << 9)	// if为1,开中断
#define EFLAGS_IF_0	0		// if为0,关中断
#define EFLAGS_IOPL_3	(3 << 12)	// IOPL3,用于测试用户程序在非系统调用下进行IO
#define EFLAGS_IOPL_0	(0 << 12)	// IOPL0
#define default_prio 21
#define USER_STACK3_VADDR  (0xc0000000 - 0x1000)
#define USER_VADDR_START 0x8040000
extern ubitmap_t *pid_bitmap;
task_struct_t* process_execute(void* filename, char* name);
void start_process(void* filename_);
void process_activate(task_struct_t* p_thread);
void page_dir_activate(task_struct_t* p_thread);
uint32* create_page_dir(void);
void create_user_vaddr_bitmap(task_struct_t* user_prog);

//将指定stack替换当前程序的stack
//用于exec 和 用户程序执行的过程中
//注意，此函数不能单独使用
//返回:返回值
int process_do_switch(struct intr_stack *proc_stack,task_struct_t *task);

//等待进程结束并获取返回值
//通常用于父进程等待子进程
//同时，此函数会调用destroy_thread来释放进程
int wait_to_died(uint32 pid);


//根据pid找进程
task_struct_t *get_task_by_pid(uint32 pid);
#endif