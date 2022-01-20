/*
 * @Description: inode结构
 * @Author: QIUFUYU
 * @Date: 2021-12-05 11:11:50
 * @LastEditTime: 2022-01-10 21:44:24
 */
#ifndef _H_INODE
#define _H_INODE
#include"types.h"
#include"list.h"
#include"fs/qufs/fs.h"


enum qu_inode_permission
{
    INODE_CAN_READ=1,
    INODE_CAN_WRITE,
    INODE_CAN_EXECUTE
};
enum qu_inode_special_tag
{
    INODE_TAG_COMMON,//普通文件
    INODE_TAG_SYSTEM,//系统文件
    INODE_TAG_PROTECTED,//强制保护文件
    INODE_TAG_TMP,//临时文件
    INODE_TAG_MEM_SWAP //内存交换缓存文件
};
typedef struct qu_inode
{
    uint32 inode_idx;
    uint32 sz;
    uint32 open_cnts;
    bool write_deny;
    uint32 create_time;//创建日期
    uint32 last_edit_time;//最后一次修改日期
    uint32 last_visit_time;//最后一次访问日期

    //TODO:文件权限管理（账户管理模块)

    /*uint8 own_usr_perm;//所有者的权限
    uint8 own_group_perm;//所属用户组权限
    uint8 other_perm;//其他用户权限

    uint32 own_usr_id;//所属用户id
    uint32 own_group_id;//所属用户组id*/
    uint8 special_tag;//特殊标记
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
int32 qu_inode_bitmap_alloc(qufs_desc_t*fs);
void qu_inode_sync(qufs_desc_t*fs,qu_inode_t*inode);
void qu_inode_init(qu_inode_t*inode,uint32 sect_lba,uint32 inode_idx);
char *qu_inode_getdata(qufs_desc_t*fs,qu_inode_t*inode,uint32 *sect_cnt,bool check_safe);
#endif