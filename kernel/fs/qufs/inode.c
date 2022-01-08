/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-12-06 21:09:04
 * @LastEditTime: 2022-01-08 15:37:15
 */
#include"fs/qufs/inode.h"
#include"fs/qufs/fs.h"
#include"qstring.h"
#include"fs/hdd.h"
#include"kstdio.h"
#include"qmath.h"

void get_inode_phy_location(qu_inode_phy_location_t*loc,uint32 inode,qufs_desc_t*fs)
{
    uint32 abs_lba=fs->sb->inode_table_lba+inode*sizeof(qu_inode_t)/512;
    uint32 offset_byte=inode*sizeof(qu_inode_t)%512;
    uint32 left_sz=512-offset_byte;
    if(left_sz<sizeof(qu_inode_t))
    {
        loc->is_2_sect=true;
    }else
    {
        loc->is_2_sect=false;
    }
    loc->sec_num=abs_lba;
    loc->sec_offset=offset_byte;
    //printk("lba:%d offset:%d is_2:%d\n",loc->sec_num,loc->sec_offset,loc->is_2_sect);
}
static qu_inode_t*qu_inode_getbyphy(qu_inode_phy_location_t*loc,qufs_desc_t*fs)
{
    qu_inode_t*re=calloc(1,sizeof(qu_inode_t));
    if(!re)return NULL;
    if(loc->is_2_sect)
    {
        char *buf=calloc(1,1024);
        if(!buf)
        {
            free(re);
            return NULL;
        }
        ata_read(ata_selected_dev,loc->sec_num,2,buf);
        memcpy(re,buf+loc->sec_offset,sizeof(qu_inode_t));
        free(buf);
        return re;
    }else
    {
        char *buf=calloc(1,512);
        if(!buf)
        {
            free(re);
            return NULL;
        }
        ata_read(ata_selected_dev,loc->sec_num,1,buf);
        memcpy(re,buf+loc->sec_offset,sizeof(qu_inode_t));
        free(buf);
        return re;
    }
}
qu_inode_t*qu_get_inode(qufs_desc_t*fs ,uint32 idx)
{
    //首先，遍历列表
    //list_traversal()
    list_elem_t*elem=fs->inode_list->head.next;
    //printk("next:%x tail:%x\n",elem,&fs->inode_list->tail);
    qu_inode_t*re=NULL;
    while (elem!=&fs->inode_list->tail)
    {
        //printk("bad search in list! %x %x\n",elem,&fs->inode_list->tail);   
        /* code */
        if((elem2entry(qu_inode_t,inode_tag,elem))->inode_idx==idx)
        {
            re=elem2entry(qu_inode_t,inode_tag,elem);
            break;
        }
        elem=elem->next;
    }
    if(!re)
    {
        //打开过的inode中没有
        qu_inode_phy_location_t loc;
        get_inode_phy_location(&loc,idx,fs);
        //printk("loc :%d %d \n",loc.sec_num,loc.sec_offset);
        re=qu_inode_getbyphy(&loc,fs);
        if(!re)return NULL;
        list_append(fs->inode_list,&re->inode_tag);
        return re;
    }else
    {
        return re;
    }
}
//将inode同步到磁盘
void qu_inode_sync(qufs_desc_t*fs,qu_inode_t*inode)
{
    qu_inode_phy_location_t loc;
    get_inode_phy_location(&loc,inode->inode_idx,fs);
    qu_inode_t tmp_inode;
    memcpy(&tmp_inode,inode,sizeof(qu_inode_t));
    tmp_inode.inode_tag.prev=tmp_inode.inode_tag.next=NULL;
    tmp_inode.open_cnts=0;
    tmp_inode.write_deny=false;
    if(loc.is_2_sect)
    {
        char *buf=calloc(1,2*SECTOR_SIZE);
        ata_read(ata_selected_dev,loc.sec_num,2,buf);
        memcpy(buf+loc.sec_offset,&tmp_inode,sizeof(qu_inode_t));
        ata_write(ata_selected_dev,loc.sec_num,2,buf);
        free(buf);
    }else
    {
        char *buf=calloc(1,1*SECTOR_SIZE);
        ata_read(ata_selected_dev,loc.sec_num,1,buf);
        memcpy(buf+loc.sec_offset,&tmp_inode,sizeof(qu_inode_t));
        ata_write(ata_selected_dev,loc.sec_num,1,buf);
        free(buf);
    }
}
char *qu_inode_getdata(qufs_desc_t*fs,qu_inode_t*inode,uint32 *sect_cnt,bool check_safe)
{
    uint32 abs_lba=inode->data_sects[0];
    uint32 *all_blocks=calloc(1,(12+128)*4);
    if(!all_blocks)return NULL;
    memcpy(all_blocks,inode->data_sects,12*4);
    if(inode->data_sects[12]!=0)
    {
        //有间接引用块
        
        ata_read(ata_selected_dev,inode->data_sects[12],1,all_blocks+12);
        //memcpy(all_blocks+11,ls_buf,)
    }
    //uint32 used_block_cnt=0;
    for (int i = 0; i < 12+128; i++)
    {
        if(all_blocks[i])(*sect_cnt)++;
        else break;
    }
    if(!*sect_cnt)
    {
        free(all_blocks);
        //没有数据，则返回一个512字节的空数据
        *sect_cnt=1;
        return calloc(1,512);
    }
    if(check_safe)
    {
    if(!mem_is_safe(DIV_ROUND_UP(*sect_cnt,4),0.7f))
    {
        printf("rate:%d/%d\n",active_memory.used_page_cnt,active_memory.max_page_cnt);
        free(all_blocks);
        return NULL;
    }
    }
    printk("check mem safe ok!\n sz: %d\n",*sect_cnt);
    char *re=malloc_pages(DIV_ROUND_UP(*sect_cnt,4));
    char *old=re;
    if(!re)
    {
        free(all_blocks);
        return NULL;
    }
    for (int i = 0; i < *sect_cnt; i++)
    {
        printk("read lba:%d\n",all_blocks[i]);
        if(ata_read(ata_selected_dev,all_blocks[i],1,re)<0)
        {
            printk("read err\n");
            while(1);
        }
        
        re+=512;
    }

    free(all_blocks);
    return old;
}