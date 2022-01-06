/*
 * @Description: 键盘设备
 * @Author: QIUFUYU
 * @Date: 2021-10-01 21:13:11
 * @LastEditTime: 2021-10-02 20:36:22
 */
#ifndef _H_KEYBOARD
#define _H_KEYBOARD
#include"ioqueue.h"
extern ioqueue_t kbd_buf;
void init_keyboard();
char get_key();
#endif