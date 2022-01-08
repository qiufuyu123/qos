/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-12-07 19:42:38
 * @LastEditTime: 2022-01-07 22:27:27
 */
#include"fs/qufs/dir.h"
#include"fs/qufs/file.h"
#include"mem/malloc.h"
#include"kstdio.h"
#include"qstring.h"
#include"qmath.h"
#include"qassert.h"
#include"fs/hdd.h"

#ifndef _DEBUG_QUFS_DIR
#define printk printf_null
#endif
qu_dir_entry_t*qu_dir_new_entry(char *name,uint32 inode_idx,uint8 type)
{
    qu_dir_entry_t*re=calloc(1,sizeof(qu_dir_entry_t));
    if(!re)return NULL;
    //re->file_namep=strdup(name);
    //strcpy(re->file_namep,name);
    memcpy(re->file_namep,name,strlen(name)+1);
    re->inode_idx=inode_idx;
    re->type=type;
    return re;
}
qu_dir_entry_t *qu_dir_getentry(qufs_desc_t*fs,char*pdir ,char *name)
{
    /*qu_dir_t*dir=calloc(1,sizeof(qu_dir_t));   
    if(!dir)return false;
    dir->cnt=inode;
    */

   qu_file_t*f=qu_file_get(fs,pdir);
   printk("get file! %s\n",f->name);
   if(!f)return NULL;
   printk("f_name:%s\n",name);
   if(f->data.enable)
   {
       printk("has data!\n");
       char *buf=f->data.data;
       printbins(buf,32);
       printk("%s\n",buf);
       //while(1);
       qu_dir_entry_t*entry=calloc(SECTOR_SIZE/sizeof(qu_dir_entry_t),sizeof(qu_dir_entry_t));
           if(!entry)
           {
               //free_pages(old,DIV_ROUND_UP(cnt,4));
               return NULL;
           }
       for (int i = 0; i < DIV_ROUND_UP(f->data.data_page_sz,4); i++)
       {
           
           //entry=buf;
           memcpy(entry,buf,SECTOR_SIZE/sizeof(qu_dir_entry_t)*sizeof(qu_dir_entry_t));
           for (int i = 0; i <SECTOR_SIZE/sizeof(qu_dir_entry_t) ; i++)
           {
               if(strcmp(entry[i].file_namep,name)==0)
               {
                   //free(entry);
                   printk("get it !\n");
                   //free_pages(old,DIV_ROUND_UP(cnt,4));
                   //free(f);
                   qu_dir_entry_t*re=calloc(1,sizeof(qu_dir_entry_t));
                   if(!re)
                   {
                       free(entry);
                       return NULL;
                   }
                   memcpy(re,&entry[i],sizeof(qu_dir_entry_t));
                   printf("0x%x\n",re);
                   return re;
               }else
               {
                   printk("cmp :%s to %s\n",entry[i].file_namep,name);
               }
           }
           //free(entry);
           buf+=512;
       }
   }else
   {
       uint32 cnt=0;
       char *buf= qu_inode_getdata(fs,f->inode,&cnt,false);
       char *old=buf;
       if(!buf)return NULL;
        qu_dir_entry_t*entry=calloc(SECTOR_SIZE/sizeof(qu_dir_entry_t),sizeof(qu_dir_entry_t));
       for (int i = 0; i < cnt; i++)
       {
           if(!entry)
           {
               free_pages(old,DIV_ROUND_UP(cnt,4));
               return NULL;
           }
           memcpy(entry,buf,SECTOR_SIZE/sizeof(qu_dir_entry_t)*sizeof(qu_dir_entry_t));
           for (int i = 0; i <SECTOR_SIZE/sizeof(qu_dir_entry_t) ; i++)
           {
               if(strcmp(entry[i].file_namep,name)==0)
               {
                   //free(entry);
                   free_pages(old,DIV_ROUND_UP(cnt,4));
                   //free(f);
                   printk("get it !\n");
                   qu_dir_entry_t *re=calloc(1,sizeof(qu_dir_entry_t));
                   if(!re)
                   {
                    free_pages(old,DIV_ROUND_UP(cnt,4));
                    free(entry);
                    return NULL;   
                   }
                   memcpy(re,&entry[i],sizeof(qu_dir_entry_t));
                   free_pages(old,DIV_ROUND_UP(cnt,4));
                   free(entry);
                   return re;
               }else
               {
                   printk("cmp :%s to %s\n",entry[i].file_namep,name);
               }
           }
           //free(entry);
           buf+=512;
       }
       //free_pages(old,DIV_ROUND_UP(cnt,4));
       free(entry);
       //free(f);
        return NULL;
   }
}

