/*
 * @Description: 环形队列
 * @Author: QIUFUYU
 * @Date: 2021-10-01 21:32:58
 * @LastEditTime: 2021-10-02 10:46:00
 */
#ifndef _H_IOQUEUE
#define _H_IOQUEUE


#include"task/kthread.h"
#include"task/sync.h"
#include"types.h"
typedef void* buf_block;
typedef struct ioqueue
{
    struct lock lock;//锁，用于实现原子操作
    task_struct_t* producer;//生产者
    task_struct_t* consumer;//消费者
    buf_block *bufs;
    uint32 per_sz;
    uint32 sz;
    uint32 head;
    uint32 tail;
}ioqueue_t;


void ioqueue_init(ioqueue_t *ioq,uint32 per_sz,uint32 sz);
bool ioq_full(ioqueue_t *ioq);
bool is_empty(ioqueue_t *ioq);
buf_block ioq_get(ioqueue_t*ioq);
void ioq_put(ioqueue_t*ioq,buf_block val);
#endif