/*
 * @Description: inode结构
 * @Author: QIUFUYU
 * @Date: 2021-12-05 11:11:50
 * @LastEditTime: 2021-12-19 12:04:22
 */
#ifndef _H_INODE
#define _H_INODE
#include"types.h"
#include"list.h"
#include"fs/qufs/fs.h"
typedef struct qu_inode
{
    uint32 inode_idx;
    uint32 sz;
    uint32 open_cnts;
    bool write_deny;
    uint32 data_sects[13];
    /* data */
    list_elem_t inode_tag;
}qu_inode_t;
typedef struct qu_inode_phy_location
{
    bool is_2_sect;//是否跨扇区
    uint32 sec_num;
    uint32 sec_offset;
}qu_inode_phy_location_t;
void get_inode_phy_location(qu_inode_phy_location_t*loc,uint32 inode,qufs_desc_t*fs);
qu_inode_t*qu_get_inode(qufs_desc_t*fs ,uint32 idx);
char *qu_inode_getdata(qufs_desc_t*fs,qu_inode_t*inode,uint32 *sect_cnt,bool check_safe);
#endif