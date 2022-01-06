/*
 * @Description: 内核从硬盘启动程序的功能实现
 * @Author: QIUFUYU
 * @Date: 2021-10-07 08:23:35
 * @LastEditTime: 2021-10-17 11:27:20
 */
#ifndef _H_EXEC
#define _H_EXEC

#include"types.h"

//启动纯二进制文件
int32 exec_elf32(char *path);

#endif