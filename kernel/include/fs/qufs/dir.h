/*
 * @Description: dir结构
 * @Author: QIUFUYU
 * @Date: 2021-12-05 11:14:36
 * @LastEditTime: 2022-01-02 22:15:24
 */
#ifndef _H_DIR
#define _H_DIR
#include"fs/qufs/inode.h"
#include"fs/qufs/fs.h"
#include"fs/qufs/file.h"
#include"mem/cleaner.h"
#include"types.h"



typedef struct qu_dir_entry
{
    char file_namep[MAX_FILE_NAME_LEN];
    uint32 inode_idx;
    enum qu_file_type type;
}qu_dir_entry_t;

qu_dir_entry_t*qu_dir_new_entry(char *name,uint32 inode_idx,uint8 type);
qu_dir_entry_t*qu_dir_getentry(qufs_desc_t*fs,char*pdir ,char *name);

bool qu_dir_open(qufs_desc_t*fs,qu_inode_t*inode,char *pdir_name);
bool qu_dir_sync_entry(qufs_desc_t*fs,qu_file_t*pdir,qu_dir_entry_t*entry,mem_cleaner_t*cleaner);
#endif