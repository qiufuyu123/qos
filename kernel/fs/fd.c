/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-12-19 11:09:21
 * @LastEditTime: 2022-01-16 19:57:02
 */
#include"fs/fd.h"
#include"task/kthread.h"
#include"mem/malloc.h"
#include"fs/hdd.h"
fd_manager_t fs_fd_manager;
static bool check_fd(uint32 fd)
{
    if(fd<fs_fd_manager.cnt&&fd>0)return true;
    return false;
}
static uint32 fs_find_free(fd_err*e)
{
    for (uint32 i =1;  i < fs_fd_manager.cnt;i++)
    {
        /* code */
        if(!fs_fd_manager.fds[i].deny)
        {
            *e=FD_ERR_SUCCESS;
            return i;
        }

    }
    
}
 fd_err fs_init_fdmgr(uint32 cnt)
{
    fs_fd_manager.cnt=cnt;
    fs_fd_manager.fds=calloc(cnt,sizeof(fd_desc_t));
    if(!fs_fd_manager.fds)return FD_ERR_MEM_ERR;
    fs_fd_manager.last_free=0;
    return FD_ERR_SUCCESS;
}
uint32 fs_put_fd(uint32 ptr, fd_type type,uint16 flags, fd_err *e)
{
    
    fd_err err;
    uint32 idx=fs_find_free(&err);
    if(err!=FD_ERR_SUCCESS)
    {
        *e=FD_ERR_FULL;
        return 0;
    }
    fs_fd_manager.fds[idx].deny=true;
    fs_fd_manager.fds[idx].flags=flags;
    fs_fd_manager.fds[idx].target_ptr=ptr;
    fs_fd_manager.fds[idx].type=type;
    fs_fd_manager.fds[idx].pid=running_thread()->pid;
    fs_fd_manager.fds[idx].fs_magic=ata_selected_dev->fs->info.magic;
    for(int i=3;i<FD_LIST_MAX;i++)
    {
        if(!running_thread()->open_fd[i])
        {
            running_thread()->open_fd[i]=idx;
            *e=FD_ERR_SUCCESS;
            return i;
        }
    }
    fs_fd_manager.fds[idx].deny=false;
    *e=FD_ERR_FULL;
    return 0;
}
void fs_deny_fd(uint32 i,bool state, fd_err *e)
{
    
    uint32 idx=running_thread()->open_fd[i];
    if(!check_fd(idx))
    {
        *e=FD_ERR_NO_SUCH_FD;
        return;
    }
    fs_fd_manager.fds[idx].deny=state;
    *e=FD_ERR_SUCCESS;
    return;
}
 fd_err fs_del_fd(uint32 i)
{
    
    uint32 idx=running_thread()->open_fd[i];
    if(!check_fd(idx))
    {
        return FD_ERR_NO_SUCH_FD;
    }
    fs_fd_manager.fds[idx].deny=false;
    fs_fd_manager.last_free=idx;
    return FD_ERR_SUCCESS;
}
int32 fs_idx2fd(uint32 idx)
{
    for(int i=3;i<running_thread()->fd_cnt;i++)
    {
        if(running_thread()->open_fd[i]==idx)
        {
            return i;
        }
        if(running_thread()->open_fd[i]==0)return -1;
    }
}
uint32 fs_fd2idx(uint32 fd)
{
    return running_thread()->open_fd[fd];
}
fd_desc_t *fs_get_fd(uint32 i)
{
    uint32 idx=running_thread()->open_fd[i];
    if(!check_fd(idx))
    {
        return NULL;
    }
    return &fs_fd_manager.fds[idx];
}