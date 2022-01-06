/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-12-10 21:13:39
 * @LastEditTime: 2022-01-02 22:17:26
 */
#include"fs/qufs/file.h"
#include"fs/qufs/dir.h"

#include"hashmap.h"
#include"mem/malloc.h"
#include"fs/qufs/inode.h"
#include"qmath.h"
#include"kstdio.h"
#include"fs/hdd.h"
#include"qstring.h"
#include"console.h"
//#define _DEBUG_QU_FILE
#ifndef _DEBUG_QU_FILE
#define printk printf_null
#endif
unsigned int GetHash(char* str, unsigned int len)
{
    
   return keyToIndex(str);
}
static char *break_name(char *name,uint32 len)
{
    char *re=calloc(1,len);
    if(!re)return NULL;
    memcpy(re,name,len);
    //先计算hash
    sprintf(re,"%d",GetHash(name,len));
    printk("break_name:%s\n",re);
    return re;
}
char *qu_long2short(char *name,uint32 len)
{
    if(strlen(name)<64)return name;
    char *br=break_name(name,len+1);
    char *re=calloc(0,65);
    sprintf(re,"%s%d",br,GetHash(br,len));
    re[63]='\0';
    re[64]='\0';
    return re;
}
static qu_file_t*new_file()
{
    qu_file_t*re=calloc(1,sizeof(qu_file_t));
    return re;

}
enum QU_FILE_ERR qu_file_create(qufs_desc_t*fs,qu_inode_t*inode,char *path)
{
    char *name=path;
    if(strlen(path)>=64)
    {
        name=qu_long2short(path,strlen(path));
    }
    if(findValueByKey(&fs->file_map,GetHash(name,strlen(path)+1)))return ERR_SUCCESS;
    printk("not exsites! %s %d\n",name,GetHash(name,strlen(path)));
    qu_file_t*f=new_file();
    f->data.enable=true;
    if(!f)return ERR_MEM_FAIL;
    uint32 sect_cnt=0;
    console_clean();
    printk("inode info:idx: %d  sect:%d\n",inode->inode_idx,inode->data_sects[0]);
    f->data.data=qu_inode_getdata(fs,inode,&sect_cnt,true);
    printk("getting data!\n");
    printbins(f->data.data,32);
    qu_dir_entry_t *entry=f->data.data;
    printk("%s %d \n",entry->file_namep,entry->inode_idx);
    //while(1);
    if(!sect_cnt)
    {
        //inode内无data数据
        free(f);
        return ERR_NO_DATA_IN_INODE;
    }
    f->data.data_page_sz=DIV_ROUND_UP(sect_cnt,2);
    if(!f->data.data)
    {
        printk("fail to read data!\n");
        f->data.enable=false;
        f->data.data_page_sz=0;
    }
    memcpy(f->name,name,strlen(name)+1);
    f->inode=inode;
    printk("inode info:idx: %d  sect:%d\n",inode->inode_idx,inode->sz);
    insertEntry(&fs->file_map,GetHash(name,strlen(name)+1),f);
    printk("success!\n");
    return ERR_SUCCESS;
}

qu_file_t* qu_file_get(qufs_desc_t*fs,char *path)
{
    char *name=path;
    if(strlen(path)>=64)
    {
        name=qu_long2short(path,strlen(path));
    }
    printk("hash:%d\n",GetHash(name,strlen(name)+1));
    qu_file_t*f= findValueByKey(&fs->file_map,GetHash(name,strlen(name)+1));
    return f;
}

int32_t qu_bitmap_alloc(qufs_desc_t*fs)
{
    int32_t idx=ubitmap_scan(1,fs->data_bitmap);
    if(idx==-1)return -1;
    ubitmap_set_page(fs->data_bitmap,idx);
    return fs->sb->data_start_lba+idx;
}
void qu_bitmap_sync(qufs_desc_t*fs,uint32_t bit_idx,uint8_t bitmap_type)
{
    uint32_t off_sec=bit_idx/4096;
    uint32_t off_sz=off_sec*BLOCK_SIZE;

    uint32 lba;
    uint8 *bitmap_ready;
    switch (bitmap_type)
    {
    case QU_BITMAP_DATA:
        /* code */
        lba=fs->sb->bitmap_lba+off_sec;
        bitmap_ready=fs->data_bitmap->bits+off_sz;
        break;
    
    case QU_BITMAP_INODE:
        lba=fs->sb->inode_bitmap_lba+off_sec;
        bitmap_ready=fs->inode_bitmap->bits+off_sz;
        break;
    }
    ata_write(ata_selected_dev,lba,1,bitmap_ready);
}