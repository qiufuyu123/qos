/*
 * @Description: 计时器模块
 * @Author: QIUFUYU
 * @Date: 2021-09-17 21:55:52
 * @LastEditTime: 2022-01-03 15:44:16
 */
#include"timer.h"
#include"isr.h"
#include"console.h"
#include"qstring.h"
//#include"task/task.h"
#include"task/kthread.h"
#include"qassert.h"
#include"kio.h"
#include"kstdio.h"
#include"driver/clock.h"
#define CONTROL_PORT 0x43
#define COUNTER0_NO 0
 #define COUNTER_MODE 2
uint32 ticks;
static int HZ;
static void timer_called(registers_t* reg)
{
    //reg.esp=(uint32)schedule((CPU_state_t*)reg.esp);
    //tick++;
    //char num[10]={0};
    //console_debug(myitoa(tick,num,10),DEBUG_INFO);
    //if()
    ticks++;
    //printf("running timer");
    if((ticks % HZ==0)&&clock_is_enabled)
    {
        ClockChangeSystemDate();
    }
    task_struct_t *cur=running_thread();
        if(cur->status==TASK_DIED && (!elem_find(&thread_died_list,&cur->general_tag)))
    {
        list_remove(&cur->general_tag);
        list_append(&thread_died_list,&cur->general_tag);
        
        //printf("!!!!!!!!!!!!!!!!in died!\n");
        schedule();
        
        return;
    }
    //if(cur->stack_magic==0x0)
    //console_clean();
    //printk("ass! %x %s\n",cur->stack_magic,cur->name);
    ASSERT(cur->stack_magic==0x1919);
    
    cur->elapsed_ticks++;
    //ticks++;
    //console_clean();
    //printk("bbbbbbss %d\n",cur->ticks);
    if(cur->ticks==0)
    {
        //printk("ass %d\n",cur->ticks);
        schedule();
    }else cur->ticks--;
}
void init_timer(uint32 hz)
{
    register_interrupt_handler(IRQ0,&timer_called);
    uint32 div=1193180/hz;
   //u32int divisor = 1193180 / frequency;
    HZ=hz;
   // Send the command byte.
   outb(0x43, 0x36);

   // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
   uint8 l = (uint8)(div & 0xFF);
   uint8 h = (uint8)( (div>>8) & 0xFF );

   // Send the frequency divisor.
   outb(0x40, l);
   outb(0x40, h);
}