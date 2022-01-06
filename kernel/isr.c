/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-09-16 21:31:48
 * @LastEditTime: 2021-12-06 22:18:03
 */

#include"isr.h"
#include"kio.h"
#include"qstring.h"
#include"kstdio.h"
#include"console.h"
#include"task/kthread.h"

#define EFLAGS_IF   0x00000200       // eflags寄存器中的if位为1
#define GET_EFLAGS(EFLAG_VAR) asm volatile("pushfl; popl %0" : "=g" (EFLAG_VAR))

isr_t interrupt_handlers[256];
void isr_handler(registers_t regs)
{
   uint8 num=regs.int_num&0xff;
   //console_print_dec(num);
   printk("recv a inter!\n");
   if(interrupt_handlers[num]!=0)
   {
      isr_t handle=interrupt_handlers[num];
      handle(&regs);
   }else
   {
      //很快啊，就产生错误了
      intr_disable_loop();
      task_struct_t*cur=running_thread();
      if(cur->pid==0)
      {
      if(regs.int_num==14)
      {
               int page_fault_vaddr = 0; 
      asm ("movl %%cr2, %0" : "=r" (page_fault_vaddr));	  // cr2是存放造成page_fault的地址
      printf("                           page not map:0x%x\n",page_fault_vaddr);
      console_debug("page error!",DEBUG_PANIC);
      }else 
      {
   char num[2]={0};
   console_debug(strcat("recv a INTERRUPT...",myitoa(regs.int_num,num,10)),DEBUG_PANIC);
      }
      }else
      {
         //进程出错
         
         //intr_disable_loop();
         
         console_set_color(BLACK,RED);
         printf("#==========ERROR==========#\n");
         console_set_color(BLACK,LIGHT_BLUE);
         printf("AN Error Occurs In Process!\n");
         printf("task pid:%d name:%s ! err_no:%d\n",cur->pid,cur->name,regs.int_num);
         printf("thread has died!\n");
         console_set_color(BLACK,RED);
         printf("#==========ERROR==========#\n");
         console_set_color(BLACK,WHITE);
         cur->status=TASK_DIED;
         printk("open intr?\n");
         //while(1);
         //intr_enable_loop();
         //printk("open intr!\n");
         //while(1);
         intr_enable();
      }
   }
}
int irq_handler(registers_t regs)
{
   // Send an EOI (end of interrupt) signal to the PICs.
   // If this interrupt involved the slave.
      //printk("recv a inter!\n");

   if (regs.int_num >= 40)
   {
       // Send reset signal to slave.
       outb(0xA0, 0x20);
   }
   // Send reset signal to master. (As well as slave, if necessary).
   outb(0x20, 0x20);

   if (interrupt_handlers[regs.int_num] != 0)
   {
       isr_t handler = interrupt_handlers[regs.int_num];
       return handler(&regs);
   }
   return 0;
}


void register_interrupt_handler(uint8 n, isr_t handler)
{
  interrupt_handlers[n] = handler;
}

enum intr_status intr_get_status(void)
{
  uint32 eflag;
  GET_EFLAGS(eflag);
  return (EFLAGS_IF&eflag)?INTR_ON:INTR_OFF;
}
enum intr_status intr_set_status (enum intr_status st)
{
  return st&INTR_ON?intr_enable():intr_disable();
}
enum intr_status intr_enable (void)
{
   enum intr_status old_status;
   if (INTR_ON == intr_get_status()) {
      old_status = INTR_ON;
      return old_status;
   } else {
      old_status = INTR_OFF;
      asm volatile("sti");	 // 开中断,sti指令将IF位置1
      return old_status;
   }
}
enum intr_status intr_disable (void)
{
   enum intr_status old_status;
   if (INTR_ON == intr_get_status()) {
      old_status = INTR_ON;
      asm volatile("cli" : : : "memory"); // 关中断,cli指令将IF位置0
      return old_status;
   } else {
      old_status = INTR_OFF;
      return old_status;
   }
}
//循环尝试关中断
//直到中断被成功关闭
//注意：此操作只用于应对一些奇怪的bug
void intr_disable_loop()
{
   while (intr_get_status()!=INTR_OFF)
   {
      intr_disable();
   }
   
}
//循环尝试开中断
//直到中断被成功开启
//注意：此操作只用于应对一些奇怪的bug
void intr_enable_loop()
{
      while (intr_get_status()!=INTR_ON)
   {
      intr_enable();
   }
}