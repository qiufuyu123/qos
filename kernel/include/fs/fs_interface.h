/*
 * @Description: 所有文件系统的接口
 * @Author: QIUFUYU
 * @Date: 2021-12-19 11:20:49
 * @LastEditTime: 2022-01-03 10:27:07
 */
#ifndef _H_FS_INTERFACE
#define _H_FS_INTERFACE
#include"types.h"
typedef struct fs_interface_methods 
{
    //文件系统必须实现的方法
    int (*open)(char *path,char *flags);//打开
    int (*close)(uint32 fd);
    int (*lseek)(uint32 fd,uint32 offset,uint8 flag);//文件指针
    uint32 (*tell)(uint32 fd);
    int (*read)(uint32 fd,char *buf,uint32 len);
    int (*write)(uint32 fd,char *buf,uint32 len);
    int (*del)(uint32 fd);//删除文件
}fs_interface_methods_t;
typedef struct fs_interface_info
{
    uint32 magic;
    char name[32];
    char info[32];
    uint32 version;
}fs_interface_info_t;

typedef struct fs_interface
{
    uint8 *self_data;//文件系统自身数据
    //uint32 data_len;//数据长度
    fs_interface_methods_t methods;//所有实现的方法
    fs_interface_info_t info;
}fs_interface_t;




#endif