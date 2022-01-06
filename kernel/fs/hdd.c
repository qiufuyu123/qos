/*
 * @Description: 硬盘驱动
 * @Author: QIUFUYU
 * @Date: 2021-10-04 18:42:41
 * @LastEditTime: 2021-12-07 20:41:06
 */
#include"kio.h"
#include"mem/malloc.h"
#include"fs/hdd.h"
#include"console.h"
#include"kstdio.h"
#include"isr.h"
#include"qstring.h"
ata_device_t*ata_selected_dev=NULL;
//#define _HDD_DEBUG
#ifndef _HDD_DEBUG
#define printk printf_null
#endif
/*
#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_ERR          0x1F1
#define ATA_PRIMARY_SECCOUNT     0x1F2
#define ATA_PRIMARY_LBA_LO       0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HI       0x1F5
#define ATA_PRIMARY_DRIVE_HEAD   0x1F6
#define ATA_PRIMARY_COMM_REGSTAT 0x1F7
#define ATA_PRIMARY_ALTSTAT_DCR  0x3F6


#define STAT_ERR  (1 << 0) // Indicates an error occurred. Send a new command to clear it
#define STAT_DRQ  (1 << 3) // Set when the drive has PIO data to transfer, or is ready to accept PIO data.
#define STAT_SRV  (1 << 4) // Overlapped Mode Service Request.
#define STAT_DF   (1 << 5) // Drive Fault Error (does not set ERR).
#define STAT_RDY  (1 << 6) // Bit is clear when drive is spun down, or after an error. Set otherwise.
#define STAT_BSY  (1 << 7) // Indicates the drive is preparing to send/receive data (wait for it to clear).
                           // In case of 'hang' (it never clears), do a software reset.


*/
/**
 * 48-bit LBA read
 *
 * Send 0x40 for the "master" or 0x50 for the "slave" to port 0x1F6: outb(0x1F6, 0x40 | (slavebit << 4))
 * outb (0x1F2, sectorcount high byte)
 * outb (0x1F3, LBA4)
 * outb (0x1F4, LBA5)
 * outb (0x1F5, LBA6)
 * outb (0x1F2, sectorcount low byte)
 * outb (0x1F3, LBA1)
 * outb (0x1F4, LBA2)
 * outb (0x1F5, LBA3)
 * Send the "READ SECTORS EXT" command (0x24) to port 0x1F7: outb(0x1F7, 0x24)
 */
/*
void ata_pio_read48(uint64_t LBA, uint16 sectorcount, uint8 *target) {
    // HARD CODE MASTER (for now)
    outb(ATA_PRIMARY_DRIVE_HEAD, 0x40);                     // Select master
    outb(ATA_PRIMARY_SECCOUNT, (sectorcount >> 8) & 0xFF ); // sectorcount high
    outb(ATA_PRIMARY_LBA_LO, (LBA >> 24) & 0xFF);           // LBA4
    outb(ATA_PRIMARY_LBA_MID, (LBA >> 32) & 0xFF);          // LBA5
    outb(ATA_PRIMARY_LBA_HI, (LBA >> 40) & 0xFF);           // LBA6
    outb(ATA_PRIMARY_SECCOUNT, sectorcount & 0xFF);         // sectorcount low
    outb(ATA_PRIMARY_LBA_LO, LBA & 0xFF);                   // LBA1
    outb(ATA_PRIMARY_LBA_MID, (LBA >> 8) & 0xFF);           // LBA2
    outb(ATA_PRIMARY_LBA_HI, (LBA >> 16) & 0xFF);           // LBA3
    outb(ATA_PRIMARY_COMM_REGSTAT, 0x24);                   // READ SECTORS EXT


    uint8 i;
    for(i = 0; i < sectorcount; i++) {
        // POLL!
        while(1) {
            uint8 status = inb(ATA_PRIMARY_COMM_REGSTAT);
            if(status & STAT_DRQ) {
                // Drive is ready to transfer data!
                break;
            }
        }
        // Transfer the data!
        insw(ATA_PRIMARY_DATA, (void *)target, 256);
        target += 256;
    }

}
*/
/**
 * To outsw sectors in 48 bit PIO mode, send command "WRITE SECTORS EXT" (0x34), instead.
 * (As before, do not use REP OUTSW when writing.) And remember to do a Cache Flush after
 * each outsw command completes.
 */
