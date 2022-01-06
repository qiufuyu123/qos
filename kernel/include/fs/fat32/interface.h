/*
 * @Description: 文件接口
 * @Author: QIUFUYU
 * @Date: 2021-10-07 08:25:51
 * @LastEditTime: 2022-01-03 10:30:02
 */
#ifndef _H_INTERFACE
#define _h_INTERFACE

#include"fs/fat32/fat32.h"
#define O_RW 1
#define O_RO 2
#define O_WO 4
#define O_CR 8
#define O_CLEAN 16
#define SEEK_SET 2
#define SEEK_END 1
#define SEEK_START 0
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

int fs_open(char *name,uint32 flags);
int fs_read(int fd,char* buff,uint32 sz);
void fs_close(int fd);
uint32 fs_write(int fd,char*buf,uint32 size);
uint32 fs_get_size(int fd);
void fs_lseek(int fd, uint32 offset ,uint8 flag);

//device
//创建设备
void fs_print_root();
void fs_interface_init(f32 *fs);
#endif