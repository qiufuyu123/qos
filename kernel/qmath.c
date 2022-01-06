/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-11-29 22:47:17
 * @LastEditTime: 2021-11-29 22:47:17
 */
#include"qmath.h"
int DIV_ROUND_UP(int a,int b)
{
    if(a<b)return 1;
    int ret=a/b;
    if(a%b)ret++;
    return ret;
}
int div_round_down(int a,int b)
{

}
int div_round_mid(int a,int b)
{

}