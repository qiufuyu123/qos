/*
 * @Description: io口输入输出
 * @Author: QIUFUYU
 * @Date: 2021-09-10 20:12:12
 * @LastEditTime: 2021-10-16 18:48:59
 */
#ifndef _IO_H
#define _IO_H
#include"types.h"
/* 向端口port写入一个字节*/
 void
outb(short port, char data);

 void
outw(short port, short data);

 char
inb(short port);
 void
insw(uint16 port, void *addr, int cnt);
 void
outsw(uint16 port, const void *addr, int cnt);
#endif