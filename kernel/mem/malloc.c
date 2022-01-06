/*
 * @Description: 简单的内存分配算法
 * @Author: QIUFUYU
 * @Date: 2021-09-25 11:52:37
 * @LastEditTime: 2022-01-03 15:50:26
 */
#include"mem/memory.h"
#include"qstring.h"
#include"console.h"
#include"task/sync.h"
#include"qassert.h"
#include"kstdio.h"
uint32 KENEL_START_ID=0;
#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))
typedef struct Large_mem_info
{
    uint32 addr;//TODO: addr 4kb对齐，所以可以考虑将页大小放在addr的后3位
    uint16 p_sz;//不至于大到用u32吧...
}Large_mem_info_t;
Large_mem_info_t* large_mem_list;
uint32 large_mem_len=0;
struct lock lock;
static inline void set_page(uint32 page_index) {
    active_memory.pages[page_index/32] |= (1 << (page_index % 32));
}

static inline void unset_page(uint32 page_index) {
    active_memory.pages[page_index/32] &= ~(1 << (page_index % 32));
}

static inline int test_page(uint32 page_index) {
    return active_memory.pages[page_index/32] & (1 << (page_index % 32));
}
static int32 get_pages(uint32 sz)
{
    uint32 i, j;
    uint32 cnt=0,lst=0,fst=0;
    uint32 num_pages = 1024*1024;
    uint32 start=0;
    start=KENEL_START_ID;
    for (i=start; i<num_pages/32; ++i) {
        if (active_memory.pages[i] != 0xFFFFFFFF) {
            for (j=0; j<32; ++j) {
                if (!(active_memory.pages[i] & (1 << j))) {
                    if(lst==0)
                    { 
                        lst=i*32 + j;
                        fst=lst;
                        cnt=1;
                    }
                    else if(i*32 + j-lst==1)
                    {
                        lst=i*32 + j;
                        cnt++;
                    }else lst=0;
                    if(cnt==sz)
                    {
                        return fst;
                    }
                }
            }
        }
    }
    return -1;
}
bool init_large_mem(uint32 len)
{
    large_mem_list=malloc(sizeof(Large_mem_info_t)*len);
    if(large_mem_list==NULL)return false;
    memset(large_mem_list,0,sizeof(Large_mem_info_t)*len);
    large_mem_len=len;
    return true;
}
void *get_user_pages_vaddr(uint32 sz)
{
    lock_acquire(&lock);
    task_struct_t *cur=running_thread();
    uint32 idx=ubitmap_scan(sz,&cur->user_vaddr.bitmap);
    if(idx<=1)
        return NULL;
    uint32 cnt=0;
    while (cnt<sz)
    {
        ubitmap_set_page(&cur->user_vaddr.bitmap,idx+cnt++);
    }
    uint32 vaddr=cur->user_vaddr.vaddr+idx*4096;
    ASSERT(vaddr<(0xc0000000-4096));
    lock_release(&lock);
    return vaddr;
}
void *get_a_thread_page(uint32 vaddr,bool is_kernel)
{
    lock_acquire(&lock);

    task_struct_t *cur=running_thread();
    uint32 bit_idx=-1;
    if(cur->pgdir!=NULL && !is_kernel)
    {
        //printf("                ---------user alloc user\n");
        //printf("v:%x ,u:%x\n",vaddr,cur->user_vaddr.vaddr);
        uint32 idx=(vaddr-cur->user_vaddr.vaddr)/4096;
        //console_print_dec(cur->user_vaddr.bitmap.len);
        //printf("idx:%d\n",idx);
        ASSERT(idx>0);
        ubitmap_set_page(&cur->user_vaddr.bitmap,idx);
    }else if(cur->pgdir==NULL && is_kernel)
    {
        //printf("            --------------kernel alloc kernel!\n");
        uint32 idx=vaddr/4096;
        ASSERT(idx>=0);
        set_page(idx);
    }
    else 
    {
        printf("cur:0x%x thread name:%s pdir:%x\n",cur, cur->name,cur->pgdir);
        console_debug("get a thread page:not allow get a user page by kernel or get a kernel page by user!",DEBUG_PANIC);
    }
    //printf("vaddr:0x%x\n",vaddr);
    //console_print_str("\nmmmm");
    map_page(pm_alloc_frame(),vaddr);
    //printf("almost ok!\n");
    lock_release(&lock);
    return vaddr;

}
void *malloc_pages(uint32 sz)
{

    lock_acquire(&lock);
    int32 idx= get_pages(sz);
    //printf("            pages:idx:%d\n",idx);
    //console_print_dec(idx);
    if(idx<0)
    {
        lock_release(&lock);
        return NULL;
    }
    void *phy=NULL;
    void *vir=NULL;
    //printk("addr:0x%x\n",&vir);
    for(uint32 n=idx;n<idx+sz;n++)
    {
        phy=pm_alloc_frame();
        if(phy==NULL)
        {
            lock_release(&lock);
            //printf("           error:");
            //console_print_str("m");
            return NULL;
        }
        vir=(void*)(n*4096);
        map_page(phy,vir);
        set_page(n);
        memset(vir,0,4096);
        //printk("map:%d / %d\n",n,idx+sz);
        //printk(""); 
    }
    active_memory.used_page_cnt+=sz;
    lock_release(&lock);
    //printf("           pages:%x\n",idx*4096);
    
    return (void*)(idx*4096);
}
void free_pages(void *vir_addr,uint32 sz)
{
    lock_acquire(&lock);
    for(uint32 i=(uint32)vir_addr;i<(uint32)vir_addr+sz*4096;i+=0x1000)
    {
        //printf("now free p:0x%x\n",i);
        unmap_page(i);
        //printf("!\n");
        unset_page(i/4096);
        //printf("!\n");
    }
    lock_release(&lock);
    //printf("fpageox!\n");
}
void* allocate_new_page() {
    return malloc_pages(1);
}
struct Memory active_memory;

