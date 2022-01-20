/*
 * @Description: fatfs文件系统接口
 * @Author: QIUFUYU
 * @Date: 2022-01-17 21:40:50
 * @LastEditTime: 2022-01-18 22:22:42
 */
#ifndef _H_FATFS_INTERFACE
#define _H_FATFS_INTERFACE

#include"types.h"
#include"fs/fs_interface.h"

//device
//创建设备
//void fs_print_root();
fs_interface_t* fatfs_interface_init(int pdrv);

#endif