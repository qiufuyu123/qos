/*
 * @Description: 定义malloc用的函数
 * @Author: QIUFUYU
 * @Date: 2021-09-24 20:06:26
 * @LastEditTime: 2021-12-10 21:24:59
 */
#ifndef _MALLOC_H
#define _MALLOC_H
#include"types.h"

#define VIRTUAL_KERNEL_BASE 0xc0000000
extern uint32 KENEL_START_ID;
typedef struct mem_block
{
    struct mem_block *prev;
    struct mem_block *next;
    uint8 sz;//规格 2 4 8 16 32...
    uint16 used;//已经使用的项
    /* data */
}mem_block_t;
extern struct lock lock;

struct Memory {
    // 4gb的内存需要4096*4096kb: this will take up 128KB of memory itself :O
    uint32 pages[1024*1024/32];
    uint32 max_page_cnt;
    uint32 used_page_cnt;
    mem_block_t *blocks[11];//4 8 16 32 64 128 256 512 1024 2048
};
enum
{
    M_2,
    M_4,
    M_8,
    M_16,
    M_32,
    M_64,
    M_128,
    M_256,
    M_512,
    M_1024,
    M_2048
};
extern struct Memory active_memory;
void *realloc(void *addr,uint32 sz);
void *calloc(int cnt,int len);
bool mem_is_safe(uint32 page_sz,double save_ret);
void* malloc(uint32 size);
uint32 free(void* address);
//初始化大内存分配模块
bool init_large_mem(uint32 len);
//要连续分配sz页
void *get_user_pages_vaddr(uint32 sz);
void *malloc_pages(uint32 sz);
void *get_a_thread_page(uint32 vaddr,bool is_kernel);
//连续释放sz个页
void free_pages(void *vir_addr,uint32 sz);
void* allocate_new_page();

//void* allocate_for_physical_addr(void* phys);


#endif