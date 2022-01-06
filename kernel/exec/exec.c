/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-10-07 08:25:04
 * @LastEditTime: 2022-01-03 15:56:26
 */
#include"exec/exec.h"
#include"fs/hdd.h"
#include"fs/fat32/interface.h"
#include"mem/malloc.h"
#include"qstring.h"
#include"kstdio.h"
#include"mem/memory.h"
#include"exec/elf32.h"
#include"isr.h"
#include"console.h"
#include"task/kthread.h"
#include"task/process.h"
#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))
static bool segment_load(int fd,uint32 offset,uint32 sz,uint32 vaddr)
{
    uint32 vaddr_first_page=vaddr&0xFFFFF000;
    //printf("vaddr:0x%x  ,   first:0x %x\n",vaddr,vaddr_first_page);
    uint32 first_use_in_page=4096-(vaddr&0x00000fff);
    uint32 copied=0;
    if(sz>first_use_in_page)
    {
        //空余部分容不下这个段
        uint32 left=sz-first_use_in_page;
        copied=DIV_ROUND_UP(left,4096)+1;

    }else copied=1;
    uint32 page_idx=0;
    uint32 vaddr_page=vaddr_first_page;
    while (page_idx<copied)
    {
        printf("allocing!!!!!!!!!!!!!!!!!!!!!!!!!!");
        if(!get_a_thread_page(vaddr_page,false))
            return false;
        
        vaddr_page+=4096;
        page_idx++;
    }
    printf("alloc done!\n");
    printf("from%d\n",ata_selected_dev->fs->methods.tell(fd));
    ata_selected_dev->fs->methods.lseek(fd,offset,SEEK_SET);
    printf("to%d\n",ata_selected_dev->fs->methods.tell(fd));
    ata_selected_dev->fs->methods.read(fd,(char*)vaddr,sz);
    return true;
    
}

static int32 load_elf(char *path)
{
    int32 re=-1;
    struct Elf32_Ehdr elf_header;
    struct Elf32_Phdr prog_header;
    memset(&elf_header,0,sizeof(struct Elf32_Ehdr));
    console_clean();
    printk("pause %s\n",path);
    int fd=ata_selected_dev->fs->methods.open(path,"r");
    if(fd<0)
        return -1;
    printf("pause");
    if(ata_selected_dev->fs->methods.read(fd,&elf_header,sizeof(struct Elf32_Ehdr))!=sizeof(struct Elf32_Ehdr))
    {
        printf("%d -> %d\n",ata_selected_dev->fs->methods.read(fd,&elf_header,sizeof(struct Elf32_Ehdr)),sizeof(struct Elf32_Ehdr));
        printf("elf:read error\n");
        re=-1;
        goto done;
    }
    printf("pause");
    printf("e_type:%d ,machine:%d ,ver:%d ,e_phnum:%d ,e_size:%d\n",elf_header.e_type,elf_header.e_machine,elf_header.e_version,elf_header.e_phnum,elf_header.e_phentsize);
    //while(1);
    if(memcmp(elf_header.e_ident,"\177ELF\1\1\1",7)||elf_header.e_type!=2||elf_header.e_machine!=3 \
    ||elf_header.e_version!=1||elf_header.e_phnum>1024|| elf_header.e_phentsize != sizeof(struct Elf32_Phdr))
    {
        printf("elf:header error\n");
        re=-1;
        goto done;
    }
    printf("cmp elf ok!\n");
    Elf32_Off prog_header_offset=elf_header.e_phoff;
    Elf32_Half prog_header_size=elf_header.e_phentsize;
    uint32 prog_idx=0;
    while (prog_idx<elf_header.e_phnum)
    {
        //printf("load header!\n");
        memset(&prog_header,0,prog_header_size);
        ata_selected_dev->fs->methods.lseek(fd,prog_header_offset,SEEK_SET);
    
        if(ata_selected_dev->fs->methods.read(fd,&prog_header,prog_header_size)!=prog_header_size)
        {
            printf("elf:read2 error\n");
            re=-1;
            goto done;
        }
        printf("read 1\n");
        if(prog_header.p_type==PT_LOAD)
        {
            //printf("in PTLOAD\n");
            printf("segment load:0x%x\n",prog_header.p_vaddr);
            if(!segment_load(fd,prog_header.p_offset,prog_header.p_filesz,prog_header.p_vaddr))
            {
                printf("elf:load segment error\n");
                
                re=-1;
                goto done;
            }
            printf("load segment ok!\n");
        }else 
        {
            console_clean();
            printf("----------------------type:%d,alain:%x\n",prog_header.p_type,prog_header.p_align);
            //while(1);
            }
        prog_header_offset+=elf_header.e_phentsize;
        prog_idx++;
    }
    re=elf_header.e_entry;
    
    done:
        ata_selected_dev->fs->methods.close(fd);
        return re;
}
int32 exec_elf32(char *path)
{
    //intr_disable_loop();
    printf("in exec");
    int32 entry_point= load_elf(path);
    printf("e:%d\n",entry_point);
    while(1);
    if(entry_point==-1)return -1;
    task_struct_t *cur=running_thread();
    //console_clean();
    //printf("cur_t:0x%x,name:%s --->name:%s,entry:0x%x\n",cur,cur->name,path,entry_point);
    memcpy(cur->name,path,16);
    //printf("now it is:%s\n",cur->name);
    struct intr_stack *intr_0_stack=(struct intr_stack*)((uint32)cur+0x1000-sizeof(struct intr_stack));
    intr_0_stack->ebx=NULL;
    intr_0_stack->ecx=0;
    intr_0_stack->eip=entry_point;
    intr_0_stack->esp=(void*)0xc0000000;
    int (*f)(void)=entry_point;
    process_do_switch(intr_0_stack,cur);
    //intr_enable_loop();
    return entry_point;
}
