/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-09-09 21:21:22
 * @LastEditTime: 2021-11-28 13:02:51
 */
#include"console.h"
//#include"io.h"
#include"qstring.h"
#include"task/sync.h"
#include"kio.h"
#include"mem/malloc.h"

//双缓冲是个好东西呀
static struct lock console_lock;
uint8 con_x=0,con_y=0;
uint8 bg_color=BLACK,fg_color=WHITE;

#define FB_COMMAND_PORT         0x3D4
#define FB_DATA_PORT            0x3D5

    /* The I/O port commands */
#define FB_HIGH_BYTE_COMMAND    14
#define FB_LOW_BYTE_COMMAND     15
#define DISPLAY_ADDR (uint8*)0x000B8000+VIRTUAL_KERNEL_BASE
uint8 *buff=NULL;
int p=0;
bool is_double_buffer=false;

static void console_acquire()
{
    lock_acquire(&console_lock);
}
static void console_release()
{
    lock_release(&console_lock);
}
static void _move_cursor(unsigned short pos)
{
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,    ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,    pos & 0x00FF);
}
static void _fresh_buffer()
{
    //刷新双缓冲
    memcpy(DISPLAY_ADDR,buff,80*25*2);
}
static void _fresh_screen()
{
    _move_cursor(0);
}
static void _refresh_cursor()
{
    _move_cursor(con_x+con_y*80);
}
void _put_char(char c,uint8 bg,uint8 fc)
{
    console_acquire();
    buff[con_x*2+con_y*80*2]=c;
    buff[con_x*2+con_y *80*2+1]=((bg & 0x0F) << 4) | (fc & 0x0F);
    con_x++;
    if(con_x>=79)
    {
        con_x=0;
        con_y++;
    }
    if(con_y>=24)
    {
        con_y=0;
        _fresh_screen();
    }
    _refresh_cursor();
    console_release();
}
void put_char(char c,uint8 bg,uint8 fc)
{
    _put_char(c,bg,fc);
    _fresh_buffer();
}


void console_set_cursor(uint8 x,uint8 y)
{
    console_acquire();
    con_x=x;
    con_y=y;
    _refresh_cursor();
    console_release();
}

void console_set_color(uint8 bg,uint8 fg)
{
    console_acquire();
    fg_color=fg;
    bg_color=bg;
    console_release();
}
void console_debug(char *str,uint8 type)
{
    if(type==DEBUG_INFO)
    {
        //console_print_str("\n");
        put_char('[',BLACK,LIGHT_GREY);
        console_set_color(BLACK,LIGHT_BLUE);
        console_print_str("INFO");
        put_char(']',BLACK,LIGHT_GREY);
        console_set_color(BLACK,WHITE);
        console_print_str(str);
        console_print_str("  .\n");
    }else if(type==DEBUG_OK)
    {
        //console_print_str("\n");
        put_char('[',BLACK,LIGHT_GREY);
        console_set_color(BLACK,LIGHT_GREEN);
        console_print_str("OK");
        put_char(']',BLACK,LIGHT_GREY);
        console_set_color(BLACK,WHITE);
        console_print_str(str);
        console_print_str("  .\n");
    }else if(type==DEBUG_ERROR)
    {
        //console_print_str("\n");
        put_char('[',BLACK,LIGHT_GREY);
        console_set_color(BLACK,LIGHT_RED);
        console_print_str("ERROR");
        put_char(']',BLACK,LIGHT_GREY);
        console_set_color(BLACK,WHITE);
        console_print_str(str);
        console_print_str("  .\n");
    }else if(type==DEBUG_FAIL)
    {
        //console_print_str("\n");
        put_char('[',BLACK,LIGHT_GREY);
        console_set_color(BLACK,MAGENTA);
        console_print_str("FAIL");
        put_char(']',BLACK,LIGHT_GREY);
        console_set_color(BLACK,WHITE);
        console_print_str(str);
        console_print_str("  .\n");
    }else if(type==DEBUG_PANIC)
    {
        //console_print_str("\n");
        put_char('[',BLACK,LIGHT_GREY);
        console_set_color(BLACK,RED);
        console_print_str("!PANIC!");
        put_char(']',BLACK,LIGHT_GREY);
        console_set_color(BLACK,WHITE);
        console_print_str(str);
        console_print_str("  .\n");
        while (1)
        {
            halt();
        }
        
    }
}
void console_print_hex(uint32 num)
{
    char n[8];
    myitoa(num,n,16);
    console_print_str(n);
}
void console_print_dec(uint32 num)
{
    char n[10];
    myitoa(num,n,10);
    console_print_str(n);
}
void console_print_str(char * str)
{
    console_acquire();
    //put_char('a',bg_color,fg_color);
    for(int i=0;i<strlen(str);i++)
    {
        if(str[i]=='\n')
        {
            if(con_y>=24)
            {
                con_y=0;
                _fresh_screen();
            }
            else con_y++;
            con_x=0;
            _refresh_cursor();
            
        }else
        {
            _put_char(str[i],bg_color,fg_color); 
        }
        
    }
    if(is_double_buffer)
        _fresh_buffer();
    //put_char('j',bg_color,fg_color);
    console_release();
}
void console_clean()
{
    console_acquire();
    console_set_cursor(0,0);
    for(int i=0;i<25;i++)
    {
        //console_set_cursor(0,i);
        for(int j=0;j<80;j++)
        {
            buff[j*2+i*80*2]=' ';
            buff[j*2+i *80*2+1]=((BLACK & 0x0F) << 4) | (BLACK & 0x0F);
        }
        
    }
    console_set_cursor(0,0);
    if(is_double_buffer)
        _fresh_buffer();
    console_release();
}
void console_pre_init()
{
    buff=(uint8*)0x000B8000+VIRTUAL_KERNEL_BASE;
    lock_init(&console_lock);
}
void console_init()
{
    //在这里开启双缓冲
    
    is_double_buffer=true;
    //80*25*2=4000 4page
    buff=malloc_pages(4);
}