/*
void ata_pio_outsw48(uint64_t LBA, uint16 sectorcount, uint8 *target) {

    // HARD CODE MASTER (for now)
    outb(ATA_PRIMARY_DRIVE_HEAD, 0x40);                     // Select master
    outb(ATA_PRIMARY_SECCOUNT, (sectorcount >> 8) & 0xFF ); // sectorcount high
    outb(ATA_PRIMARY_LBA_LO, (LBA >> 24) & 0xFF);           // LBA4
    outb(ATA_PRIMARY_LBA_MID, (LBA >> 32) & 0xFF);          // LBA5
    outb(ATA_PRIMARY_LBA_HI, (LBA >> 40) & 0xFF);           // LBA6
    outb(ATA_PRIMARY_SECCOUNT, sectorcount & 0xFF);         // sectorcount low
    outb(ATA_PRIMARY_LBA_LO, LBA & 0xFF);                   // LBA1
    outb(ATA_PRIMARY_LBA_MID, (LBA >> 8) & 0xFF);           // LBA2
    outb(ATA_PRIMARY_LBA_HI, (LBA >> 16) & 0xFF);           // LBA3
    outb(ATA_PRIMARY_COMM_REGSTAT, 0x34);                   // READ SECTORS EXT

    uint8 i;
    for(i = 0; i < sectorcount; i++) {
        // POLL!
        while(1) {
            uint8 status = inb(ATA_PRIMARY_COMM_REGSTAT);
            if(status & STAT_DRQ) {
                // Drive is ready to transfer data!
                break;
            }
            else if(status & STAT_ERR) {
                console_debug("DISK SET ERROR STATUS!",DEBUG_PANIC);
            }
        }
        // Transfer the data!
        outsw(ATA_PRIMARY_DATA, (void *)target, 256);
        target += 256;
    }

    // Flush the cache.
    outb(ATA_PRIMARY_COMM_REGSTAT, 0xE7);
    // Poll for BSY.
    while(inb(ATA_PRIMARY_COMM_REGSTAT) & STAT_BSY) {}
}
*/


/**
 * 记录一下
 * 这个驱动本来只能写lba28的
 * 被我魔改成了lba48的
 * 
 */
#define MAX_ATA_DEVICE 2

static struct ata_device devices[MAX_ATA_DEVICE];
static uint8_t number_of_actived_devices = 0;
static volatile bool ata_irq_called;
static void ata_400ns_delays(struct ata_device *device)
{
	inb(device->io_base + 7);
	inb(device->io_base + 7);
	inb(device->io_base + 7);
	inb(device->io_base + 7);
}

static uint8_t ata_polling(struct ata_device *device)
{
	uint8_t status;

	while (true)
	{
		status = inb(device->io_base + 7);
		if (!(status & ATA_SREG_BSY) || (status & ATA_SREG_DRQ))
			return ATA_POLLING_SUCCESS;
		if ((status & ATA_SREG_ERR) || (status & ATA_SREG_DF))
			return ATA_POLLING_ERR;
	}
}

static uint8_t ata_polling_identify(struct ata_device *device)
{
	uint8_t status;

	while (true)
	{
		status = inb(device->io_base + 7);

		if (status == 0)
			return ATA_IDENTIFY_NOT_FOUND;
		if (!(status & ATA_SREG_BSY))
			// FIXME: MQ 2019-05-29 ATAPI never work (against https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command)
			// if (inb(device.io_base + 4) || inb(device.io_base + 5))
			//   return ATA_IDENTIFY_ERR;
			// else
			break;
	}

	while (true)
	{
		status = inb(device->io_base + 7);

		if (status & ATA_SREG_DRQ)
			break;
		if (status & ATA_SREG_ERR || status & ATA_SREG_DF)
			return ATA_IDENTIFY_ERR;
	}

	return ATA_IDENTIFY_SUCCESS;
}

static int32_t ata_irq(registers_t *regs)
{
	ata_irq_called = true;
	//irq_ack(regs->int_no);
	//return IRQ_HANDLER_CONTINUE;
}

static int32_t ata_wait_irq()
{
	while (!ata_irq_called)
		;
	ata_irq_called = false;

	//return IRQ_HANDLER_CONTINUE;
}

