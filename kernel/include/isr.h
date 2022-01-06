/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-09-16 21:31:59
 * @LastEditTime: 2021-10-04 13:49:43
 */
#ifndef _H_ISR
#define _H_ISR

#include"types.h"
#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47
#define IRQ48 80
enum intr_status {		 // 中断状态
    INTR_OFF,			 // 中断关闭
    INTR_ON		         // 中断打开
};
typedef struct registers
{
    uint32 ds;
    uint32 edi,esi,ebp,esp,ebx,edx,ecx,eax;
    uint32 int_num,error_code;
    uint32 eip,cs,eflags,useresp,ss;
}registers_t;
typedef int (*isr_t)(registers_t*);

void register_interrupt_handler(uint8 n, isr_t handler);


enum intr_status intr_get_status(void);
enum intr_status intr_set_status (enum intr_status);
enum intr_status intr_enable (void);
enum intr_status intr_disable (void);
//循环尝试关中断
//直到中断被成功关闭
//注意：此操作只用于应对一些奇怪的bug
void intr_disable_loop();
//循环尝试开中断
//直到中断被成功开启
//注意：此操作只用于应对一些奇怪的bug
void intr_enable_loop();
#endif