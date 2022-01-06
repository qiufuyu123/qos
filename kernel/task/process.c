/*
 * @Description: 用户进程
 * @Author: QIUFUYU
 * @Date: 2021-10-02 19:05:35
 * @LastEditTime: 2022-01-03 15:51:33
 */
#include"task/process.h"
#include"gdt.h"
#include"mem/malloc.h"
#include"mem/memory.h"
#include"console.h"
#include"qstring.h"
#include"gdt.h"
#include"isr.h"
#include"qassert.h"
#include"kstdio.h"
ubitmap_t *pid_bitmap;
#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))
extern void intr_exit();
void update_tss_esp(struct task_struct* pthread) {
    tss_entry.esp0 = (uint32*)((uint32)pthread + 4096);
}
void release_pid(uint32  pid)
{
    ubitmap_unset_page(pid_bitmap,pid);
}
task_struct_t* process_execute(void* filename, char* name)
{
   /* pcb内核的数据结构,由内核来维护进程信息,因此要在内核内存池中申请 */
   struct task_struct* thread = malloc_pages(1);
   
   //while(1);
   //console_print_str("ut:0x");
   //console_print_hex(thread);
   //console_print_str("\n");
   init_thread(thread, name, default_prio); 
   create_user_vaddr_bitmap(thread);
   //console_print_str("bits:0x");
   //console_print_hex(thread->user_vaddr.bitmap.bits);
   //console_print_str("\n");
   thread_create(thread, start_process, filename);
   thread->pgdir = create_page_dir();
   //printf("pdir:0x%x\n",thread->pgdir);
    //console_print_str("pdir:0x");
   //console_print_hex(thread->pgdir);
   //console_print_str("\n");
   //enum intr_status old_status = intr_disable();
   intr_disable_loop();
   ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
   list_append(&thread_ready_list, &thread->general_tag);

   ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));
   list_append(&thread_all_list, &thread->all_list_tag);
   //intr_set_status(old_status);
