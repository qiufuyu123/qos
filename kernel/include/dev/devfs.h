/*
 * @Description: 设备管理器
 * @Author: QIUFUYU
 * @Date: 2021-10-20 20:41:21
 * @LastEditTime: 2021-10-20 21:12:27
 */
#ifndef _DEVFS_H
#define _DEVFS_H
#include"types.h"
#define fs_dev_open(f) int(*f)(void *module)
#define fs_dev_close(f) int(*f)(void *module)
#define fs_dev_write(f) int(*f)(char *buf,uint32 sz)
#define fs_dev_read(f) int(*f)(char *buf ,uint32 sz)
#define fs_dev_lseek(f) int(*f)(int offset,uint8 flag) 
typedef struct dev_method
{
    fs_dev_open(open);
    fs_dev_close(close);
    fs_dev_read(read);
    fs_dev_write(write);
    fs_dev_lseek(lseek);
}dev_method_t;
typedef struct dev_module
{
    char *name;
    char *bind_file_name;
    dev_method_t methods;
}dev_module_t;
enum
{
    DEVFS_OP_READ=1,
    DEVFS_OP_WRITE,
    DEVFS_OP_OPEN,
    DEVFS_OP_CLOSE,
    DEVFS_OP_LSEEK
};
void init_devfs();

int devfs_add_dev(dev_module_t mod);
dev_module_t devfs_create_dev(char *name,char *bind_f_name,fs_dev_open(open),fs_dev_close(close),\
fs_dev_read(read),fs_dev_write(write),fs_dev_lseek(lseek) );

int devfs_find_by_fname(char *bind_f_name);

int devfs_find_by_name(char *name);

int devfs_open(int fd);
int devfs_close(int fd);
int devfs_write(int fd,char *src,uint32 count);
int devfs_read(int fd,char *dst,uint32 count);
int devfs_lseek(int fd,uint32 off,uint8 flag);
#endif