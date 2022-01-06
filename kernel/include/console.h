/*
 * @Description: 控制台输出
 * @Author: QIUFUYU
 * @Date: 2021-09-09 21:17:09
 * @LastEditTime: 2021-11-28 12:54:52
 */
#ifndef _CONSOLE_H
#define _CONSOLE_H
#include"types.h"
#define VIRTUAL_KERNEL_BASE 0xC0000000
enum CONSOLE_COLOR
{
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    LIGHT_GREY,
    DARK_GREY,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_MAGENTA,
    LIGHT_BROWN,
    WHITE
};
enum CONSOLE_DEBUG_TYPE
{
    DEBUG_INFO,
    DEBUG_OK,
    DEBUG_FAIL,
    DEBUG_ERROR,
    DEBUG_PANIC
};
//void put_char(char c,uint8 bg,uint8 fc);
void console_set_cursor(uint8 x,uint8 y);
void console_set_color(uint8 bg,uint8 fg);
void put_char(char c,uint8 bg,uint8 fc);
void console_print_str(char * str);
void console_print_hex(uint32 num);
void console_print_dec(uint32 num);
void console_debug(char *str,uint8 type);
void console_clean();
void console_pre_init();
void console_init();
#endif