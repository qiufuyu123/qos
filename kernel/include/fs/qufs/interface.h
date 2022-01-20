/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2022-01-11 20:36:35
 * @LastEditTime: 2022-01-16 19:19:46
 */
#ifndef _QUFS_INTERFACE
#define _QUFS_INTERFACE
#include"types.h"
#include"fs/qufs/fs.h"
typedef struct qufs_fd
{
    void* ptr;
    bool is_file;//文件还是设备
    
}qufs_fd_t;

int qufs_open(char *name,uint32 flags);
int qufs_read(int fd,char* buff,uint32 sz);
void qufs_close(int fd);
uint32 qufs_write(int fd,char*buf,uint32 size);
uint32 qufs_ftell(int fd);
void qufs_lseek(int fd, uint32 offset ,uint8 flag);


bool qufs_install_indevice(qufs_desc_t*fs);

#endif