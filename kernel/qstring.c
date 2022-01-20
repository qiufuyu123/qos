/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-09-16 21:43:56
 * @LastEditTime: 2022-01-19 21:17:05
 */
#include"qstring.h"
#include"mem/malloc.h"
char *strcpy(char *dest, const char *src)
{
    char *tmp = dest;

    while ((*dest++ = *src++) != '\0')
        /* nothing */;
    return tmp;
}

char *strncpy(char *dest, const char *src, uint32 count)
{
    char *tmp = dest;

    while (count) {
        if ((*tmp = *src) != 0)
            src++;
        tmp++;
        count--;
    }
    return dest;
}

char *myitoa(int num,char *str,int radix) 
{  
	/* 索引表 */ 
	char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; 
	unsigned unum; /* 中间变量 */ 
	int i=0,j,k; 
	/* 确定unum的值 */ 
	if(radix==10&&num<0) /* 十进制负数 */ 
	{ 
		unum=(unsigned)-num; 
		str[i++]='-'; 
	} 
	else unum=(unsigned)num; /* 其他情况 */ 
	/* 逆序 */ 
	do  
	{ 
		str[i++]=index[unum%(unsigned)radix]; 
		unum/=radix; 
	}while(unum); 
	str[i]='\0'; 
	/* 转换 */ 
	if(str[0]=='-') k=1; /* 十进制负数 */ 
	else k=0; 
	/* 将原来的“/2”改为“/2.0”，保证当num在16~255之间，radix等于16时，也能得到正确结果 */ 
	char temp; 
	for(j=k;j<=(i-k-1)/2.0;j++) 
	{ 
		temp=str[j]; 
		str[j]=str[i-j-1]; 
		str[i-j-1]=temp; 
	} 
	return str; 
}
char *strcat(char *dest, const char *src)
{
    char *tmp = dest;

    while (*dest)
        dest++;
    while ((*dest++ = *src++) != '\0')
        ;
    return tmp;
}

int strcmp(const char *cs, const char *ct)
{
    unsigned char c1, c2;

    while (1) {
        c1 = *cs++;
        c2 = *ct++;
        if (c1 != c2)
            return c1 < c2 ? -1 : 1;
        if (!c1)
            break;
    }
    return 0;
}

uint32 strlen(const char *s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
}

void *memset(void *s, int c, uint32 count)
{
    char *xs = (char *)s;

    while (count--)
        *xs++ = c;
    return s;
}
char * strchr (const char * string,int ch)
{
   while (*string && *string != (char)ch)
        string++;
   if (*string == (char)ch)
       return((char *)string);
    return(NULL);
}
void *memcpy(void *dest, const void *src, uint32 count)
{
    char *tmp = (char *)dest;
    const char *s = (char *)src;

    while (count--)
        *tmp++ = *s++;
    return dest;
}
char * strdup(const char *str)

{

   char *p;

   if (!str)

      return(NULL);

   if (p = calloc(1,strlen(str) + 1))

      return(strcpy(p,str));

   return(NULL);

}
int memcmp(const void* buffer1,const void* buffer2,uint32  count)
{
    if(!count)
    {
        return 0;
    }
    // 当比较位数不为0时，且每位数据相等时，移动指针
    while(count-- && *(char*)buffer1 == *(char*)buffer2)
    {
        buffer1 = (char*)buffer1 + 1;    // 转换类型，移动指针
        buffer2 = (char*)buffer2 + 1;
    }
    // 返回超过比较位数之后 比较的大小
    return( *((unsigned char *)buffer1) - *((unsigned char *)buffer2) );   
 
}
char *strsplit(char **str,char c)
{
    char *old=*str;
    char* idx=strchr(*str,c);
    if(!idx)return NULL;
    *idx='\0';
    *str=idx+1;
    return old;
}
char *strtok(char *str,const char *tok)
{
    int i = 0;
    while (str[i] != '\0')
    {
        for (int j = 0; j < strlen(tok); j++)
        {
            if (str[i] == tok[j])
            {
                str[i] = '\0';
                char *re=str;
                //str+=i+1;
                return re;
            }
        }
        i++;
    }
    return NULL;
}
void itoa(uint32 value, char** buf_ptr_addr, uint8 base) {
   uint32 m = value % base;	    // 求模,最先掉下来的是最低位   
   uint32 i = value / base;	    // 取整
   if (i) {			    // 如果倍数不为0则递归调用。
      itoa(i, buf_ptr_addr, base);
   }
   if (m < 10) {      // 如果余数是0~9
      *((*buf_ptr_addr)++) = m + '0';	  // 将数字0~9转换为字符'0'~'9'
   } else {	      // 否则余数是A~F
      *((*buf_ptr_addr)++) = m - 10 + 'A'; // 将数字A~F转换为字符'A'~'F'
   }
}