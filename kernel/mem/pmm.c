/*
 * @Description: 物理内存管理模块，即bitmap
 * @Author: QIUFUYU
 * @Date: 2021-09-24 20:13:27
 * @LastEditTime: 2021-09-25 11:24:45
 */
#include"mem/memory.h"
#include"types.h"
#include"qstring.h"
static uint32 num_frames; //bitmap个数
static uint32 used_frames; //使用的个数
static uint32 * frames; //bitmap起始地址


//inline关键词注
//内联，将程序中所有使用此函数的地方替换成函数体
//类似于#define宏替换
//而不是用call的方法调用（这样会消耗栈空间)
static inline void set_frame(uint32 bit) {
    frames[bit/32] |= (1 << (bit % 32));
}

static inline void unset_frame(uint32 bit) {
    frames[bit/32] &= ~(1 << (bit % 32));
}

static inline int test_frame(uint32 bit) {
    return frames[bit/32] & (1 << (bit % 32));
}
static int get_first_free_frame() {
    uint32 i, j;
    for (i=0; i<num_frames/32; ++i) {
        if (frames[i] != 0xFFFFFFFF) {
            for (j=0; j<32; ++j) {
                if (!(frames[i] & (1 << j))) {
                    return i*32+j;
                }
            }
        }
    }
    return -1;
}
//初始化
void pm_init(uint32 mem_sz_in_kb,uint32 *bitmap_addr)
{
    used_frames=num_frames=mem_sz_in_kb/4;
    //因为一个Page是4kb
    //所以我们将内存大小（kb)除以4
    frames=bitmap_addr;
    memset((char*)frames,0xff,sizeof(uint32)*num_frames/8);
    //注意
    //我们bitmap的布局方式为
    // 1：空闲 0：占用
    //used_frames随页开辟而增加，当used_frame=num_frame时表示
    //无可用页，所以可用页=used_frame-num_frame
    //by qiufuyu
    
}
void pm_init_region(uint32 base_address, uint32 size)
{
    unsigned a, b;
    for (a=base_address/4096, b=size/4096; b>0; --b) {
        unset_frame(a++);
        used_frames++;
    }
    set_frame(0); 													// 0x0 address is always invalid
}

void pm_deinit_region(uint32 base_address, uint32 size)
{
    unsigned a, b;
    for (a=base_address/4096, b=size/4096; b>0; --b) {
        set_frame(a++);
        used_frames--;
    }
}

void* pm_alloc_frame()
{
    if (used_frames == num_frames)
        return 0;
    int frame = get_first_free_frame();
    if (frame == -1)
        return 0;
    set_frame(frame);
    used_frames++;

    return (void*)(frame*4096);
}

void pm_free_frame(void* address)
{
    uint32 frame = (uint32)address / 4096;
    if (!test_frame(frame))
        return;
    unset_frame(frame);
    used_frames--;
}