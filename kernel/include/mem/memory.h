/*
 * @Description: 内存管理通用头文件
 * @Author: QIUFUYU
 * @Date: 2021-09-24 20:07:37
 * @LastEditTime: 2021-10-02 19:28:17
 */
#ifndef _MEMORY_H
#define _MEMORY_H
#include"mem/malloc.h"
#include"boot/multiboot.h"
#define	 PG_P_1	  1	// 页表项或页目录项存在属性位
#define	 PG_P_0	  0	// 页表项或页目录项存在属性位
#define	 PG_RW_R  0	// R/W 属性位值, 读/执行
#define	 PG_RW_W  2	// R/W 属性位值, 读/写/执行
#define	 PG_US_S  0	// U/S 属性位值, 系统级
#define	 PG_US_U  4	// U/S 属性位值, 用户级
void init_memory(multiboot_info_t* mbt);

// Physical memory management

void pm_init(uint32 mem_size_in_kb, uint32 * bitmap_storage_address);

void pm_init_region(uint32 base_address, uint32 size);

void pm_deinit_region(uint32 base_address, uint32 size);

void* pm_alloc_frame();

void pm_free_frame(void* address);

// 从这儿开始是虚拟内存管理
uint32* get_page_directory() ;
void setup_kernel_page_dir();
bool map_page(void* physical_address, void* virtual_address);
void unmap_page(void* virtual_address);
void* get_physical_addr(void* virtual_address);
#endif