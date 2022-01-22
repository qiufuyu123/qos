/*
 * @Description: 最终文件接口
 * @Author: QIUFUYU
 * @Date: 2022-01-03 10:15:48
 * @LastEditTime: 2022-01-21 12:18:56
 */
#include"fs/fs_interface.h"
#include"fs/hdd.h"
#include"qtree.h"
#include"kstdio.h"
#include"task/kthread.h"
#include"mem/malloc.h"
#include"qstring.h"
//#include"isr.h"
qtree_t* fs_tree;
#define TYPE_COMMON 0
#define TYPE_DEV 1
#define TYPE_MOUNTED 2
#define FS_MAX_CNT 5
fs_interface_t general_interface;
fs_interface_t fs_list[5];
mounted_info_t mount_info;
#define FILE_DESC_MAGIC 0xABCC
typedef struct file_desc
{
    //在fs_tree内的文件描述信息
    uint16 magic;

    uint32 pid;//持有文件的进程,为0表示没有被持有
    //bool deny;//是否被打开了
    uint8 type;//文件类型
    int open_times;//打开次数,用于释放算法
    void *f_data;
    void *f_mounted_info;
}file_desc_t;


#define DIR_COMMON 0
static file_desc_t* new_desc(uint8 type,void *data,void *mounted)
{
    file_desc_t* re=calloc(1,sizeof(file_desc_t));
    if(!re)return NULL;
    re->f_data=data;
    re->open_times=1;
    re->pid=running_thread()->pid;
    re->magic=0xABCC;
    re->type=type;
    re->f_mounted_info=mounted;
    return re;

}
static void switch_to_mounted()
{

}
bool check_fd(int fd)
{
    if(fd>=3&&fd<FD_LIST_MAX)return true;
    return false;
}

//读取文件
int gen_read(int fd,char *buf,uint32 len)
{
    if(!check_fd(fd))return -1;
    file_desc_t*des=running_thread()->open_fd[fd];
    des->pid=0;
    mounted_info_t m_info=mount_info;
    if(des->f_mounted_info)
    {
        //当前文件是mounted的，那么调用他自身的算法
        mount_info=*((mounted_info_t*)des->f_mounted_info);
    }
    int ret=mount_info.fs->methods.read(des->f_data,buf,len);
    mount_info=m_info;
    if(ret<0)
    {
        
        return -1;
    }
    return ret;    
}

//写入文件接口
int gen_write(int fd,char *buf,uint32 len)
{
    if(!check_fd(fd))return -1;
    file_desc_t*des=running_thread()->open_fd[fd];
    des->pid=0;
    mounted_info_t m_info=mount_info;
    if(des->f_mounted_info)
    {
        //当前文件是mounted的，那么调用他自身的算法
        mount_info=*((mounted_info_t*)des->f_mounted_info);
    }
    int ret=mount_info.fs->methods.write(des->f_data,buf,len);
    mount_info=m_info;
    if(ret<0)
    {
        
        return -1;
    }
    return ret;
}   

//关闭文件
int gen_close(int fd)
{
    if(!check_fd(fd))return -1;
    file_desc_t*des=running_thread()->open_fd[fd];
    if(des->magic!=FILE_DESC_MAGIC)return -1;
    des->pid=0;
    mounted_info_t m_info=mount_info;
    if(des->f_mounted_info)
    {
        //当前文件是mounted的，那么调用他自身的算法
        mount_info=*((mounted_info_t*)des->f_mounted_info);
    }

    if(mount_info.fs->methods.close(des->f_data)>=0)
    {
        mount_info=m_info;
        return -1;
    }
    mount_info=m_info;
    return 0;
}

