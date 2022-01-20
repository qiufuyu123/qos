/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2022-01-11 20:36:17
 * @LastEditTime: 2022-01-18 21:13:33
 */
#include"fs/fs_interface.h"
#include"fs/qufs/interface.h"
#include"qstring.h"
#include"fs/qufs/file.h"
#include"fs/qufs/dir.h"
//#include"fs/fd.h"
#include"kstdio.h"
#include"isr.h"
static qufs_fd_t*new_qufs_fd(qu_file_t*f)
{
    qufs_fd_t*re=calloc(1,sizeof(qufs_fd_t));
    if(!re)return NULL;
    re->is_file=true;
    re->ptr=f;
    return re;
}

static qufs_desc_t*get_self_qufs()
{
    return fs_get_selected_selfdata();
}
#define QUFS__INS get_self_qufs()
int qufs_open(char *name,uint32 flags)
{
    if(name[strlen(name)-1]=='/')
    {
        return ERR_INTER_CANT_OPEN_DIR;
    }
    //int fd=-1;

    //1.检测文件是否存在
    //qu_dir_entry_t* last_dir_file;
    int err;
    qu_file_t*target_file= qu_file_search(QUFS__INS,name,&err);
    if(!target_file)
    {
        printf("NOT FOUND!\n"); 
        if(flags&FA_CREATE_NEW||flags&FA_CREATE_ALWAYS)
        {
            
            target_file= qu_file_create(QUFS__INS,name,&err);
            if(err!=ERR_SUCCESS)
            {
                return ERR_INTER_NO_SUCH_A_FILE;
            }
            //好的，到现在说明已经创建好了文件
            
        }else return ERR_INTER_NO_SUCH_A_FILE;
    }
    if(target_file->bind_fd_idx)
    {
        printf("NONONONON\n");
        return ERR_INTER_DENY;
    }
    intr_disable_loop();
    qufs_fd_t*fd=new_qufs_fd(target_file);
    if(!fd)return ERR_INTER_MEM_FAIL;
    int e;
    int idx= fs_put_fd(fd,FD_FILE,flags,&e);
    target_file->bind_fd_idx=fs_fd2idx(idx);
    printf("target:0x%x bind:%d %s\n",target_file,target_file->bind_fd_idx,target_file->name); 
    intr_enable_loop();
    if(e!=FD_ERR_SUCCESS)
    {
        free(fd);
        return ERR_INTER_FS_ERR;
    }
    return idx;
}
int qufs_read(int fd,char* buff,uint32 sz)
{

}
void qufs_close(int fd)
{

}
uint32 qufs_write(int fd,char*buf,uint32 size)
{

}
uint32 qufs_ftell(int fd)
{

}
void qufs_lseek(int fd, uint32 offset ,uint8 flag)
{

}

bool qufs_install_indevice(qufs_desc_t *fs)
{
    fs_interface_t *face=calloc(1,sizeof(fs_interface_t));
    if(!face)return face;
    face->self_data=fs;
    strcpy(face->info.info,"QUFS A standard fs");
    strcpy(face->info.name,"QUFS");
    face->info.magic=0x1903141;
    face->info.version=0x01;
    face->methods.open=qufs_open;
    fs_set_selected_interface(face);
    printf("addr 0x%x\n",fs_get_selected_interface());
    //while(1);
}