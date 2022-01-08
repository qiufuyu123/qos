/*
 * @Description: 内存管理内核接口
 * @Author: QIUFUYU
 * @Date: 2021-09-25 16:15:42
 * @LastEditTime: 2022-01-08 00:34:38
 */
#include"mem/memory.h"
#include"console.h"
#include"task/sync.h"
#include"qstring.h"
#include"kstdio.h"
#include"qmath.h"
void kernel_phys_end_var();
void kernel_virt_end_var();


void init_memory(multiboot_info_t* mbt) {
    // First init physical memory manager

    // Get kernel end address
    uint32 kernel_phys_end = (uint32) &kernel_phys_end_var;
    uint32 kernel_virt_end = (uint32) &kernel_virt_end_var;

    // Put the physical memory manager at just after the kernel
    uint32 mem_size = 0x400 + mbt->mem_upper;     // = 1MB + Upper memory size
	pm_init(mem_size, (uint32*)kernel_virt_end);

    // Get available physical memory space and mark them all free
    memory_map_t* mmap = (memory_map_t*) (mbt->mmap_addr + VIRTUAL_KERNEL_BASE);
    uint32 page_max=0;
	while((uint32)mmap < mbt->mmap_addr + mbt->mmap_length + VIRTUAL_KERNEL_BASE) {
		if (mmap->type == 1) {
            pm_init_region(mmap->base_addr_low, mmap->length_low);
            page_max+= mmap->length_low/4096;
        }

		mmap = (memory_map_t*) ((uint32)mmap + mmap->size + sizeof(uint32));
	}

    // Also mark from start of memory to end of kernel as taken
	pm_deinit_region(0, kernel_phys_end+4096);  // Size = END_ADDRESS + 1 frame

    // Next (re)setup kernel page directory

    setup_kernel_page_dir();

    // Initialize allocation system, including marking free and available pages

    memset(&active_memory.pages[0], 0, 1024*1024/8);
    // For kernel we will only be using higher half memory
    // So reserve all pages up to the one where kernel is currently at
    //memset((void*)((char*)&active_memory.pages[0]+VIRTUAL_KERNEL_BASE/4096/8), 0xFF,DIV_ROUND_UP(DIV_ROUND_UP((kernel_virt_end-VIRTUAL_KERNEL_BASE),4096),8));
    memset(&active_memory.pages[0] + ((VIRTUAL_KERNEL_BASE >> 22) * 1024)/8, 0xFF, 1024/8);
    // Last page table, used for recursive page directory
    KENEL_START_ID=VIRTUAL_KERNEL_BASE/4096/32+DIV_ROUND_UP(DIV_ROUND_UP((kernel_virt_end-VIRTUAL_KERNEL_BASE),4096),32)+1;
    active_memory.used_page_cnt=DIV_ROUND_UP(DIV_ROUND_UP((kernel_virt_end-VIRTUAL_KERNEL_BASE),4096),32)+1;
    memset(&active_memory.pages[0] + (1023*1024)/8, 0xFF, 1024/8);
    memset(&active_memory.pages[0],0x01,1);
    // Now initialize the block list in the memory
   //active_memory.first_block = 0;
   active_memory.max_page_cnt=page_max;
   for(int i=0;i<11;i++)
   {
       active_memory.blocks[i]=NULL;
   }
    //malloc(4);          // To create the first free block
    lock_init(&lock);
    printk("ruuning 0x%x\n",running_thread());
    running_thread()->all_list_tag.next=0x114514;
    printk("first page:0x%x vir:0x%x tmp:0x%x small_mem 0x\n",malloc_pages(1),kernel_virt_end,&kernel_virt_end);
    //printf("l:%d\n",lock.holder_repeat_nr);
    //while(1) 
    malloc(1);
    init_large_mem(100);
    //while(1 );
    //init_large_mem(100);
    //printk("whats up\n");
    //puts("\n");
    //puts("Kernel end location: 0x"); puthex(kernel_virt_end); puts("\n");
    //puts("Kernel size: "); putdec((kernel_virt_end - VIRTUAL_KERNEL_BASE)/1024); puts(" KB\n");
    //puts("\n");
}