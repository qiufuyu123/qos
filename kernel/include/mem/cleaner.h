/*
 * @Description: 内存清理器
 * @Author: QIUFUYU
 * @Date: 2022-01-02 22:08:48
 * @LastEditTime: 2022-01-02 22:22:09
 */
#ifndef _H_CLEANER
#define _H_CLEANER
#include"types.h"
typedef struct mem_cleaner
{
    uint32 free_addr;//需要被释放的地址
    uint32 pg_cnt;//要释放的page数
}mem_cleaner_t;

mem_cleaner_t* mem_cleaner_new();
void mem_cleaner_clean(mem_cleaner_t* cleaner,bool clean_self);

#endif