static const uint32 FREE_BLOCK = 0xc001d00d;
static const uint32 ALLOC_BLOCK = 0xdeadd00d; 
mem_block_t* alloc_mem_block(uint8 sz)
{
    
    void* page=allocate_new_page();
    
    if(page==NULL)
    {
        //*is_fail=true;
        return NULL;
    }
    //console_print_str("malloc: alloc a new page\n");
    memset(page,0,4096);
    mem_block_t *re=page;
    re->next=NULL;
    re->prev=NULL;
    re->sz=sz;
    re->used=0;
    
    //*is_fail=false;
    return re;
}
bool connect_new(mem_block_t *block_old)
{
    mem_block_t *new=alloc_mem_block(block_old->sz);
    //console_print_str("connect a page.\n");
    if(new==NULL)
        return false;
    block_old->next=new;
    new->prev=block_old;
    new->next=NULL;
    return true;
}
static void*alloc_block(mem_block_t *block)
{
    //pS
    //这里调试了很久
    //max_num一会儿是0,一会儿是正常值
    //后来发现是栈太小了
    //mov esp得大一点（现在0x2400)
    //by qiufuyu
    mem_block_t *ls_block=block;
    uint32 spit=(1<<(ls_block->sz+1))+1;
    uint32 max_num=(4096-sizeof(mem_block_t))/spit;
    //printf("spit:%d      ,    max:%d       sz:%d\n",spit,max_num,ls_block->sz);
    while (1)
    {
        
        //printf("==============NEXT:0x%x\n",ls_block->next);
        //printf("ls_block:%x,used:%d,max:%d\n",ls_block,ls_block->used,max_num);
        if(ls_block->used<max_num)
        {
            //printf("**********************************8ptr:0x%x,max ptr:0x%x",(uint32)ls_block+sizeof(mem_block_t),(uint32)ls_block+4096);
            //还有空间可以分配
            //则遍历分配
            uint32 re_ptr=0;
            for(uint32 ptr=(uint32)ls_block+sizeof(mem_block_t);ptr<(uint32)ls_block+4096;ptr+=spit)
            {
                char *p=(char*)ptr;
                //printf("scan:0x%x\n",p);
                if(*p==0x00)
                {
                    //写入单位记号
                    *p=ls_block->sz;
                    *p|=0x80;
                    re_ptr=ptr+1;
                    ls_block->used++;
                    //printf("alloc:0x%x\n",re_ptr);
                    return re_ptr;
                }
            }
            return NULL;
        }else if(ls_block->used>=max_num){
            //卧槽 !!!!!!!!!!!!!!!!!!!!!!
            //这里调试了好久
            //ls_block 被我写成了 block
            //无语了
            //printf("--------------used:%d,max:%d\n",ls_block->used,max_num);
            //printf("==========================-------------==connect new!\n");
            if(ls_block->next==NULL)
            {
                
                if(!connect_new(ls_block))
                    return NULL;
                //printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaold sz:%d\n",ls_block->used);
                
                //printf("========================new:0x%x",ls_block->next);
            }
            
            ls_block=ls_block->next;
            //printf("hasls:0x%x=====================\n",ls_block);
        }
        /* code */
    }
    
}
static void* alloc_mem(uint8 sz)
{
    if(active_memory.blocks[sz]==NULL)
    {
        //bool is_fail;
        active_memory.blocks[sz]=alloc_mem_block(sz);
        //printf("sz:%d alloc a new page!\n",sz);
        
        if(active_memory.blocks[sz]==NULL)
        {
            console_print_str("fail to alloc page");
            console_print_hex(active_memory.blocks[sz]);
            console_print_str("\n");
            return NULL;
        }
    }
    return alloc_block(active_memory.blocks[sz]);
}
void *realloc(void *addr,uint32 sz)
{

    if(addr==NULL)
        return 0;
    
    //lock_acquire(&lock);
    uint32 real_sz=0;
    for(int i=0;i<large_mem_len;i++)
    {
        if(large_mem_list[i].addr==addr)
        {
            //这是个大内存块
            //调用free_pages释放
            real_sz=large_mem_list[i].p_sz*4096;
        }
    }
    
    if(!real_sz)
    {
        char*head=(void*)((uint32)addr-1);
        char s=*head;
        if(s==0)
            return NULL;
        real_sz=1<<((s&0x0F)+1);
    }
    if(real_sz<sz)
    {
        void *new=malloc(sz);
        if(new==NULL)
            return NULL;
        memcpy(new,addr,real_sz);
        free(addr);
        return new;
    }else if(real_sz>sz)
    {
        void *new=malloc(sz);
        if(new==NULL)
            return NULL;
        memcpy(new,addr,sz);
        free(addr);
        return new;
    }
    else return addr;
    
}
bool mem_is_safe(uint32 page_sz,double save_ret)
{
    if(active_memory.max_page_cnt*save_ret<active_memory.used_page_cnt)return false;
    if(page_sz<=(active_memory.max_page_cnt*save_ret-active_memory.used_page_cnt))return true;
    else return false;
}
void *calloc(int cnt,int len)
{
    void *re=malloc(len*cnt);
    if(re)
    {
        memset(re,0,len*cnt);

    }
    return re;
}
void* malloc(uint32 size)
{
    lock_acquire(&lock);
    ////printf("malloc :%d\n",size);
    //先分割size
    if(size<=2)
    {
        
        void*re= alloc_mem(M_2);
        lock_release(&lock);
        //printf("000000000000000000malloc1:%x\n",re);
        return re;
    }
    else if(size>2&& size<=4)
    {
        void*re= alloc_mem(M_4);
        lock_release(&lock);
        return re;
    }else if(size>4 && size<=8)
    {
        void*re= alloc_mem(M_8);
        lock_release(&lock);
        return re;  
    }else if(size>8 && size<=16)
    {
        void*re= alloc_mem(M_16);
        lock_release(&lock);
        return re;
    }else if(size>16 && size<=32)
    {
        void*re= alloc_mem(M_32);
        lock_release(&lock);
        return re;
    }else if(size>32 && size<=64)
    {
        void*re= alloc_mem(M_64);
        lock_release(&lock);
        return re;
    }else if(size>64 && size<=128)
    {
        void*re= alloc_mem(M_128);
        lock_release(&lock);
        return re;
    }else if(size>128 &&size<=256)
    {
        void*re= alloc_mem(M_256);
        lock_release(&lock);
        return re;
    }else if(size>256 &&size<=512)
    {
        void*re= alloc_mem(M_512);
        lock_release(&lock);
        return re;
    }
    else if(size>512 && size<=1024)
    {
        void*re= alloc_mem(M_1024);
        lock_release(&lock);
        return re;
    }
    else if(size>1024 && size<=2048)
    {
        void*re= alloc_mem(M_2048);
        lock_release(&lock);
        return re;
    }
    else
    {
        uint32 re_addr=0;
        //printf("!!!!!!!!!!!!!!!!alloc a big mem!!!!!!\n");
        //大内存分配直接用页
        if(large_mem_list==NULL||large_mem_len==0)
        {
            console_debug("fail to init Large_mem_list.",DEBUG_PANIC);
        }
        for(int i=0;i<large_mem_len;i++)
        {
            if(large_mem_list[i].addr==0)
            {
                re_addr=malloc_pages(DIV_ROUND_UP(size,0x1000));
                if(re_addr==0)break;
                large_mem_list[i].addr=re_addr;
                large_mem_list[i].p_sz=DIV_ROUND_UP(size,0x1000);
                //printf("addr:0x%x\n",re_addr);
                break;
            }
        }
        lock_release(&lock);
        //printf("error :malloc too big!\n");
        //while (1)
        //{
            /* code */
        //}
        
         return re_addr;//大内存申请请用malloc_pages
    }
    
}
uint32 free(void* address)
{
    //TODO: lock acquire应该在前面
    //首先检查大内存模块
    if(address)
    {
    for(int i=0;i<large_mem_len;i++)
    {
        if(large_mem_list[i].addr==address)
        {
            //这是个大内存块
            //调用free_pages释放
            free_pages(address,large_mem_list[i].p_sz);
            return large_mem_list[i].p_sz*4096;
        }
    }
    char*head=(void*)((uint32)address-1);
    char sz=*head;
    if(sz==0)
        return 0;//释放失败
    lock_acquire(&lock);
    uint8 real_sz=sz&0x0F;
    if(active_memory.blocks[real_sz]!=NULL)
    {
        mem_block_t *ls=active_memory.blocks[real_sz];
        while (1)
        {
            if(ls==NULL)
                break;
            if((uint32)address>(uint32)ls && (uint32)address<(uint32)ls+4096)
            {
                ls->used--;
                *head=0;
                //我们需要检测used==0
                //如果used==0
                //则unsetpage

                if(ls->used==0&&ls->prev!=NULL)
                {
                    ls->next->prev=ls->prev;
                    ls->prev->next=ls->next;
                    uint32  addr=(uint32)ls;
                    unset_page(addr/4096);
                    unmap_page(addr);
                    
                }

                //console_print_str("free ok! 0x");
                //console_print_hex(head+1);
                //console_print_str("\n");
                lock_release(&lock);
                return (1<<(sz+2));
            }
            else if(ls->next!=NULL)
            {
                ls=ls->next;
            }else{
lock_release(&lock);
                return 0;
            }
        }
        
    }
    else 
    {
        lock_release(&lock);
        return 0;
        }
    }
    return 0;
}