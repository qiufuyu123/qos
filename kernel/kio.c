/*
 * @Description: kio
 * @Author: QIUFUYU
 * @Date: 2021-10-16 18:49:21
 * @LastEditTime: 2021-10-16 18:49:22
 */
#include"kio.h"
void
outb(short port, char data)
{
  asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

 void
outw(short port, short data)
{
  asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

 char
inb(short port)
{
  char data;

  asm volatile("in %1,%0" : "=a" (data) : "d" (port));
  return data;
}
 void
insw(uint16 port, void *addr, int cnt)
{
  asm volatile("cld; rep insw" :
               "=D" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "memory", "cc");
}

 void
outsw(uint16 port, const void *addr, int cnt)
{
  asm volatile("cld; rep outsw" :
               "=S" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "cc");
}