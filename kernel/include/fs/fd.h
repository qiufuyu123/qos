/*
 * @Description: 文件描述符管理
 * @Author: QIUFUYU
 * @Date: 2021-12-19 10:52:52
 * @LastEditTime: 2022-01-03 09:52:08
 */
#ifndef _H_FD
#define _H_FD
#include"types.h"
#include"fs/qufs/file.h"
typedef enum
{
    FD_NULL,
    FD_FILE,
    FD_MOUNTED,
    FD_DEV
}fd_type;
typedef enum 
{
    FD_FLAG_NULL,
    FD_FLAG_OPEN=1,
    FD_FLAG_CLOSE=1<<1,
    FD_FLAG_WRITE=1<<2,
    FD_FLAG_READ=1<<3,
    FD_FLAG_WR=1<<4,
    FD_FLAG_CRT=1<<5
}fd_flag;
typedef enum 
{
    FD_ERR_SUCCESS,
    FD_ERR_NO_SUCH_FD,
    FD_ERR_MEM_ERR,
    FD_ERR_FULL
}fd_err;
typedef struct fd_desc
{
    uint32 target_ptr;
    fd_type type;
    uint16 flags;
    bool deny;
    uint32 pid;
}fd_desc_t;
typedef struct fd_manager
{
    fd_desc_t*fds;
    uint32 cnt;
    uint32 last_free;//最后一次释放fd时的位置（快速找到空闲项)
}fd_manager_t;

extern fd_manager_t fs_fd_manager;

fd_err fs_init_fdmgr(uint32 cnt);
uint32 fs_put_fd(uint32 ptr,fd_type type,uint16 flags,fd_err *e);//加入fd
void fs_deny_fd(uint32 idx,bool state,fd_err *e);
fd_err fs_del_fd(uint32 idx);
fd_desc_t *fs_get_fd(uint32 idx);
#endif
