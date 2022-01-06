/*
 * @Description: kernel main
 * @Author: QIUFUYU
 * @Date: 2021-09-09 21:11:53
 * @LastEditTime: 2022-01-03 15:53:21
 */
#include"console.h"
#include"gdt.h"
#include"timer.h"
#include"mem/memory.h"
#include"task/kthread.h"
#include"isr.h"

#include"keyboard.h"
#include"task/process.h"
#include"test/edit.h"
#include"syscall.h"
#include"kstdio.h"
//#include"fs/fat32.h"
//#include"mem/page.h"
#include"fs/hdd.h"
#include"fs/fat32/fat32.h"
#include"fs/qufs/fs.h"
#include"fs/fat32/interface.h"
#include"exec/exec.h"
#include"dev/devfs.h"
#include"driver/clock.h"
#include"fs/fd.h"
void k_thread_a(void*);
int u_proc_a(void);
#define TEST_DEV
#ifdef TEST_DEV
int dev_open(dev_module_t *dev)
{
    printf("dev_open!\n");
}
int dev_close(void *dev)
{
    printf("dev_close!\n");
}
#endif


void test()
{
    char *a=malloc(3);
    //printf("a:%x\n",&a[0]);
    fs_read(0,3,a);
    printf("a:%s",a);
}


#define VIRTUAL_KERNEL_BASE 0xC0000000
void kernel_main(multiboot_info_t *mbt)
{
    intr_disable();
    console_pre_init();
    //此时由于内存管理模块还没写好
    //所以console不启用双缓冲
    console_debug("QOS booting...",DEBUG_INFO);
    init_gdt();
    console_debug("Load GDT...",DEBUG_OK);
    console_print_str("code seg: 0x");
    console_print_hex(gdt_get_segment(1));
    console_print_str("\n");
    init_idt();
    console_debug("Load IDT...",DEBUG_OK);
    
    init_memory(mbt);
    console_debug("Setup Memory...",DEBUG_OK);
    //malloc(2);
    //内存管理写好后
    //开始启用双缓冲
    console_init();
    console_clean();
    thread_init();
    init_syscall();
    //fatInit();
    
    console_debug("Setup Kernel Thread... ",DEBUG_OK);


    ata_init();
    ata_set_selected_dev(get_ata_device("/dev/hda"));
    fs_init_fdmgr(1000);

    
    
    
    //init_devfs();
    //init_vfs();
    //console_clean();
    //int fdd= devfs_add_dev(devfs_create_dev("dev0","/dev0",dev_open,dev_close,NULL,NULL,NULL));
    //devfs_open(devfs_find_by_fname("/dev0"));
    //uint32 sz= get_ata_device("/dev/hda")->size;
    //printk("disk size:%d Mb\n",sz*512/1024/1024);
    //int fd=fs_open("/")
    //while(1);
    //print_directory(fs,&dir);
    //struct dir_entry *d1=get_dir_entry(&dir,"d1");
    /*if(d1!=NULL)
    {
        //printf("\nfound direcotry:d1!\n");
        //populate_dir(fs,&dir,d1->first_cluster);
        //print_directory(fs,&dir);
    }*/
    //console_clean();

    //char *buf=malloc(100);
    //fs_lseek(fd,5,SEEK_SET);
    //fs_read(fd,100,buf);
    //console_clean();
    //printf("txt:%s\n",buf);
    //printf("root:0x%x\n",root_dir);
    //while(1);
    //int(*func)()= exec_bin("/d1/test.bin");
    //printf("re:%d\n",func());
    //while(1)
    
    thread_start("kernel_main",31,k_thread_a,"argB ");
    task_struct_t*u1= process_execute(u_proc_a,"u1");
    //uint32 upid=u1->pid;
    f32 *fat_fs= makeFilesystem("fat32");
    printf("fs:%x\n",fat_fs);
    fs_interface_init(fat_fs);
    init_clock();
    init_timer(1000);
    intr_enable();
    
    init_keyboard();
    
    

    while(1)
    {
        //printf("aaa");
    }
    //printf("\n");
    //int fd=sys_open("/d1/new2.txt","r+");
    //printf("fd:%d\n",fd);
    //printf("out:%s\n",l);
    // /printf("write ok! %s\n",r);
    //sys_read(0,0,0);
    //test();
        char *a=malloc(3);
    //printf("a:%x\n",&a[0]);
    //fs_read(0,3,a);
    //printf("a:%s",a);
    //test();
        
    //qufs_desc*qfs= qufs_mkfs(get_ata_device("/dev/hdb"));
    //printf("free:%d\n",qu_bitmap_getfree(qfs,1));
    console_clean();
    ata_device_t*dev=get_ata_device("/dev/hda");
    printk("dev:0x%x\n",dev);
    ata_set_selected_dev(dev);
    qufs_desc_t*fs= init_qufs();
    
    while(1);
    struct SystemDate *date=calloc(1,sizeof(struct SystemDate));
    while(1)
    {
        console_clean();
        uint32_t bin=DATE2BIN(systemDate);
        BIN2DATE(date,bin);
        printf("%d-%d-%d %d-%d-%d",date->year,date->month,date->day,date->hour,date->minute,date->second);
    }
    while (1)
    {
        //printf("a");
        /* code */
        
    }
    
    
}
int u_proc_a(void)
{
    //uint32 t1=1/0;
    //printf("cur:0x%x\n",running_thread()->stack_magic);
    //__syscall1(0,"abc");
    //console_clean();
    //exec_elf32("/d1/test.elf");
    //printf("this is a child thread!\n");
    //fs_print_root();
    exec_elf32("/d1/test.elf");
    printf("exe exit\n");
    while(1);
    return 1;
}
void k_thread_a(void*args)
{
    //return;
    printf("cur:0x%x\n",running_thread());
    //新的kernel进程
    while (1)
    {
        char a=get_key();
        if(a!=0)
            put_char(a,BLACK,WHITE);
        
        /* code */
    }
    
    
}