static uint8_t ata_identify(struct ata_device *device)
{
	outb(device->io_base + 6, device->is_master ? 0xA0 : 0xB0);
	ata_400ns_delays(device);

	outb(device->io_base + 2, 0);
	outb(device->io_base + 3, 0);
	outb(device->io_base + 4, 0);
	outb(device->io_base + 5, 0);
	outb(device->io_base + 7, 0xEC);

	uint8_t identify_status = ata_polling_identify(device);
	if (identify_status != ATA_IDENTIFY_SUCCESS)
		return identify_status;

	if (!(inb(device->io_base + 7) & ATA_SREG_ERR))
	{
		
		insw(device->io_base, device->info, 256);
		//printk("%d\n",device->info->lba28Sectors[0]);
        //printk("in info:%d\n",(device->info->lba28Sectors[1]<<16)+device->info->lba28Sectors[0]);
		//while(1);
		uint32 lba48_sz=(device->info->lba48Sectors[1]<<16)+device->info->lba48Sectors[0];
		uint32 lba28_sz=(device->info->lba28Sectors[1]<<16)+device->info->lba28Sectors[0];
		if(lba48_sz)
		{
			device->support_lba48=true;
			device->size=lba48_sz;
		}else
		{
			device->support_lba48=false;
			device->size=lba28_sz;
		}
		return ATA_IDENTIFY_SUCCESS;
	}
	return ATA_IDENTIFY_ERR;
}

static uint8_t atapi_identify(struct ata_device *device)
{
	outb(device->io_base + 6, device->is_master ? 0xA0 : 0xB0);
	ata_400ns_delays(device);

	outb(device->io_base + 7, 0xA1);

	uint8_t identify_status = ata_polling_identify(device);
	if (identify_status != ATA_IDENTIFY_SUCCESS)
		return identify_status;
    
	if (!(inb(device->io_base + 7) & ATA_SREG_ERR))
	{
		uint16_t buffer[256];

		insw(device->io_base, buffer, 256);

		return ATA_IDENTIFY_SUCCESS;
	}
	return ATA_IDENTIFY_ERR;
}

static struct ata_device *ata_detect(uint16_t io_addr1, uint16_t io_addr2, uint8_t irq, bool is_master, char *dev_name)
{
	struct ata_device *device = calloc(1, sizeof(struct ata_device));
	device->io_base = io_addr1;
	device->associated_io_base = io_addr2;
	device->irq = irq;
	device->is_master = is_master;
	device->info=calloc(1,sizeof(struct IdeIdentifyInfo));
	if(device->info==NULL)console_debug("FAIL TO ALLOC A DISK_INFO",DEBUG_PANIC);
	if (ata_identify(device) == ATA_IDENTIFY_SUCCESS)
	{
		printk("ATA: Identified %s", dev_name);
		device->is_harddisk = true;
		device->dev_name = dev_name;
		devices[number_of_actived_devices++] = *device;
		
		return device;
	}
	else if (atapi_identify(device) == ATA_IDENTIFY_SUCCESS)
	{
		printk("ATAPI: Identified %s", dev_name);
		device->is_harddisk = false;
		device->dev_name = "/dev/cdrom";
		devices[number_of_actived_devices++] = *device;
		return device;
	}
	
	return 0;
}
/*
#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_ERR          0x1F1
#define ATA_PRIMARY_SECCOUNT     0x1F2
#define ATA_PRIMARY_LBA_LO       0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HI       0x1F5
#define ATA_PRIMARY_DRIVE_HEAD   0x1F6
#define ATA_PRIMARY_COMM_REGSTAT 0x1F7
#define ATA_PRIMARY_ALTSTAT_DCR  0x3F6
*/
/**
 * 48-bit LBA read
 *
 * Send 0x40 for the "master" or 0x50 for the "slave" to port 0x1F6: outb(0x1F6, 0x40 | (slavebit << 4))
 * outb (0x1F2, sectorcount high byte)
 * outb (0x1F3, LBA4)
 * outb (0x1F4, LBA5)
 * outb (0x1F5, LBA6)
 * outb (0x1F2, sectorcount low byte)
 * outb (0x1F3, LBA1)
 * outb (0x1F4, LBA2)
 * outb (0x1F5, LBA3)
 * Send the "READ SECTORS EXT" command (0x24) to port 0x1F7: outb(0x1F7, 0x24)
 */



