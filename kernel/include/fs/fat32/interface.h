/*
 * @Description: 文件接口
 * @Author: QIUFUYU
 * @Date: 2021-10-07 08:25:51
 * @LastEditTime: 2022-01-18 21:18:18
 */
#ifndef _H_INTERFACE
#define _h_INTERFACE

#include"fs/fat32/fat32.h"
#include"fs/fs_interface.h"
extern struct directory *root_dir;
enum dev_func_
{
    DEV_OPEN,
    DEV_READ,
    DEV_WRITE,
    DEV_LSEEK,
    DEV_CLOSE,
    DEV_FUNC_NR
};

void* fs_open(char *name,uint32 flags);
int fs_read(void* f,char* buff,uint32 sz);
void fs_close(void* f);
uint32 fs_write(void* f,char*buf,uint32 size);
uint32 fs_get_size(void* f);
void fs_lseek(void* f, uint32 offset ,uint8 flag);

//device
//创建设备
void fs_print_root();
fs_interface_t* fs_interface_init(f32 *fs);
#endif