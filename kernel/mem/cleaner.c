/*
 * @Description: 内存清理器
 * @Author: QIUFUYU
 * @Date: 2022-01-02 22:08:33
 * @LastEditTime: 2022-01-02 22:22:57
 */
#include"mem/cleaner.h"
#include"mem/malloc.h"
#include"kstdio.h"
mem_cleaner_t* mem_cleaner_new()
{
    mem_cleaner_t*re =calloc(1,sizeof(mem_cleaner_t));
    return re;
}
void mem_cleaner_clean(mem_cleaner_t* cleaner,bool clean_self)
{
    if(!cleaner->free_addr)
    {
        printk("nothing to clean\n");
        if(clean_self)free(cleaner);
        return;
    }
    else
    {
        if(cleaner->pg_cnt)
        {
            free_pages(cleaner->free_addr,cleaner->pg_cnt);
            printk("clean page :%d",cleaner->pg_cnt);
        }else
        {
            printk("clean simple mem\n");
            free(cleaner->free_addr);
        }
        if(clean_self) free(cleaner);
    }
}