int8_t ata_read_48(struct ata_device *device, uint64_t lba, uint16_t n_sectors, uint16_t *buffer)
{
	if(lba>= device->size)return 0;	
	//outb(device->io_base + 6, (device->is_master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
    outb(device->io_base + 6, (device->is_master ? 0x40 : 0x50));
	ata_400ns_delays(device);

	//outb(device->io_base + 1, 0x00);
	//outb(device->io_base + 2, n_sectors);
    outb(device->io_base + 2, (n_sectors>>8)&0xff);

	//outb(device->io_base + 3, (uint8_t)lba);
    outb(device->io_base + 3, (lba>>24)&0xff);
	//outb(device->io_base + 4, (uint8_t)(lba >> 8));
    outb(device->io_base + 4, (lba>>32)&0xff);
	//outb(device->io_base + 5, (uint8_t)(lba >> 16));
    outb(device->io_base + 5, (lba>>40)&0xff);

    outb(device->io_base + 2, n_sectors&0xff);//new
    outb(device->io_base + 3, lba&0xff);
    outb(device->io_base + 4, (lba>>8)&0xff);
    outb(device->io_base + 5, (lba>>16)&0xff);
	//outb(device->io_base + 7, 0x20);
    outb(device->io_base + 7, 0x24);
	if (ata_polling(device) == ATA_POLLING_ERR)
		return -1;

	for (int i = 0; i < n_sectors; ++i)
	{
		insw(device->io_base, buffer + i * 256, 256);
		ata_400ns_delays(device);

		if (ata_polling(device) == ATA_POLLING_ERR)
			return -1;
	}
	return 0;
}
int8_t ata_read_28(struct ata_device *device, uint32 lba, uint8_t n_sectors, uint16_t *buffer)
{
	if(lba>= device->size)return 0;	
	//outb(device->io_base + 6, (device->is_master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
    outb(device->io_base + 6, (device->is_master ? 0xE0 : 0xF0)| ((lba >> 24) & 0x0F));
	ata_400ns_delays(device);

	outb(device->io_base + 1, 0x00);
	outb(device->io_base + 2, n_sectors);
    //outb(device->io_base + 2, (n_sectors>>8)&0xff);

	outb(device->io_base + 3, (uint8_t)lba);
    //outb(device->io_base + 3, (lba>>24)&0xff);
	outb(device->io_base + 4, (uint8_t)(lba >> 8));
    //outb(device->io_base + 4, (lba>>32)&0xff);
	outb(device->io_base + 5, (uint8_t)(lba >> 16));
    //outb(device->io_base + 5, (lba>>40)&0xff);

    //outb(device->io_base + 2, n_sectors&0xff);//new
    //outb(device->io_base + 3, lba&0xff);
    //outb(device->io_base + 4, (lba>>8)&0xff);
    //outb(device->io_base + 5, (lba>>16)&0xff);
	outb(device->io_base + 7, 0x20);
    //outb(device->io_base + 7, 0x24);
	if (ata_polling(device) == ATA_POLLING_ERR)
		return -1;

	for (int i = 0; i < n_sectors; ++i)
	{
		insw(device->io_base, buffer + i * 256, 256);
		ata_400ns_delays(device);

		if (ata_polling(device) == ATA_POLLING_ERR)
			return -1;
	}
	return 0;
}
int8_t ata_setzero(struct ata_device *device,uint64_t lba,uint16_t n_sectors)
{
		if(lba>= device->size)return 0;	
	outb(device->io_base + 6, (device->is_master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
	ata_400ns_delays(device);
    /*
	outb(device->io_base + 1, 0x00);
	outb(device->io_base + 2, n_sectors);
	outb(device->io_base + 3, (uint8_t)lba);
	outb(device->io_base + 4, (uint8_t)(lba >> 8));
	outb(device->io_base + 5, (uint8_t)(lba >> 16));
	outb(device->io_base + 7, 0x30);
    */
	outb(device->io_base + 1, 0x00);
    outb(device->io_base + 2, (n_sectors>>8)&0xff);
    outb(device->io_base + 3, (lba>>24)&0xff);
    outb(device->io_base + 4, (lba>>32)&0xff);
	
    outb(device->io_base + 5, (lba>>40)&0xff);

    outb(device->io_base + 2, n_sectors&0xff);//new
    outb(device->io_base + 3, lba&0xff);
    outb(device->io_base + 4, (lba>>8)&0xff);
    outb(device->io_base + 5, (lba>>16)&0xff);

    outb(device->io_base + 7, 0x34);
	if (ata_polling(device) == ATA_POLLING_ERR)
		return -1;

	for (int i = 0; i < n_sectors; ++i)
	{
		uint16_t *buffer=calloc(1,512);
		outsw(device->io_base, buffer, 256);
		outw(device->io_base + 7, 0xE7);
		ata_400ns_delays(device);
		free(buffer);
		if (ata_polling(device) == ATA_POLLING_ERR)
			return -1;
	}
	return 0;
}
int8_t ata_write_28(struct ata_device *device, uint32_t lba, uint8_t n_sectors, uint16_t *buffer)
{
	if(lba>= device->size)return 0;	
	outb(device->io_base + 6, (device->is_master ? 0xE0 : 0xF0)|((lba>>24)&0x0f));
	ata_400ns_delays(device);
    /*
	outb(device->io_base + 1, 0x00);
	outb(device->io_base + 2, n_sectors);
	outb(device->io_base + 3, (uint8_t)lba);
	outb(device->io_base + 4, (uint8_t)(lba >> 8));
	outb(device->io_base + 5, (uint8_t)(lba >> 16));
	outb(device->io_base + 7, 0x30);
    */
	outb(device->io_base + 1, 0x00);
    outb(device->io_base + 2, n_sectors);
    outb(device->io_base + 3, lba);
    outb(device->io_base + 4, lba>>8);
	
    outb(device->io_base + 5, lba>>16);

    outb(device->io_base + 7, 0x30);
	if (ata_polling(device) == ATA_POLLING_ERR)
		return -1;

	for (int i = 0; i < n_sectors; ++i)
	{
		outsw(device->io_base, buffer + i * 256, 256);
		outb(device->io_base + 7, 0xE7);
		ata_400ns_delays(device);

		if (ata_polling(device) == ATA_POLLING_ERR)
			return -1;
	}
	//printk("write with lba28!\n");
	return 0;
}
int8_t ata_write_48(struct ata_device *device, uint64_t lba, uint16_t n_sectors, uint16_t *buffer)
{
	if(lba>= device->size)return 0;	
	outb(device->io_base + 6, (device->is_master ? 0x40 : 0x50));
	ata_400ns_delays(device);
    /*
	outb(device->io_base + 1, 0x00);
	outb(device->io_base + 2, n_sectors);
	outb(device->io_base + 3, (uint8_t)lba);
	outb(device->io_base + 4, (uint8_t)(lba >> 8));
	outb(device->io_base + 5, (uint8_t)(lba >> 16));
	outb(device->io_base + 7, 0x30);
    */
	//outb(device->io_base + 1, 0x00);
    outb(device->io_base + 2, (n_sectors>>8)&0xff);
    outb(device->io_base + 3, (lba>>24)&0xff);
    outb(device->io_base + 4, (lba>>32)&0xff);
	
    outb(device->io_base + 5, (lba>>40)&0xff);

    outb(device->io_base + 2, n_sectors&0xff);//new
    outb(device->io_base + 3, lba&0xff);
    outb(device->io_base + 4, (lba>>8)&0xff);
    outb(device->io_base + 5, (lba>>16)&0xff);

    outb(device->io_base + 7, 0x34);
	if (ata_polling(device) == ATA_POLLING_ERR)
		return -1;

	for (int i = 0; i < n_sectors; ++i)
	{
		outsw(device->io_base, buffer + i * 256, 256);
		outb(device->io_base + 7, 0xE7);
		ata_400ns_delays(device);

		if (ata_polling(device) == ATA_POLLING_ERR)
			return -1;
	}
	return 0;
}

int8_t atapi_read_48(struct ata_device *device, uint32_t lba, uint8_t n_sectors, uint16_t *buffer)
{
	uint8_t packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	outb(device->io_base + 6, device->is_master ? 0xE0 : 0xF0);
	ata_400ns_delays(device);

	outb(device->io_base + 1, 0);
	outb(device->io_base + 4, (2048 & 0xff));
	outb(device->io_base + 5, 2048 > 8);
	outb(device->io_base + 7, 0xA0);

	ata_polling(device);

	packet[9] = n_sectors;
	packet[2] = (lba >> 0x18) & 0xFF;
	packet[3] = (lba >> 0x10) & 0xFF;
	packet[4] = (lba >> 0x08) & 0xFF;
	packet[5] = (lba >> 0x00) & 0xFF;

	outsw(device->io_base, (uint16_t *)packet, 6);

	ata_wait_irq();
	ata_polling(device);

	for (int i = 0; i < n_sectors; ++i)
	{
		insw(device->io_base, buffer + 256 * i, 256);

		if (ata_polling(device) == ATA_POLLING_ERR)
			return -1;
	}
	return 0;
}
int8_t ata_read(ata_device_t*device,uint64_t lba,uint16 n_sectors,uint16_t*buffer)
{
	if(lba>= 0x10000000)
	{
		if(!device->support_lba48)
			return -1;
	}
	if(!device->support_lba48)
	{
		if(n_sectors<=256)
		{
			return ata_read_28(device,lba,n_sectors,buffer);
		}
		uint16 cnt=n_sectors;
		uint16 lba_offset=0;
		char *ls_buf=buffer;
		while (true)
		{
			if(cnt<=256)
			{
				return ata_read_28(device,lba+lba_offset,cnt,ls_buf);
			}
			if(ata_read_28(device,lba+lba_offset,256,ls_buf)<0)return -1;
			cnt-=256;
			ls_buf+=256*512;
			lba_offset+=256;
		}
		
	}else
	{
		printk("read use lba48!\n");
		return ata_read_48(device,lba,n_sectors,buffer);
	}
}
int8_t ata_write(ata_device_t*device,uint64_t lba,uint16 n_sectors,uint16_t*buffer)
{
	if(lba>= 0x10000000)
	{
		if(!device->support_lba48)
			return -1;
	}
	if(!device->support_lba48)
	{
		if(n_sectors<=256)
		{
			
			return ata_write_28(device,lba,n_sectors,buffer);
		}
		uint16 cnt=n_sectors;
		uint16 lba_offset=0;
		char *ls_buf=buffer;
		while (true)
		{
			if(cnt<=256)
			{
				return ata_write_28(device,lba+lba_offset,cnt,ls_buf);
			}
			if(ata_write_28(device,lba+lba_offset,256,ls_buf)<0)return -1;
			cnt-=256;
			lba_offset+=256;
			ls_buf+=256*512;
			//printk("too large lba28!\n");
		}
		
	}else
	{
		printk("write use lba48!\n");
		return ata_write_48(device,lba,n_sectors,buffer);
	}
}
struct ata_device *get_ata_device(char *dev_name)
{
	for (uint8_t i = 0; i < MAX_ATA_DEVICE; ++i)
	{
		printk("cmp:%s %s 0x%x\n",devices[i].dev_name,dev_name,&devices[i]);
		if (strcmp(devices[i].dev_name, dev_name) == 0)
		{
			printk("ok!\n");
			return &devices[i];
		}
	}
	return NULL;
}
uint8_t ata_init()
{
	printk("ATA: Initializing");

	register_interrupt_handler(IRQ14, ata_irq);
	register_interrupt_handler(IRQ15, ata_irq);

	ata_detect(ATA0_IO_ADDR1, ATA0_IO_ADDR2, IRQ14, true, "/dev/hda");
	ata_detect(ATA0_IO_ADDR1, ATA0_IO_ADDR2, IRQ14, false, "/dev/hdb");
	//ata_detect(ATA1_IO_ADDR1, ATA1_IO_ADDR2, IRQ15, true, "/dev/hdc");
	//ata_detect(ATA1_IO_ADDR1, ATA1_IO_ADDR2, IRQ115, false, "/dev/hdd");

	printk("ATA: DONE");
	return 0;
}
void ata_set_selected_dev(ata_device_t *dev)
{

	printk("select to:%x\n",dev);
	ata_selected_dev=dev;
	printk("select to:%x\n",ata_selected_dev);
}
#ifndef _HDD_DEBUG
#define printk printf
#endif