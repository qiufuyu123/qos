/*
 * @Description: 用户虚拟内存管理模块
 * @Author: QIUFUYU
 * @Date: 2021-10-02 18:24:09
 * @LastEditTime: 2021-12-05 19:23:13
 */
#include"console.h"
#include"mem/ubitmap.h"
#include"kstdio.h"
#include"qmath.h"
void ubitmap_set_page(ubitmap_t *bitmap, uint32 page_index) {
    bitmap->bits[page_index/32] |= (1 << (page_index % 32));
}

void ubitmap_unset_page(ubitmap_t *bitmap, uint32 page_index) {
    bitmap->bits[page_index/32] &= ~(1 << (page_index % 32));
}

int ubitmap_test_page(ubitmap_t *bitmap, uint32 page_index) {
    return bitmap->bits[page_index/32] & (1 << (page_index % 32));
}

uvir_addr_t uvir_init(uint32 vaddr_start,uint32 page_len)
{
    uvir_addr_t re;
    re.bitmap.len=page_len/4096;
    re.bitmap.bits=malloc_pages(page_len/32/4096);
    //所有Bits都由page来分配
    re.vaddr=vaddr_start;
    return re;
}
void ubitmap_free(ubitmap_t*map)
{
    free_pages(map->bits,map->use_page);
    free(map);
}
ubitmap_t*ubitmap_init(uint32 bytes)
{
    ubitmap_t*re=malloc(sizeof(ubitmap_t));
    if(!re)return NULL;
    re->bits=malloc_pages(DIV_ROUND_UP(bytes,4096));
    if(!re->bits)
    {
        free(re);
        return NULL;
    }
    re->use_page=DIV_ROUND_UP(bytes,4096);
    re->len=bytes*8;
    return re;
}
int32 ubitmap_scan(uint32 psz,ubitmap_t *bitmap)
{
    uint32 i, j;
    int32 cnt=0,lst=-1,fst=0;
    uint32 num_pages = 1024*1024;
    uint32 start=0;
    //TODO: 没有实现向上取整
    for (i=0; i<bitmap->len/32; ++i) {
        if (bitmap->bits[i] != 0xFFFFFFFF) {
            for (j=0; j<32; ++j) {
                if (!(bitmap->bits[i] & (1 << j))) {
                    if(lst==-1)
                    { 
                        lst=i*32 + j;
                        fst=lst;
                        cnt=1;
                        //printk("fst:%d lst:%d\n",fst,lst); 
                    }
                    else if(i*32 + j-lst==1)
                    {
                        lst=i*32 + j;
                        cnt++;
                    }else 
                    {
                        //printk("not linked:%d\n",i*32 + j-lst);
                        lst=-1;
                    }
                    if(cnt==psz)
                    {
                        printk("fst found!\n");
                        return fst;
                    }
                }
            }
        }
    }
    printk("not found!!!!!!!!!!!!!!!!!!!!!!\n");
    return -1;
}