/*
 * @Description: 时钟驱动
 * @Author: QIUFUYU
 * @Date: 2021-11-28 12:26:49
 * @LastEditTime: 2021-11-29 20:28:29
 */
#ifndef _H_CLOCK
#define _H_CLOCK
#include"types.h"
typedef long clock_t;
#define TIME_TO_DATA16(hou, min, sec) ((unsigned short)(((hou&0x1f)<<11)|((min&0x3f)<<5)|((sec/2)&0x1f)))

#define DATA16_TO_TIME_HOU(data) ((unsigned int)((data>>11)&0x1f))
#define DATA16_TO_TIME_MIN(data) ((unsigned int)((data>>5)&0x3f))
#define DATA16_TO_TIME_SEC(data) ((unsigned int)((data&0x1f) *2))

/* 日期和数据互相转换 */
#define DATE_TO_DATA16(yea, mon, day) ((unsigned short)((((yea-1980)&0x7f)<<9)|((mon&0xf)<<5)|(day&0x1f)))

#define DATA16_TO_DATE_YEA(data) ((unsigned int)(((data>>9)&0x7f)+1980))
#define DATA16_TO_DATE_MON(data) ((unsigned int)((data>>5)&0xf))
#define DATA16_TO_DATE_DAY(data) ((unsigned int)(data&0x1f))

/**
 * 时间转换：
 * 1秒（s） = 1000毫秒（ms）
 * 1毫秒（ms） = 1000微秒（us）
 * 1微秒（us）= 1000纳秒（ns）
 * 1纳秒（ns）= 1000皮秒（ps）
 */

struct SystemDate
{
	int second;         /* [0-59] */
	int minute;         /* [0-59] */
	int hour;           /* [0-23] */
	int day;            /* [1-31] */
	int month;          /* [1-12] */
	int year;           /* year */
	int weekDay;        /* [0-6] */
	int yearDay;        /**/
	int isDst;          /* 夏令时[-1,0,1] */
};

extern struct SystemDate systemDate;

extern volatile clock_t systicks;

extern bool clock_is_enabled;
void init_clock();
 void ClockChangeSystemDate();

 void PrintSystemTime();
 void GetLocalTime(struct SystemDate *time);

 unsigned long SysTime(struct tm *tm);

static inline unsigned int DATE2BIN(struct SystemDate td)
{
	unsigned short date = DATE_TO_DATA16(td.year, td.month, td.day);
	unsigned short time =  TIME_TO_DATA16(td.hour, td.minute, td.second);
	return (date << 16) | time;
}
static inline void BIN2DATE(struct SystemDate* date,unsigned int bin)
{
	date->year=DATA16_TO_DATE_YEA(bin>>16);
	date->month=DATA16_TO_DATE_MON(bin>>16);
	date->day=DATA16_TO_DATE_DAY(bin>>16);
	date->hour=DATA16_TO_TIME_HOU(bin);
	date->minute=DATA16_TO_TIME_MIN(bin);
	date->second= DATA16_TO_TIME_SEC(bin);
}
#endif