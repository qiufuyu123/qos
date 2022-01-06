/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-12-05 11:22:56
 * @LastEditTime: 2022-01-02 22:22:32
 */
#include"fs/qufs/fs.h"
#include"fs/hdd.h"
#include"fs/qufs/inode.h"
#include"fs/qufs/super_block.h"
#include"qmath.h"
#include"console.h"
#include"kstdio.h"
#include"fs/qufs/dir.h"
#include"fs/qufs/file.h"
#include"mem/malloc.h"
#include"qstring.h"
#include"mem/ubitmap.h"
#include"mem/cleaner.h"
bool qufs_mount(qufs_desc_t*fs)
{
    fs->sb=calloc(1,sizeof(qu_super_block_t));
    if(!fs->sb)return false;
    fs->inode_list=calloc(1,sizeof(list_t));
    if(!fs->inode_list)
    {
        free(fs->sb);
        //free(fs->dir_list);
        return false;
    }
    //printk("pause 1\n");
    //while(1);
    if(ata_read(ata_selected_dev,0,1,fs->sb)<0)
    {
        printk("fail to read ata\n");
        //free(fs->data_bitmap);
        //free(fs->inode_bitmap);
        return false;
    }

    //检查魔数
    if(fs->sb->magic!=MAGIC_NUM)
    {
        printk("error magic_num:%x\n",fs->sb->magic);
        return false;
    }
    fs->data_bitmap=ubitmap_init(fs->sb->bitmap_sect_num*512);
    printk("check magic ok!\n");
    
    if(!fs->data_bitmap)
    {
        return false;
    }
    fs->inode_bitmap=ubitmap_init(fs->sb->inode_bitmap_sect_num*512);
    if(!fs->inode_bitmap)
    {
        return false;
    }

    if(ata_read(ata_selected_dev,fs->sb->bitmap_lba,fs->sb->bitmap_sect_num,fs->data_bitmap->bits)<0)
    {
        return false;
    }
    if(ata_read(ata_selected_dev,fs->sb->inode_bitmap_lba,fs->sb->inode_bitmap_sect_num,fs->inode_bitmap->bits)<0)
    {
        return false;
    }
    list_init(fs->inode_list);
    printk("next:%x tail:%x\n",fs->inode_list->head.next,&fs->inode_list->tail);
    //list_init(fs->dir_list);
    initHashTable(&fs->file_map);
    return true;
}
qufs_desc_t* init_qufs()
{
    qufs_desc_t*re=malloc(sizeof(qufs_desc_t));
    if(!re)return NULL;
    
    uint32 sect_sz_without_sb=ata_selected_dev->size-1;
    //uint32 inode_bitmap_sects=sect_sz_without_sb/10;

    uint32 inode_table_sects=sect_sz_without_sb/10;
    uint32 inodes_cnt=inode_table_sects*512/sizeof(qu_inode_t);
    //while(1);
    uint32 inode_bitmap_sects=inodes_cnt/8/512;
    uint32 block_bitmap_sects=(sect_sz_without_sb-inode_bitmap_sects-inode_table_sects)/8/512;

    qu_super_block_t *sb=malloc(sizeof(qu_super_block_t));
    if(!sb)
    {
        printk("fail to alloc super_block!\n");
        //free(re);
        return NULL;
    }
    sb->magic=0x20201205;
    sb->sect_cnt=ata_selected_dev->size;
    sb->inode_cnt=inodes_cnt;
    sb->lba_start=0;
    sb->bitmap_lba=sb->lba_start+1;
    sb->bitmap_sect_num=block_bitmap_sects;

    sb->inode_bitmap_lba=sb->bitmap_lba+sb->bitmap_sect_num;
    sb->inode_bitmap_sect_num=inode_bitmap_sects;

    sb->inode_table_lba=sb->inode_bitmap_lba+sb->inode_bitmap_sect_num;
    sb->inode_table_sect_num=inode_table_sects;
    
    sb->data_start_lba=sb->inode_table_lba+sb->inode_table_sect_num;
    sb->root_inode_num=0;
    sb->dir_entry_sz=sizeof(qu_dir_entry_t);
    
    console_clean();
    printf("qufs is initing...\n lba_start:%d\nbitmap l&s :%d , %d \ninode_bitmap l&s :%d ,%d \ninode_table l&s :%d ,%d \ndata_l:%d\nsect_max:%d\n" \
    ,sb->lba_start,sb->bitmap_lba,sb->bitmap_sect_num,sb->inode_bitmap_lba,sb->inode_bitmap_sect_num,sb->inode_table_lba,sb->inode_table_sect_num,sb->data_start_lba,ata_selected_dev->size);
printk("sz:%d\n",ata_selected_dev->size);
    //开始写入超级快
    if(ata_write(ata_selected_dev,0,1,sb)<0)
    {
        printk("fail to write super_block!\n");
        return false;
    }

    //uint32 buff_sz=(sb->bitmap_sect_num>=sb->inode_bitmap_sect_num?sb->bitmap_sect_num:sb->inode_bitmap_sect_num);
    uint32 buff_sz=SECTOR_SIZE*4;

    //找出最大的bitmap数据，以此创建缓冲区
    printk("page cnt:%d\n",buff_sz);
    buff_sz=DIV_ROUND_UP(buff_sz,4096);
    printk("page cnt:%d\n",buff_sz);
    //malloc_pages(1525);
    //while(1);
    char *buf=malloc_pages(buff_sz);
    //printk("p1!\n");
    //while(1);
    
    if(!buf)
    {
        printk("fail to alloc buf!\n");
        return false;
    }

    buf[0]=0xE0;//第一位给root目录,第二位给sys(测试用),第三位给new1
    printk("write bitmap!\n");
    if(ata_write(ata_selected_dev,sb->bitmap_lba,sb->bitmap_sect_num,buf)<0)
    {
        free_pages(buf,buff_sz);
        printk("fail to write bitmap back !\n");
        return false;
    }

    //现在，buf变为了inodebitmap
    //但是第一位还是分配给root目录
    //所以不需要动
    printk("write inode_bitmap!\n");
    if(ata_write(ata_selected_dev,sb->inode_bitmap_lba,sb->inode_bitmap_sect_num,buf)<0)
    {
        free_pages(buf,buff_sz);
        printk("fail to write inode bitmap back to disk!\n");
        return false;
    }
    buf[0]=0x00;
    qu_inode_t*inodes=buf;
    inodes->sz=sb->dir_entry_sz*3;
    inodes->inode_idx=0;
    inodes->data_sects[0]=sb->data_start_lba;
    //sizeof(qu_inode_t);
    printk("wirte inode_table!\n");
    if(ata_write(ata_selected_dev,sb->inode_table_lba,1,buf)<0)
    {
        free_pages(buf,buff_sz);
        printk("fail to write back inode table!\n");
        return false;
    }
    printk("set dir!\n");
    memset(buf,0x00,sizeof(qu_inode_t));
    qu_dir_entry_t *dir=buf;
    memcpy(dir->file_namep,".",1);
    printk("dir_name:%s\n",dir->file_namep);
    dir->inode_idx=0;
    dir->type=FILE_DIR;
    dir++;
    memcpy(dir->file_namep,"..",2);
    dir->inode_idx=0;
    dir->type=FILE_DIR;
    dir++;
    memcpy(dir->file_namep,"sys",3);
    dir->inode_idx=1;
    dir->type=FILE_FILE;
    printk("write root ! %d\n",sb->data_start_lba);
    //12243
    //while(1);
    if(ata_write(ata_selected_dev,sb->data_start_lba,1,buf)<0)
    {
        free_pages(buf,buff_sz);
        printk("fail to write root dir_entry!\n");
        return false;
    }
    printk("free pages!\n");
    free_pages(buf,buff_sz);
    if(!qufs_mount(re))
    {
        printk("fail to mount fs\n");
        free(re);
        return NULL;
    }
    console_clean();
    //printk("mount func:0x%x\n",&qufs_mount);
    printk("qufs init ok!!\n");
    printk("root idx:%d sect:%d\n",qu_get_inode(re,0)->inode_idx,qu_get_inode(re,0)->data_sects[0]);
    char b[512];
    ata_read(ata_selected_dev,qu_get_inode(re,0)->data_sects[0],1,b);
    printbins(b,32);
    //console_clean();
    int8 err= qu_file_create(re,qu_get_inode(re,0),"/");
    if(err!=ERR_SUCCESS)
    {
        printk("return err code:%d \n",err);
        console_debug("FS_ERROR!",DEBUG_PANIC);
    }
    printk("ok!\n");
    //while(1);
    //qu_dir_open(re,)
    qu_file_t*f= qu_file_get(re,"/");
    qu_dir_entry_t*entry= qu_dir_getentry(re,"/","sys");
    if(!entry)
    {
        printk("fail to get dir entry !");
    }
    else
    {
        printk("ohhhhhhhhhh!==============\n");
        printk("entry name:%s\n",entry->file_namep);
    }
    printk("now try to new file:\n");
    strcpy(entry->file_namep,"new1");
    //qu_bitmap_alloc(re);
    entry->inode_idx=2;
    entry->type=FILE_FILE;
    mem_cleaner_t *cleaner=mem_cleaner_new();
    if(!cleaner)
    {
        printk("fail to init memcleanr!\n");
        return NULL;
    }
    qu_dir_sync_entry(re,f,entry,cleaner);
    mem_cleaner_clean(cleaner,true);
    entry= qu_dir_getentry(re,"/","new1");
    printk("got file : %s <============\n",entry->file_namep);
    //entry->inode_idx
    //printk("name:%s\n",qu_file_get(re,"sys")->name);
    return re;
}