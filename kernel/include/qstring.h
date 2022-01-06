/*
 * @Description: string库
 * @Author: QIUFUYU
 * @Date: 2021-09-10 20:39:11
 * @LastEditTime: 2021-10-07 21:24:28
 */
#ifndef _STRING_H
#define _STRING_H
#include"types.h"
int memcmp(const void* buffer1,const void* buffer2,uint32  count);
char *strcpy(char *dest, const char *src);

char *strncpy(char *dest, const char *src, uint32 count);

char *myitoa(int num,char *str,int radix);
char *strcat(char *dest, const char *src);

int strcmp(const char *cs, const char *ct);

uint32 strlen(const char *s);

void *memset(void *s, int c, uint32 count);
void *memcpy(void *dest, const void *src, uint32 count);
char * strdup(const char *str);
#endif