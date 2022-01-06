/*
 * @Description: 内核线程
 * @Author: QIUFUYU
 * @Date: 2021-09-29 21:20:24
 * @LastEditTime: 2022-01-03 15:45:07
 */
#ifndef _H_KTHREAD
#define _H_KTHREAD
#include "types.h"
#include "list.h"
#include"mem/ubitmap.h"
#define FD_LIST_MAX 4096/4
/* 自定义通用函数类型,它将在很多线程函数中做为形参类型 */
typedef void thread_func(void*);

/* 进程或线程的状态 */
enum task_status {
   TASK_RUNNING,
   TASK_READY,
   TASK_BLOCKED,
   TASK_WAITING,
   TASK_HANGING,
   TASK_DIED
};

/*
中断栈intr_stack和线程栈thread_stack都位于线程的内核栈中，也就是都位于PCB的高地址处。
*/

/***********   中断栈intr_stack   ***********
 * 此结构用于中断发生时保护程序(线程或进程)的上下文环境:
 * 进程或线程被外部中断或软中断打断时,会按照此结构压入上下文
 * 寄存器,  intr_exit中的出栈操作是此结构的逆操作
 * 此栈在线程自己的内核栈中位置固定,所在页的最顶端
********************************************/
 struct intr_stack {
    uint32 vec_no;	 // kernel.S 宏VECTOR中push %1压入的中断号
    uint32 edi;
    uint32 esi;
    uint32 ebp;
    uint32 esp_dummy;	 // 虽然pushad把esp也压入,但esp是不断变化的,所以会被popad忽略
    uint32 ebx;
    uint32 edx;
    uint32 ecx;
    uint32 eax;
    uint32 gs;
    uint32 fs;
    uint32 es;
    uint32 ds;

/* 以下由cpu从低特权级进入高特权级时压入 */
    uint32 err_code;		 // err_code会被压入在eip之后
    void (*eip) (void);
    uint32 cs;
    uint32 eflags;
    void* esp;
    uint32 ss;
};

/***********  线程栈thread_stack  ***********
 * 线程自己的栈,用于存储线程中待执行的函数
 * 此结构在线程自己的内核栈中位置不固定,
 * 用在switch_to时保存线程环境。
 * 实际位置取决于实际运行情况。
 ******************************************/
struct thread_stack {
   uint32 ebp;
   uint32 ebx;
   uint32 edi;
   uint32 esi;

/* 线程第一次执行时,eip指向待调用的函数kernel_thread 
其它时候,eip是指向switch_to的返回地址*/
// 线程是使函数单独上处理器运行的机制，因此线程肯定得知道要运行哪个函数，首次执行某个函数时，这个栈就用来保存待运行的函数，其中eip便是该函数的地址
   void (*eip) (thread_func* func, void* func_arg);

/*****   以下仅供第一次被调度上cpu时使用   ****/

/* 参数unused_ret只为占位置充数为返回地址 */
   void (*unused_retaddr);
   thread_func* function;   // 由Kernel_thread所调用的函数名
   void* func_arg;    // 由Kernel_thread所调用的函数所需的参数
};

/* 进程或线程的pcb,程序控制块 */
typedef struct task_struct {
   //int i[sizeof (struct task_struct)];
   uint32* self_kstack;	 // 各内核线程都用自己的内核栈
   enum task_status status; // 记录线程状态
   char name[16]; // 记录任务（线程或进程）的名字
   uint8 priority; // 记录线程优先级
   uint8 ticks;	   // 每次在处理器上执行的时间嘀嗒数
   uint32* open_fd;//打开的fd指针   
/* 此任务自上cpu运行后至今占用了多少cpu嘀嗒数,
 * 也就是此任务执行了多久*/
   uint32 elapsed_ticks;

/* general_tag的作用是用于线程在一般的队列中的结点 */
   struct list_elem general_tag;				    

/* all_list_tag的作用是用于线程队列thread_all_list中的结点 */
   struct list_elem all_list_tag;

   uint32* pgdir;              // 进程自己页表的虚拟地址
   /*
   stack_magic是栈的边界标记，用于检测栈的溢出。咱们PCB和0级栈是在同一个页中，栈位于页的顶端并向下发展，因此担心压栈过程中会把PCB中的信息给覆盖，所以每次在线程或进程调度时要判断是否触及到了进程信息的边界，也就是判断stack_magic的值是否为初始化的内容，stack_magic实际上就是个魔数。
   */
   uvir_addr_t user_vaddr;
   uint32 stack_magic;	 // 用这串数字做栈的边界标记,用于检测栈的溢出
   //int fd_handles[100];
   uint32 pid;
   int32 ret;
}task_struct_t;

//uint32 pid_free[100];
extern struct task_struct* main_thread;    // 主线程PCB
extern struct list thread_ready_list;	    // 就绪队列
extern struct list thread_all_list;	    // 所有任务队列
extern struct list thread_died_list;
void thread_create(struct task_struct* pthread, thread_func function, void* func_arg);
void init_thread(struct task_struct* pthread, char* name, int prio);
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg);
struct task_struct* running_thread(void);
void schedule(void);
void thread_init(void);
void thread_block(enum task_status stat);
void thread_unblock(struct task_struct* pthread);

#endif