/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "fs/fatfs/ff.h"			/* Obtains integer types */
#include "fs/fatfs/diskio.h"		/* Declarations of disk functions */
#include"fs/hdd.h"
#include"kstdio.h"
/* Definitions of physical drive number for each drive */
//#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
//#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
//#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */
#define	DEV_FATFS_SYSDISK 1

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat=STA_NOINIT;
	int result;

	//更具设备号获取硬盘号
	if(get_ata_device_by_id(pdrv))
		stat=0;
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	
	return disk_status(pdrv);
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	if(ata_read(get_ata_device_by_id(pdrv),sector,count,buff)!=-1)
		res=RES_OK;
	else res=RES_PARERR;

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;
	if(ata_write(get_ata_device_by_id(pdrv),sector,count,buff)!=-1)
		res=RES_OK;
	else res=RES_PARERR;

	return res;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;
	ata_device_t *dev=get_ata_device_by_id(pdrv);
	if(!dev)return RES_PARERR;
	switch (cmd)
	{
	case GET_SECTOR_COUNT:
		*(WORD*)buff=dev->size;//一共多少个扇区
		break;
	case GET_SECTOR_SIZE:
		*(WORD*)buff=512;//一个扇区512字节
		break;
	case GET_BLOCK_SIZE:
		*(WORD*)buff=1;//每次读写一个扇区
		break;
	}

	return RES_OK;
}

