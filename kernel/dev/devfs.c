/*
 * @Description: devfs设备管理器
 * @Author: QIUFUYU
 * @Date: 2021-10-20 20:53:57
 * @LastEditTime: 2021-10-21 21:35:36
 */
#include"dev/devfs.h"
#include"mem/malloc.h"
#include"console.h"
#include"kstdio.h"
#include"qstring.h"
#include"task/sync.h"
struct lock dev_lock;
dev_module_t *dev_list;
#define DEV_LIST_SZ 1024
void init_devfs()
{
    lock_init(&dev_lock);
    dev_list=malloc(sizeof(dev_module_t)*DEV_LIST_SZ);
    if(dev_list==NULL)
        console_debug("FAIL TO INIT DEVFS!",DEBUG_PANIC);
    memset(dev_list,0,sizeof(dev_module_t)*1024);
}
static int devfs_find_free()
{
    for (int i=0;i<DEV_LIST_SZ;i++)
    {
        if(dev_list[i].name==NULL)
            return i;
    }
    return -1;
    
}
static int devfs_del(int fd)
{
    dev_list[fd].name=NULL;
    return fd;
}
int devfs_add_dev(dev_module_t mod)
{
    if(!mod.name) return -1;
    lock_acquire(&dev_lock);
    int fd=devfs_find_free();
    if(fd==-1)
    {
        lock_release(&dev_lock);
        return -1;
    }
    dev_list[fd]=mod;
    lock_release(&dev_lock);
    return fd;    
}
dev_module_t devfs_create_dev(char *name,char *bind_f_name,fs_dev_open(open),fs_dev_close(close),\
fs_dev_read(read),fs_dev_write(write),fs_dev_lseek(lseek) )
{
    dev_module_t re;
    re.bind_file_name=bind_f_name;
    re.name=name;
    re.methods.close=close;
    re.methods.open=open;
    re.methods.write=write;
    re.methods.read=read;
    re.methods.lseek=lseek;
    return re;
}

int devfs_find_by_fname(char *bind_f_name)
{
    for (int i=0;i<DEV_LIST_SZ;i++)
    {
        if(dev_list[i].bind_file_name)
        {
        if(!strcmp(dev_list[i].bind_file_name,bind_f_name))
            return i;
        }
    }
    return -1;
}

int devfs_find_by_name(char *name)
{
    for (int i=0;i<DEV_LIST_SZ;i++)
    {
        if(!strcmp(dev_list[i].name,name))
            return i;
    }
    return -1;
}

int devfs_open(int fd)
{
    if(fd>-1&&fd<DEV_LIST_SZ)
    {
        if(dev_list[fd].name)
        {
            lock_acquire(&dev_lock);
            int ret=-1;
            if(dev_list[fd].methods.open)
                ret= dev_list[fd].methods.open(&dev_list[fd]);
            lock_release(&dev_lock);
            return ret;
        }   
    }
    return -1;
}
int devfs_close(int fd)
{
    if(fd>-1&&fd<DEV_LIST_SZ)
    {
        if(dev_list[fd].name)
        {
            lock_acquire(&dev_lock);
            int ret=-1;
            if(dev_list[fd].methods.close)
                ret= dev_list[fd].methods.close(&dev_list[fd]);
            if(devfs_del(fd)!=fd)
            {
                console_debug("FAIL TO DELET DEVICE!",DEBUG_PANIC);
            }
            lock_release(&dev_lock);
            return ret;
        }   
    }
    return -1;
}
int devfs_write(int fd,char *src,uint32 count)
{
    if(fd>-1&&fd<DEV_LIST_SZ)
    {
        if(dev_list[fd].name)
        {
            lock_acquire(&dev_lock);
            int ret=-1;
            if(dev_list[fd].methods.write)
                ret= dev_list[fd].methods.write(src,count);
            lock_release(&dev_lock);
            return ret;
        }   
    }
    return -1;
}
int devfs_read(int fd,char *dst,uint32 count)
{
    if(fd>-1&&fd<DEV_LIST_SZ)
    {
        if(dev_list[fd].name)
        {
            lock_acquire(&dev_lock);
            int ret=-1;
            if(dev_list[fd].methods.read)
                ret= dev_list[fd].methods.read(dst,count);
            lock_release(&dev_lock);
            return ret;
        }   
    }
    return -1;
}
int devfs_lseek(int fd,uint32 off,uint8 flag)
{
    if(fd>-1&&fd<DEV_LIST_SZ)
    {
        if(dev_list[fd].name)
        {
            lock_acquire(&dev_lock);
            int ret=-1;
            if(dev_list[fd].methods.lseek)
                ret= dev_list[fd].methods.lseek(off,flag);
            lock_release(&dev_lock);
            return ret;
        }   
    }
    return -1;
}
