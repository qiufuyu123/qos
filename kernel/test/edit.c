/*
 * @Description: edit编辑器测试
 * @Author: QIUFUYU
 * @Date: 2021-10-02 20:35:09
 * @LastEditTime: 2021-10-03 11:23:25
 */
#include"test/edit.h"
#include"mem/malloc.h"
#include"types.h"
#include"keyboard.h"
#include"console.h"
#include"qstring.h"
typedef char* line;
line *texts;//所有文字
//行*列
uint32 cur_h=0,cur_l=0;
uint32 h=0;
void alloc_a_text()
{
    if(texts==NULL)
    {
        texts=malloc(24*sizeof(char*));
        for(int i=h;i<24;i++)
        {
            texts[i]=malloc(80);
            if(texts[i]==NULL)
            {
                console_debug("null!",DEBUG_PANIC);
            }
            memset(texts[i],0,80);
        }
        h+=25;
    }
}
void fresh()
{
    console_clean();
    console_set_cursor(0,0);
    for(int i=0;i<23;i++)
    {
        console_set_cursor(0,i);
        for(int j=0;j<80;j++)
        {
            if(texts[i][j]!=0)
                put_char(texts[i][j],BLACK,WHITE);
            else put_char(' ',BLACK,WHITE);
        }
    }
    console_set_cursor(cur_l,cur_h);
}
void start_edit()
{
    alloc_a_text();
    console_print_str("in edit");
    console_clean();
    console_set_cursor(0,0);
    while (1)
    {
        char k=get_key();
        if(k==0x48)
        {
            //上
            if(cur_h!=0)
            {
                if(strlen(texts[cur_h-1])==0)
                    cur_l=0;
                else cur_l=strlen(texts[cur_h-1]);
                cur_h--;
                console_set_cursor(cur_l,cur_h);
            }
        }
        else if(k==0x50)
        {
            //下
            if(cur_h<22)
            {
                cur_h++;
                if(strlen(texts[cur_h])==0)
                    cur_l=0;
                else cur_l=strlen(texts[cur_h]);
                console_set_cursor(cur_l,cur_h);
            }
        }
        else if(k==0x4b)
        {
            if(cur_l>0)
            {
                cur_l--;
                console_set_cursor(cur_l,cur_h);
            }
        }else if(k==0x4d)
        {
            if(cur_l<79)
            {
                if(strlen(texts[cur_h])>cur_l)
                    cur_l++;
                console_set_cursor(cur_l,cur_h);
            }
        }else if(k>=0x20&&k<=0x7e)
        {
            //如果正好在行尾部输入字符
            //则直接加入
            if(strlen(texts[cur_h])==cur_l)
                texts[cur_h][cur_l]=k;
            else 
            {
                //行中输入字符
                //先将从该字符开始往后移动所有字符

                for(int i=78;i>cur_l;i--)
                {
                    texts[cur_h][i+1]=texts[cur_h][i];
                }
                //再插入
                texts[cur_h][cur_l+1]=texts[cur_h][cur_l];
                texts[cur_h][cur_l]=k;
            }
            cur_l++;
            console_set_cursor(cur_l,cur_h);
            fresh();
        }else if(k=='\b')
        {
            if(cur_l==0&&cur_h>0)
            {
                //在行首退格
                //即将此行合并到上行
                uint32 len=strlen(texts[cur_h-1]);
                memcpy(texts[cur_h-1]+len,texts[cur_h],strlen(texts[cur_h]));

                for(int i=cur_h;i<22;i++)
                {
                    //集体向上移
                    memcpy(texts[i],texts[i+1],80);
                }
                memset(texts[22],0,80);
                cur_l=len;
                cur_h--;
                console_set_cursor(cur_l,cur_h);
                fresh();
            }else if(cur_l==strlen(texts[cur_h])&&cur_l!=0)
            {
                texts[cur_h][cur_l-1]=0;
                cur_l--;
                console_set_cursor(cur_l,cur_h);
                fresh();
            }else if(cur_l>0&&cur_l<strlen(texts[cur_h]))
            {
                //texts[cur_h][cur_l-1]=0;
                for(int i=cur_l-1;i<79;i++)
                {
                    texts[cur_h][i]=texts[cur_h][i+1];
                }
                //texts[cur_h][strlen(texts[cur_h])-1]=0;
                cur_l--;
                console_set_cursor(cur_l,cur_h);
                fresh();
            }
        }
        else if(k==13)
        {
            if(cur_h<22)
            {
                //console_print_dec(strlen(texts[cur_h]));
            if(cur_l==strlen(texts[cur_h])&&strlen(texts[cur_h])!=0)
            {
                
                if(strlen(texts[cur_h+1])==0)
                {
                    cur_h++;
                    cur_l=0;
                    console_set_cursor(cur_l,cur_h);
                }else{
                    //插入中间行
                    for(int i=21;i>cur_h;i--)
                    {
                        //开始集体向后移
                        strcpy(texts[i+1],texts[i]);
                    }
                    memset(texts[cur_h+1],0,80);
                    cur_h++;
                    cur_l=0;
                    console_set_cursor(cur_l,cur_h);
                    fresh();
                }
            }else if(cur_l==0&&strlen(texts[cur_h])!=0)
            {
                for(int i=21;i>cur_h;i--)
                {
                    //开始集体向后移
                    memcpy(texts[i+1],texts[i],80);
                }
                memcpy(texts[cur_h+1],texts[cur_h],80);
                memset(texts[cur_h],0,80);
                cur_h++;
                cur_l=0;
                console_set_cursor(cur_l,cur_h);
                fresh();
            }
            else if(cur_l!=0&&strlen(texts[cur_h])!=0)
            {
                //危，在中间换行...
                for(int i=21;i>cur_h;i--)
                {
                    //开始集体向后移
                    memcpy(texts[i+1],texts[i],80);
                }
                //strcpy(texts[cur_h+1],texts[cur_h]+cur_l);
                memcpy(texts[cur_h+1],texts[cur_h]+cur_l,80-cur_l);
                memset(texts[cur_h]+cur_l,0,80-cur_l);
                cur_h++;
                cur_l=0;
                console_set_cursor(cur_l,cur_h);
                fresh();
            }
            }
        }
    }
    
}