/*
 * @Description: cmos驱动的实现
 * @Author: QIUFUYU
 * @Date: 2021-11-28 12:23:27
 * @LastEditTime: 2021-11-28 12:24:33
 */
#include"driver/cmos.h"
#include"kio.h"
static unsigned char ReadCMOS(unsigned char p)
{
	unsigned char data;
	outb(CMOS_INDEX, p);
	data = inb(CMOS_DATA);
	outb(CMOS_INDEX, 0x80);
	return data;
}

 unsigned int CMOS_GetHourHex()
{
	return BCD_HEX(ReadCMOS(CMOS_CUR_HOUR));
}

 unsigned int CMOS_GetMinHex()
{
	return BCD_HEX(ReadCMOS(CMOS_CUR_MIN));
}
 unsigned char CMOS_GetMinHex8()
{
	return BCD_HEX(ReadCMOS(CMOS_CUR_MIN));
}
 unsigned int CMOS_GetSecHex()
{
	return BCD_HEX(ReadCMOS(CMOS_CUR_SEC));
}
 unsigned int CMOS_GetDayOfMonth()
{
	return BCD_HEX(ReadCMOS(CMOS_MON_DAY));
}
 unsigned int CMOS_GetDayOfWeek()
{
	return BCD_HEX(ReadCMOS(CMOS_WEEK_DAY));
}
 unsigned int CMOS_GetMonHex()
{
	return BCD_HEX(ReadCMOS(CMOS_CUR_MON));
}
 unsigned int CMOS_GetYear()
{
	return (BCD_HEX(ReadCMOS(CMOS_CUR_CEN))*100) + \
		BCD_HEX(ReadCMOS(CMOS_CUR_YEAR))+1980;
}