//   console_print_str("uthread alloc ok!\n");
   intr_enable_loop();
   return thread;
}
void start_process(void* filename_)
{
    
    void *func=filename_;
    task_struct_t *cur=running_thread();
    printf("in cur:0x%s\n",cur->name);
    cur->self_kstack+=sizeof(struct thread_stack);
    struct intr_stack *proc_stack=cur->self_kstack;
    
       proc_stack->edi = proc_stack->esi = proc_stack->ebp = proc_stack->esp_dummy = 0;
   proc_stack->ebx = proc_stack->edx = proc_stack->ecx = proc_stack->eax = 0;
   proc_stack->gs = 0;		 // 用户态用不上,直接初始为0
   proc_stack->ds = proc_stack->es = proc_stack->fs = gdt_get_segment(4);
   proc_stack->eip = func;	 // 待执行的用户程序地址
   proc_stack->cs = gdt_get_segment(3);
   proc_stack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
   console_print_str("in sc!\n");
   proc_stack->esp = (void*)((uint32)get_a_thread_page(USER_STACK3_VADDR,false) + 4096) ;
   proc_stack->ss = gdt_get_segment(4); 
   console_print_str("in sc!\n");



   //md,这里esp传的是栈顶pcb指针
   //我之前一直把proc_stack.esp传进去了....
   int32 ret= process_do_switch(proc_stack,cur);
   //正常情况下不会执行到这
   //除非用户程序返回
   //list_remove(&cur->general_tag);
   while (1)
   {
       /* code */
   }
   
}
void process_activate(task_struct_t* p_thread)
{
    ASSERT(p_thread!=NULL);
    page_dir_activate(p_thread);
    if(p_thread->pgdir!=NULL)
        update_tss_esp(p_thread);
}
void page_dir_activate(task_struct_t* p_thread)
{
    uint32 pdir_phy=get_physical_addr((uint32)get_page_directory());
    if(p_thread->pgdir!=NULL)
    {
        //说明再用户进程中

        //记录一下我犯的sb错误
        //我把get_phy_addr写成get_page_dir了...
        pdir_phy=get_physical_addr((uint32)p_thread->pgdir);
    }
    asm volatile ("movl %0, %%cr3" : : "r" (pdir_phy) : "memory");
}
uint32* create_page_dir(void)
{
    uint32* pdir_vaddr=malloc_pages(1);
    if(pdir_vaddr==NULL)
    {
        console_debug("cannot alloc a page for page_dir!",DEBUG_FAIL);
        return NULL;
    }
    memcpy((uint32*)((uint32)pdir_vaddr + 0x300*4), (uint32*)(0xfffff000+0x300*4), 1024);
    uint32 new_addr=get_physical_addr(pdir_vaddr);
    pdir_vaddr[1023]=new_addr| PG_US_U | PG_RW_W | PG_P_1;
    return pdir_vaddr;
}
void create_user_vaddr_bitmap(task_struct_t* user_prog)
{
    user_prog->user_vaddr.vaddr=USER_VADDR_START;
    uint32 cnt=DIV_ROUND_UP((0xc0000000-USER_VADDR_START)/4096/8,4096);

    
    user_prog->user_vaddr.bitmap.bits=malloc_pages(cnt);
   
        
    user_prog->user_vaddr.bitmap.len=(0xc0000000-USER_VADDR_START)/4096;
    memset(user_prog->user_vaddr.bitmap.bits,0,cnt*4096);
    //console_print_str("addr:0x");
    
}
int process_do_switch(struct intr_stack *proc_stack,task_struct_t *thread)
{
          asm volatile("         \
     cli;                 \
     mov %0, %%ecx;       \
     mov %1, %%esp;       \
     mov %2, %%ebp;       \
     sti;                 \
     call *%%ecx           "
                : : "r"(proc_stack->eip), "r"(proc_stack), "r"(proc_stack->ebp));
    //正常情况下这里不会执行到
    //除非call里面返回了
    int32 ret; 
    asm ("mov %%eax, %0" : "=g" (ret));
    running_thread()->ret=ret;
    running_thread()->status=TASK_DIED;
    thread->ret=ret;
    thread->status=TASK_DIED;
    while(1);
   
}
void destroy_thread(task_struct_t *pthread)
{
    intr_disable_loop();
    printf("pdir:0x%x self:0x%x tag:%x\n",pthread->pgdir,pthread,&pthread->all_list_tag);
    
    list_remove(&pthread->all_list_tag);
    if(pthread->pgdir)
        free_pages(pthread->pgdir,1);
    printf("pdir_freeok!\n");
    release_pid(pthread->pid);
    //free_pages(pthread->open_fd,1);//4096byte(1024个fd)
    free_pages(pthread,1);
    printf("pdir_freeok!\n");
    intr_enable_loop();
}
int wait_to_died(uint32 pid)
{
    /*task_struct_t *thread=get_task_by_pid(pid);
    if(thread==NULL)return 0;
    while (thread->status!=TASK_DIED)
    {
        thread=get_task_by_pid(pid);
        if(thread==NULL)return 0;
    }
    //进程已经死亡
    //进行善后处理
    printf("!!!!!!!!!!!!!!!!!!!thread died! with ret:%d\n",thread->ret);
    int32 ret=thread->ret;
    //destroy_thread(thread);
    printf("pdir_freeok!%d\n",ret);*/
}
static bool pid_check(list_elem_t *elem,uint32 pid)
{
    task_struct_t*cur=elem2entry(struct task_struct,all_list_tag,elem);
    if(cur->pid==pid)
        return true;
    return false;
}
task_struct_t *get_task_by_pid(uint32 pid)
{
    list_elem_t *elem=list_traversal(&thread_all_list,pid_check,pid);
    if(elem==NULL)  
        return NULL;
    task_struct_t *cur=elem2entry(struct task_struct,all_list_tag,elem);
    return cur;
}