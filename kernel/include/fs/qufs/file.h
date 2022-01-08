/*
 * @Description: qufs的文件（目录）
 * @Author: QIUFUYU
 * @Date: 2021-12-10 20:51:55
 * @LastEditTime: 2022-01-07 22:50:36
 */
#ifndef _H_QUFILE
#define _H_QUFILE
//#include"fs/qufs/dir.h"
#include"types.h"
#include"fs/qufs/fs.h"
#include"fs/qufs/inode.h"
enum
{
    QU_BITMAP_BITMAP,
    QU_BITMAP_INODE,
    QU_BITMAP_DATA
};
typedef struct qu_file_data
{
    char *data;
    uint32 data_page_sz;
    bool enable;
}qu_file_data_t;

typedef struct qu_file
{
    char name[MAX_FILE_NAME_LEN];
    qu_file_data_t data;
    qu_inode_t*inode;
}qu_file_t;
enum QU_FILE_ERR
{
    ERR_SUCCESS=0,
    ERR_NO_DATA_IN_INODE=1,
    ERR_MEM_FAIL,
    ERR_HASH_FAIL,
    ERR_NULL_INODE,
    ERR_BAD_PATH
};
//char *qu_long2short(char *name,uint32 len);
enum QU_FILE_ERR qu_file_reg(qufs_desc_t*fs,qu_inode_t*inode,char *name);
qu_file_t* qu_file_get(qufs_desc_t*fs,char *name);
int32_t qu_bitmap_alloc(qufs_desc_t*fs);
void qu_bitmap_sync(qufs_desc_t*fs,uint32_t bit_idx,uint8_t bitmap_type);
char *get_min_path(char *path);
qu_file_t *qu_file_search(qufs_desc_t*fs,char *path,enum QU_FILE_ERR *err);
#endif