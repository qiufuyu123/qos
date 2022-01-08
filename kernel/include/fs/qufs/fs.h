/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-12-05 11:17:49
 * @LastEditTime: 2022-01-07 22:43:57
 */
#ifndef _H_QUFS
#define _H_QUFS

#define SECTOR_SIZE 512
#define MAGIC_NUM 0x20201205
#define BLOCK_SIZE SECTOR_SIZE
#define MAX_FILE_NAME_LEN 64
#include"types.h"
#include"list.h"
#include"fs/qufs/super_block.h"
#include"mem/ubitmap.h"
#include"hashmap.h"
enum qu_file_type
{
    FILE_UNKNOWN,
    FILE_FILE,
    FILE_DIR
};
typedef struct qufs_desc
{
    qu_super_block_t*sb;
    ubitmap_t*inode_bitmap;
    ubitmap_t*data_bitmap;
    list_t *inode_list;
    //list_t*dir_list;
   map_t file_map;
}qufs_desc_t;
qufs_desc_t* init_qufs();
bool qufs_mount(qufs_desc_t*fs);
#endif