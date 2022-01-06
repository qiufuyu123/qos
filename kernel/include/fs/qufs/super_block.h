/*
 * @Description: 管理qufs的超级快
 * @Author: QIUFUYU
 * @Date: 2021-12-05 11:07:14
 * @LastEditTime: 2021-12-05 19:44:12
 */

#ifndef _H_SUPER_BLOCK
#define _H_SUPER_BLOCK
#include"types.h"
struct qu_super_block
{
    uint32 magic;//魔数
    uint32 sect_cnt;//总扇区数
    uint32 inode_cnt;//inode数
    uint32 lba_start;//lba起始

    uint32 bitmap_lba;//bitmap的lba
    uint32 bitmap_sect_num;//bitmap占扇区的数量

    uint32 inode_bitmap_lba;//bitmap的lba
    uint32 inode_bitmap_sect_num;//bitmap占扇区的数量
    
    uint32 inode_table_lba;
    uint32 inode_table_sect_num;

    uint32 data_start_lba;
    uint32 root_inode_num;
    uint32 dir_entry_sz;

    uint8 __never__use[512-13*4];
} __attribute__((packed));

typedef struct qu_super_block qu_super_block_t;


#endif