/*
 * @Description: 用户虚拟内存管理模块
 * @Author: QIUFUYU
 * @Date: 2021-10-02 18:24:25
 * @LastEditTime: 2021-12-05 19:23:03
 */
#ifndef _H_UBITMAP
#define _H_UBITMAP

#include"mem/malloc.h"
typedef struct ubitmap
{
    uint32 len;
    uint32 *bits;
    uint32 use_page;
}ubitmap_t;
typedef struct uvir_addr
{
    ubitmap_t bitmap;
    uint32 vaddr;
}uvir_addr_t;

//初始化一个vaddr
//(包括malloc部分)
void ubitmap_free(ubitmap_t*map);
ubitmap_t*ubitmap_init(uint32 bytes);
uvir_addr_t uvir_init(uint32 vaddr_start,uint32 page_len);
void ubitmap_set_page(ubitmap_t *bitmap, uint32 page_index);
void ubitmap_unset_page(ubitmap_t *bitmap, uint32 page_index);
int ubitmap_test_page(ubitmap_t *bitmap, uint32 page_index);
int32 ubitmap_scan(uint32 psz,ubitmap_t *bitmap);
#endif