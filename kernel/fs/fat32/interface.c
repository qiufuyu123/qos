/*
 * @Description: fat32文件接口
 * @Author: QIUFUYU
 * @Date: 2021-10-07 08:25:36
 * @LastEditTime: 2022-01-03 10:45:39
 */
#include"fs/fat32/interface.h"
#include"mem/malloc.h"
#include"qstring.h"
#include"kstdio.h"
#include"console.h"
#include"task/kthread.h"
#include"isr.h"
#include"fs/fd.h"
#include"fs/fs_interface.h"
#include"fs/hdd.h"
//#include"task/sync.h"
typedef char* string;
struct directory *root_dir;
//struct lock flock;
f32 *fatfs;


typedef struct fs_fd
{
    //文件描述符

    //fs_device_t *dev;
    struct dir_entry *f;
    int32 bind_dev;
    
    struct directory *last_dir;
    char *fname;
    //int pid;//描述符持有者 -1为未初始化
    uint32 seek;//文件写位置指针
    uint8 flag;//文件打开模式
}fs_fd_t;
fs_fd_t *fd_map;
uint32 fd_map_sz=1000;
static char *split_left(char *str,char c)
{
    for(int i=0;i<strlen(str);i++)
    {
        if(str[i]==c)
        {
            if(i==0)    return NULL;
            
            //printf("i:%d\n",i);
            //while(1);
            //console_clean();
            char *re=malloc(i);
            
            //printf("xxxxxxxxxxxxxxxxxre:%x\n",re);
            //while(1);
            if(re==NULL)    return NULL;
            memcpy(re,str,i);
            return re;
        }
    }
    return NULL;
}
static char *split_right(char *str,char c)
{
    for(int i=0;i<strlen(str);i++)
    {
        if(str[i]==c)
        {
            if(strlen(str)- i<=1)    return NULL;
            //printf("len:%d\n",strlen(str)-i-1);
            char *re=malloc(strlen(str)-i);
            memset(re,'\0',strlen(str)-i);
            
            //re[strlen(str)-i-1]='\0';
            //printf("re: %s\n",re);

            if(re==NULL)    return NULL;
            char *st=(char *)((uint32)str+i+1);
            //printf("test:%s\n",str);
            memcpy(re,st,strlen(str)-i-1);
            re[strlen(str)-i]='\0';
            return re;
        }
    }
    return NULL;
}
static struct dir_entry *build_path(char *name,struct directory *last_dir)
{
    //console_clean();
        char *a;
    char *b;
    char *c=split_right(name,'/');
    struct dir_entry *ls_entry;
    struct directory *ls_dir=malloc(sizeof(struct directory));
    memcpy(ls_dir,root_dir,sizeof(struct directory));
    //printf("first dir:%s\n",c);
    //printf("root:%x\n",root_dir);
    while (1)
    {
        //while(1);
        //malloc(2);
        a=split_left(c,'/');
        //while(1);
        b=split_right(c,'/');
        //while(1);
        //print_directory(fatfs,ls_dir);
        //printf("===\n");
        if(a==NULL&&b==NULL)
        {
            //printf("c:%s 0x%x\n",c,ls_dir->entries[0].name);
            //print_directory(fatfs,ls_dir);
            //while(1);
            ls_entry= get_dir_entry(ls_dir,c);
            //fname=c;
            //printf("f:%s\n",c);
            //free(c);
            //printf("file:%s attr:%x sz:%d \n",ls_entry->name,ls_entry->dir_attrs,ls_entry->file_size);
            //printf("final name:%s\n",c);
            if(last_dir)
                memcpy(last_dir,ls_dir,sizeof(struct directory));
            free(ls_dir);

            //printf("ii");
            return ls_entry;
        }
        //现在的目录=a
        //即根目录下第一个目录
        
    //printf("in dir:%s right:%s\n",a,b);
        //printf("in dir:%s right:%s\n",a,b);
        ls_entry= get_dir_entry(ls_dir,a);
        //printf("ls_entry:%s\n",ls_entry->name);
        if(ls_entry==NULL)
        {
            free(ls_dir);
            return NULL;
        }
        populate_dir(fatfs,ls_dir, ls_entry->first_cluster);
        //free(c);
        //c=malloc(strlen(b));
        //memset(c,0,strlen(c));
        //memcpy(c,b,strlen(b));
        strcpy(c,b);
        //printf("c:%s\n",c);
        //while(1);
        /* code */
        free(a);
        free(b);
    }
}
static char *get_fname(char *path)
{
    char *c=malloc(strlen(path));
    strcpy(c,path);
    char *a;
    char *b;
    //a=split_left(c,'/');
    b=split_right(c,'/');
    if(b==NULL)return NULL;
    while (1)
    {
        a=split_left(c,'/');
        b=split_right(c,'/');
        if(a==NULL&&b==NULL)
            return c;
        
        strcpy(c,b);
        free(a);
        free(b);
        /* code */
    }
    
}
static fs_fd_t *new_fd(struct dir_entry *f,uint8 flag,char *fname,struct directory* dir)
{
    fs_fd_t*fd=calloc(1,sizeof(fs_fd_t));
    if(!fd)return NULL;
    fd->f=f;
    fd->flag=flag;
    fd->seek=0;
    fd->fname=fname;
    fd->bind_dev=-1;
}
static int getflag(char c)
{
    if(c=='r')return O_RO;
    else if(c=='w')return O_WO;
    else if(c=='c')return O_CR;
    else return -1;
}
static int getstrflag(char *flags)
{
    int f1=getflag(flags[0]);
    if(f1==-1)return -1;
    if(strlen(flags)==1)return f1;
    else if(strlen(flags)==2)
    {
        int f2=getflag(flags[1]);
        if(f1==O_RO&&f2==O_WO)return O_RW;
        else return f1|f2;
    }else return -1;
}
int fs_open_cflag(char *name,char *flags)
{
    //解析字符串形式的flags
    int flag=getstrflag(flags);
    if(flag==-1)return -1;
    return fs_open(name,flag);
}
int fs_open(char *name,uint32 flags)
{
    //打开文件时不允许其他进程干扰
    intr_disable_loop();
    char* fname;
    struct directory last_dir;
    struct dir_entry *f=NULL;
    //f.name=NULL;
    f= build_path(name,&last_dir);
    fname=get_fname(name);
    //printf("fname:%s\n",f->name);
    //while(1);
    if(f==NULL && !(flags&O_CR))
    {
        free(fname);
        intr_enable_loop();
        //printf("not exesiz!\n");
        return -1;
    }else if(f==NULL && (flags&O_CR))
    {
        //TODO:此时需要创建一个文件
        //
        //printf("ls:%d\n",last_dir.num_entries);
        //print_directory(fatfs,&last_dir);
        
        //printf("creating...ls 0x%x fname:%s\n",last_dir.entries,fname);
        writeFile(fatfs,&last_dir,NULL,fname,0);
        //printf("cok!\n");
        //再build_path一遍
        f=build_path(name,NULL);
        if(!(f))
        {
            //printf("nonon!\n");
            free(fname);
            return -1;
        }
        //printf("new:%x\n",f);
        struct directory *dir=malloc(sizeof(struct directory));
        memcpy(dir,&last_dir,sizeof (struct directory));
        fs_fd_t*ls=new_fd(f,flags,fname,dir);
        if(!ls)
        {
            free(fname);
            intr_enable_loop();
            return -1;
        }
        fd_err e;
        int fd=fs_put_fd(ls,FD_FILE,flags,&e);
        if(e!=FD_ERR_SUCCESS)
        {
            free(fname);
            free(ls);
            intr_enable_loop();
            return -1;
        }
        
         return fd;
    }
    if(!(f->dir_attrs&FAT_FILE))
    {
        //printf("not file!\n");
        intr_enable_loop();
        return -1;
    }
    struct directory *dir=malloc(sizeof(struct directory));
    memcpy(dir,&last_dir,sizeof (struct directory));
    //记录一下
    //一不小心把f写成&f了（因为之前思路有问题，中间重写了一次，
    //结果这里忘改了...
    //2021/10/21 注
    fs_fd_t*ls=new_fd(f,flags,fname,dir);
    if(!ls)
    {
        free(fname);
        free(dir);
        intr_enable_loop();
        return -1;
    }
    fd_err e;
    int fd=fs_put_fd(ls,FD_FILE,flags,&e);
    if(e!=FD_ERR_SUCCESS)
    {
        free(fname);
        free(dir);
        free(ls);
        intr_enable_loop();
        return -1;
    }
    intr_enable_loop();
    return fd;

}
uint32 fs_tell(int fd)
{
    intr_disable_loop();
    //清内存
    fd_desc_t*file=fs_get_fd(fd);
    if(!file)
    {
        intr_enable_loop();
        return ;
    }
    
    fs_fd_t*self=file->target_ptr;

    intr_enable_loop();
    return self->f->file_size-self->seek;
}
void fs_del(int fd)
{
    intr_disable_loop();
    //清内存
    fd_desc_t*file=fs_get_fd(fd);
    if(!file)
    {
        intr_enable_loop();
        return ;
    }
    fs_fd_t*self=file->target_ptr;
    delFile(fatfs,self->last_dir,self->fname);
    fs_close(fd);
    intr_enable_loop();
}
void fs_close(int fd)
{
    intr_disable_loop();
    //清内存
    fd_desc_t*file=fs_get_fd(fd);
    if(!file)
    {
        intr_enable_loop();
        return ;
    }
    free(file->target_ptr);
    fs_del_fd(fd);
    intr_enable_loop();
}
int fs_read(int fd,char *buff,uint32 size)
{
    intr_disable_loop();
    fd_desc_t*file=fs_get_fd(fd);
    //fs_fd_t*self=file->target_ptr;
    if(!file)
    {
        intr_enable_loop();
        return -1;
    }
    fs_fd_t*self=file->target_ptr;

        if(file->pid==running_thread()->pid&&self->f&&((self->flag&O_RW)||(self->flag&O_RO)))
        {
            //printf("f:%s\n",self->f->name);
            //printf("===================");
            char *addr= readFile(fatfs,self->f);
            //printf("test:%s\n",addr);
            
            if(size>self->f->file_size)
            {

                //根据seek复制
                char *target=(char *)((uint32)addr+self->seek);
                //printf("seek:%d\n, tar:%s\n",self->seek, target);
                memcpy(buff,target,self->f->file_size-self->seek);
                free(addr);
                intr_enable_loop();
                return self->f->file_size;
            }
            else 
            {
                char *target=(char *)((uint32)addr+self->seek);
                //printf("seek:%d\n, tar:%s\n",self->seek, target);
                if(size>self->f->file_size-self->seek)
                {
                    memcpy(buff,target,self->f->file_size-self->seek);
                }
                else memcpy(buff,target,size);
            }
            free(addr);
            intr_enable_loop();
            return size;
        }
    intr_enable_loop();
    return 0;

}
void fs_lseek(int fd, uint32 offset ,uint8 flag)
{
    intr_disable_loop();
    fd_desc_t*file=fs_get_fd(fd);
    if(!file)
    {
        intr_enable_loop();
        return;
    }
    fs_fd_t*self=file->target_ptr;
        if(file->pid==running_thread()->pid)
        {
            uint32 new_seek;
            if(flag==SEEK_SET)
                new_seek=offset;
            else if(flag==SEEK_END)
                new_seek=self->f->file_size-1;
            else new_seek=0;
            self->seek=new_seek;
        }
    intr_enable_loop();
}
uint32 fs_write(int fd,char*buf,uint32 size)
{
    intr_disable_loop();
    fd_desc_t*file=fs_get_fd(fd);
    if(!file)
    {
        intr_enable_loop();
        return 0;
    }
    fs_fd_t*self=file->target_ptr;
        if(file->pid==running_thread()->pid&&self->f&&((self->flag&O_RW)||(self->flag&O_WO)))
        {
            //根据lseek来写文件
            char *writen=NULL;
            uint32 real_sz=size;
            uint32 old_seek=self->seek;
            bool cat=false;
            
            if(self->seek!=0)
            {
                char *ls_buff=malloc(self->seek+size);
                self->seek=0;
                fs_read(fd,old_seek,ls_buff);
                self->seek=old_seek;
                //writen=malloc(old_seek+size);
                ls_buff+=old_seek;
                memcpy(ls_buff,buf,size);
                writen=ls_buff;
                real_sz+=old_seek;
                cat=true;
            }else writen=buf;

            writeFile(fatfs,self->f,writen,self->fname,real_sz);
            if(cat) free(writen);
            //memcpy(buf,addr,size);
            //free(addr);
            intr_enable_loop();
            return real_sz;
        }
        intr_enable_loop();
        return 0;
}
uint32 fs_get_size(int fd)
{
    fd_desc_t*file=fs_get_fd(fd);
    if(!file)
    {
        return 0;
    }
    fs_fd_t*self=file->target_ptr;
    return self->f->file_size;
}
void fs_print_root()
{
    print_directory(fatfs,root_dir);
}
void fs_interface_init(f32 *fs)
{
    //fd_map=malloc_pages(sizeof(fs_fd_t)*fd_map_sz/4096+1);
    //memset(fd_map,0,sizeof(fs_fd_t)*fd_map_sz);
    root_dir =malloc(sizeof(struct directory));
    if(root_dir==NULL)
    {
        printf("fail for root dir\n");
        while(1);
    }
    populate_root_dir(fs,root_dir);
    fatfs=fs;
    print_directory(fs,root_dir);
    fs_interface_t *inter=calloc(1,sizeof(fs_interface_t));
    if(!inter)
    {
        printk("fail to malloc for FAT INTERFACE!\n");
        while(1);
        return;
    }
    strcpy(inter->info.info,"FAT32 FS");
    strcpy(inter->info.name,"FAT32 FS");
    inter->info.magic=0x23333;
    inter->info.version=10;
    inter->methods.open=fs_open_cflag;
    inter->methods.close=fs_close;
    inter->methods.lseek=fs_lseek;
    inter->methods.read=fs_read;
    inter->methods.write=fs_write;
    inter->methods.tell=fs_tell;
    inter->methods.del=fs_del;
    inter->self_data=NULL;
    
    ata_selected_dev->fs=inter;
    int fd=ata_selected_dev->fs->methods.open("/d.txt","r");
    printk("open a fd:%d\n",fd);
    char buf[100];
    printk("read : %d\n",ata_selected_dev->fs->methods.read(fd,buf,100));
    printk("show time!\n%s\n",buf);
    
    //while(1);
}


//dev