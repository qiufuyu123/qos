/*
 * @Description: 环形队列
 * @Author: QIUFUYU
 * @Date: 2021-10-01 21:32:40
 * @LastEditTime: 2021-10-10 12:19:38
 */
#include"ioqueue.h"
#include"mem/malloc.h"
#include"qassert.h"
#include"isr.h"
#include"qstring.h"
void ioqueue_init(ioqueue_t *ioq,uint32 per_sz,uint32 sz)
{
    lock_init(&ioq->lock);
    ioq->producer=ioq->consumer=NULL;
    ioq->per_sz=per_sz;
    ioq->sz=sz;
    ioq->head=ioq->tail=0;
    ioq->bufs=malloc(sizeof(buf_block)*sz);
    if(ioq->bufs==NULL)
        return;
    for(int i=0;i<sz;i++)
    {
        ioq->bufs[i]=malloc(per_sz);
    }
}
static uint32 next_pos(uint32 pos,ioqueue_t *ioq)
{
    return (pos+1)%ioq->sz;
}
bool ioq_full(ioqueue_t *ioq)
{
ASSERT(intr_get_status()==INTR_OFF);
return next_pos(ioq->head,ioq)==ioq->tail;
}
bool is_empty(ioqueue_t *ioq)
{
    ASSERT(intr_get_status()==INTR_OFF);
    return ioq->head==ioq->tail;
}
static void ioq_wait(struct task_struct** waiter) {
   ASSERT(*waiter == NULL && waiter != NULL);
   *waiter = running_thread(); // 赋值
   thread_block(TASK_BLOCKED);
}
static void wakeup(struct task_struct** waiter) {
   ASSERT(*waiter != NULL);
   thread_unblock(*waiter); // 唤醒*waiter（生产者或消费者），随后将*waiter置空
   *waiter = NULL;
}
buf_block ioq_get(ioqueue_t*ioq)
{
    ASSERT(intr_get_status()==INTR_OFF);
    while (is_empty(ioq))
    {
        lock_acquire(&ioq->lock);
        ioq_wait(&ioq->consumer);
        lock_release(&ioq->lock);
    }
    //进程运行到这里则说明有生产者产生数据
    char *re=malloc(ioq->per_sz);
    if(re==NULL)console_debug("FAIL alloc ioq mem!\n",DEBUG_PANIC);
    buf_block buf=ioq->bufs[ioq->tail];
    memcpy(re,buf,ioq->per_sz);
    memset(buf,0,ioq->per_sz);
    ioq->tail=next_pos(ioq->tail,ioq);
    if(ioq->producer!=NULL)
    {
        wakeup(ioq->producer);
    }
    return re;
    
}
void ioq_put(ioqueue_t*ioq,buf_block val)
{
    ASSERT(intr_get_status()==INTR_OFF);
    while (ioq_full(ioq))
    {
        lock_acquire(&ioq->lock);
        ioq_wait(&ioq->producer);
        lock_release(&ioq->lock);
    }
    memcpy(ioq->bufs[ioq->tail],val,ioq->per_sz);
    ioq->head=next_pos(ioq->head,ioq);
    if(ioq->consumer!=NULL)
    {
        wakeup(&ioq->consumer);
    }

    
}