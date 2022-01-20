/*
 * @Description: 所有文件系统的接口
 * @Author: QIUFUYU
 * @Date: 2021-12-19 11:20:49
 * @LastEditTime: 2022-01-18 22:39:10
 */
#ifndef _H_FS_INTERFACE
#define _H_FS_INTERFACE
#include"types.h"
enum
{
    ERR_INTER_NO_SUCH_A_FILE=-1,
    ERR_INTER_MEM_FAIL=-2,
    
    ERR_INTER_CANT_OPEN_DIR=-3,
    ERR_INTER_FS_ERR=-4,
    ERR_INTER_DENY=-5
};
#define	FA_READ				0x01
#define	FA_WRITE			0x02
#define	FA_OPEN_EXISTING	0x00
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define	FA_OPEN_APPEND		0x30
#define SEEK_SET 2
#define SEEK_END 1
#define SEEK_START 0
typedef struct fs_interface_methods 
{
    //文件系统必须实现的方法
    void* (*open)(char *path,uint8 flags);//打开
    int (*close)(void* f);
    int (*lseek)(void* f,uint32 offset,uint8 flag);//文件指针
    uint32 (*tell)(void* f);
    int (*read)(void* f,char *buf,uint32 len);
    int (*write)(void* f,char *buf,uint32 len);
    int (*del)(void* fd);//删除文件
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
typedef struct mounted_info
{
    fs_interface_t*fs;
    void *data;
}mounted_info_t;
extern mounted_info_t mount_info;
extern fs_interface_t general_interface;
bool fsinterface_init();





void *fs_get_selected_selfdata();
fs_interface_t*fs_get_selected_interface();
fs_set_selected_interface(fs_interface_t*fs);
#endif