bool qu_dir_open(qufs_desc_t*fs,qu_inode_t*inode,char *pdir_name)
{
    if(qu_file_reg(fs,inode,pdir_name)!=ERR_SUCCESS)
        return false;
    return true;
}


bool qu_dir_sync_entry(qufs_desc_t*fs,qu_file_t*pdir,qu_dir_entry_t*entry,mem_cleaner_t*cleaner)
{
    qu_inode_t*pdir_inode=pdir->inode;
    uint32 dir_sz=pdir_inode->sz;
    uint32 dir_entry_sz=fs->sb->dir_entry_sz;
    ASSERT(dir_sz%dir_entry_sz==0);
    uint32 dir_max_per_sec=(512/dir_entry_sz);
    int32 block_lba=-1;
    uint32 block_idx=0;
    uint32_t all_blocks[140] = {0}; //
    uint32 block_bitmap_idx=-1;
    while (block_idx<12)
    {
        /* code */
        all_blocks[block_idx]=pdir_inode->data_sects[block_idx];
        block_idx++;
    }
    block_idx=0;
    qu_dir_entry_t*ls_dbuf;
            int sect_cnt=0;
            if(pdir->data.enable)
            {
                ls_dbuf=pdir->data.data;
                cleaner->free_addr=NULL;
                cleaner->pg_cnt=0;
            }else
            {
                
                //TODO:留一下这里未进行内存安全性检测
                ls_dbuf=qu_inode_getdata(fs,pdir_inode,&sect_cnt,false);
                if(!ls_dbuf)
                {
                    //free(ls_buf);
                    printk("fail to get inode data!\n");
                    return false;
                }
                cleaner->free_addr=ls_dbuf;
                cleaner->pg_cnt=sect_cnt;
            }
    while (block_idx<140)
    {
        /* code */
        if(all_blocks[block_idx]==0)
        {
            //这里发现没有可用的块了
            block_lba=qu_bitmap_alloc(fs);
            if(block_lba==-1)
            {
                printk("fail to alloc a new bitmap!\n");
                //free_pages()
                return false;
            }
            block_bitmap_idx=block_lba-fs->sb->data_start_lba;
            ASSERT(block_bitmap_idx!=-1);
            qu_bitmap_sync(fs,block_idx,QU_BITMAP_BITMAP);
            block_bitmap_idx=-1;
            if(block_idx<12)
            {
                //直接快
                pdir_inode->data_sects[block_idx]=block_lba;
            }else if(block_idx==12)
            {
                //将这个作为一级快
                pdir_inode->data_sects[block_idx]=block_lba;
                block_lba=-1;
                block_lba=qu_bitmap_alloc(fs);
                if(block_lba==-1)
                {
                    block_bitmap_idx=pdir_inode->data_sects[12]-fs->sb->data_start_lba;
                    ubitmap_unset_page(fs->data_bitmap,block_bitmap_idx);
                    pdir_inode->data_sects[12]=0;
                    printk("fail to alloc a bit!\n");
                    return false;
                }
                block_bitmap_idx=block_lba-fs->sb->data_start_lba;
                ASSERT(block_bitmap_idx!=-1);
                qu_bitmap_sync(fs,block_bitmap_idx,QU_BITMAP_DATA);
                all_blocks[block_idx]=block_lba;
                ata_write(ata_selected_dev,pdir_inode->data_sects[12],1,all_blocks+12);
                qu_dir_entry_t ls;
                memcpy(&ls,entry,sizeof(qu_dir_entry_t));
                ata_write(ata_selected_dev,all_blocks[block_idx],1,&ls);
                pdir_inode->sz+=dir_entry_sz;
                return true;
            }
            
            
        }
        //如果间接块已经存在，则读取
            char *ls_buf=calloc(1,512);
            if(!ls_buf)return false;
            ata_read(ata_selected_dev,all_blocks[block_idx],1,ls_buf);
            uint8 dir_entry_idx=0;
            
            while (dir_entry_idx<dir_max_per_sec)
            {
                if((ls_dbuf+dir_entry_idx)->type==FILE_UNKNOWN)
                {
                    memcpy((ls_dbuf+dir_entry_idx),entry,sizeof(qu_dir_entry_t));
                    ata_write(ata_selected_dev,all_blocks[block_idx],1,ls_buf);
                    return true;
                }
                dir_entry_idx++;
            }
        block_idx++;
    }
    printk("dir is full!\n");
    return false;
    
}