//打开文件
int gen_open(char *path,uint32 flags)
{
    //strtok()
    char *token;
    char *file_path=strdup(path);
    bool is_mounted=false;
    file_desc_t*final_des=NULL;
    tree_node_t*cur_node=fs_tree->root_node;
    printf("n");
    if(file_path[0]!='/'||file_path[strlen(file_path)-1]=='/')return -1;
    file_path++;
    //跳过第一个‘/’ open不能用于打开目录
    printf("f:%s\n",file_path);
    //file_path++;
    token=strsplit(&file_path,'/');
    //char *mid=strchr(file_path,'/');
    printf("f:%s\n",file_path);
    //printf("token:%s\n",token);
    mounted_info_t *m_info=NULL;
    while(token)
    {
        if(!strcmp(token,"."))
        {
            // . 表示当前目录
            cur_node=cur_node;//没意义
        }else if(!strcmp(token,".."))
        {
            if(cur_node!=fs_tree->root_node)
            {
                cur_node=cur_node->prev;
            }
        }else
        
        {
            //普通的
            if(strchr(file_path,'/'))
            {
                //字符串中还存在'/'表示还没遍历结束,则当前的token为一个目录
                tree_node_t*dir_node= qtree_find(cur_node,token);
                printf("find !\n");
                if(!dir_node)
                {
                    //目录未打开，则初始化目录
                    //需要写入目录属性
                    if(qtree_add_layer(cur_node,token,DIR_COMMON)!=QTREE_OK)
                    {
                        free(file_path);
                        return -1;
                    }
                }else
                {
                    if(dir_node->data!=DIR_COMMON)
                    {
                        //这个目录被挂载了
                        
                        //保存原来的mount_info
                        mounted_info_t before=mount_info;
                        mount_info=*((mounted_info_t*)dir_node->data);
                        m_info=(mounted_info_t*)dir_node->data;
                        //int dev_idx=dir_node->data;
                        //ata_device_t*dev=get_ata_device_by_id(dev_idx);
                        //if(!)return -1;
                        //ata_set_selected_dev(dev);
                        fs_interface_t*inter=mount_info.fs;
                        if(!inter)
                        {
                            mount_info=before;
                            free(file_path);
                            return -1;
                        }
                        if(inter->methods.open)
                        {
                            mount_info=before;
                            //确认open方法存在
                            //重新组装file_path
                            char *new_path=calloc(1,strlen(file_path)+1);
                            if(!new_path)
                            {
                                mount_info=before;
                                free(file_path);
                                return -1;
                            }
                            new_path[0]='/';
                            strcat(new_path,file_path);
                            void*addr= inter->methods.open(new_path,flags);
                            //mount_info=before;
                            if(!addr)
                            {
                                free(file_path);
                                free(new_path);
                                return -1;
                            }
                            free(new_path);
                            file_desc_t*fd= new_desc(TYPE_MOUNTED,addr,m_info);
                            if(!fd)
                            {
                                free(file_path);
                                return -1;
                            }
                            final_des=fd;
                            is_mounted=true;
                            //到现在，还不能直接添加fd,先确认是否打开过了
                            //有些文件系统的接口不带有判断是否打开过的功能
                            //int free_idx=thread_lookfor_freefd(running_thread());
                        }else 
                        {
                            mount_info=before;
                            return -1;
                        }
                    }
                    //目录已经初始化
                    cur_node=dir_node;
                }
            }else
            {
                //字符串中不存在'/'表示这是最后一个token,即file_path是文件名
                file_desc_t*f_des=qtree_find(cur_node,file_path);
                if(!f_des)
                {
                    //文件未初始化
                    if(!is_mounted)
                    {
                        //默认为fatfs文件系统
                        void *addr=mount_info.fs->methods.open(path,flags);
                        if(!addr)
                        {
                            free(file_path);
                            return -1;
                        }
                        final_des=new_desc(TYPE_COMMON,addr,NULL);
                        if(!final_des)
                        {
                            free(file_path);
                            return -1;
                        }
                    }
                    if(qtree_add_node(cur_node,file_path,final_des)!=QTREE_OK)
                    {
                        free(file_path);
                        free(final_des);
                        return -1;
                    }
                }
                else
                {

                    if(is_mounted)
                    {
                        if(f_des->pid)
                        {
                            free(file_path);
                        //表明已经打开过了
                            free(final_des);
                            return -1;
                        }
                        else
                        {
                            memcpy(f_des,final_des,sizeof(file_desc_t));
                        }
                    }else final_des=f_des;

                    //最后一步，写入进程fd信息

                }
                int idx=thread_lookfor_freefd(running_thread());
                if(idx==-1)
                {
                    free(file_path);
                    //没有空余的fd了
                    return -1;
                }
                running_thread()->open_fd[idx]=final_des;
                free(file_path);
                return idx;
            }
        }
        token=strsplit(&file_path,"/");
    }
    //文件直接在目录下
    //则必为非mounted
    printf("direct open!\n");
    final_des=qtree_find(cur_node,file_path);
    if(!final_des)
    {
        //默认的mount_info 是 fatfs
        void *addr=mount_info.fs->methods.open(path,flags);
        if(!addr)
        {
            free(file_path);
            return -1;
        }
        final_des=new_desc(TYPE_COMMON,addr,NULL);
        if(!final_des)
        {
            free(file_path);
            return -1;
        }
        if(qtree_add_node(cur_node,file_path,final_des)!=QTREE_OK)
        {
            free(file_path);
            free(final_des);
            return -1;
        }
        printf("add node!\n");
    }
    else 
    {
        if(final_des->pid)
        {
            free(file_path);
            return -1;
        }else
        {
            final_des->pid=running_thread()->pid;
        }
    }
    int idx=thread_lookfor_freefd(running_thread());
    if(idx==-1)
    {
        free(file_path);
        //没有空余的fd了
        final_des->pid=0;
        printf("no free fd!\n");
        return -1;
    }
    running_thread()->open_fd[idx]=final_des;
    free(file_path);
    return idx;
}

bool fsinterface_init()
{
    fs_tree=qtree_new();
    if(!fs_tree)return false;
    strcpy(general_interface.info.info,"A kernel fs_interface");
    strcpy(general_interface.info.name,"general fs_interface");
    general_interface.info.version=1;
    general_interface.info.magic=0x10101;
    general_interface.self_data=NULL;
    general_interface.methods.open=gen_open;
    general_interface.methods.close=gen_close;
    general_interface.methods.write=gen_write;
    general_interface.methods.read=gen_read;
    
    return true;
    //mount_info.fs=
}


void *fs_get_selected_selfdata()
{
    return ata_selected_dev->fs->self_data;
}
fs_interface_t*fs_get_selected_interface()
{
    return ata_selected_dev->fs;
}
fs_set_selected_interface(fs_interface_t*fs)
{
    ata_selected_dev->fs=fs;
}