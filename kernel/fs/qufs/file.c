/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-12-10 21:13:39
 * @LastEditTime: 2022-01-17 20:30:58
 */
#include"fs/qufs/file.h"
#include"fs/qufs/dir.h"

#include"hashmap.h"
#include"mem/malloc.h"
#include"fs/qufs/inode.h"
#include"qmath.h"
#include"kstdio.h"
#include"fs/hdd.h"
#include"qstring.h"
#include"console.h"
//#define _DEBUG_QU_FILE
#ifndef _DEBUG_QU_FILE
#define printk printf_null
#endif

//获取文件名
char *path_get_filename(char *path,int *last_split_sym)
{
    for(int i=strlen(path)-1;i>=0;i--)
    {
        if(path[i]=='/')
        {
            if(last_split_sym)
            {
                *last_split_sym=i;
            }
            return strdup(path[i]+1);
        }
    }
    if(last_split_sym)*last_split_sym=-1;
    return NULL;
}

//获取最近的目录
//例:   /a/b/c.txt 返回 b
char *path_get_lastdir(char *path)
{
    int last=-1;
    char *buf=path_get_filename(path,&last);
    if(!buf)return NULL;
    if(last==-1)
    {
        free(buf);
        return NULL;
    }
    char *ls=path;
    for(int i=last-1;i>=0;i--)
    {
        if(ls[i]=='/')
        {
            free(buf);
            return strdup(ls[i]+1);
        }
    }
    free(buf);
    return NULL;

}
static char *path_parse(char *path_name,char *p_name)
{
    char *ls_name=path_name;
    uint32 i=0;
    if(ls_name[0]=='/')
    {
        while(*(++ls_name)=='/');
    }
    //printf("pause 1");
    char *old_pname=p_name;
    while (*ls_name!='/'&&*ls_name!=0)
    {
        i++;
        if(i>=MAX_FILE_NAME_LEN)
        {
            return NULL;
        }
        *p_name++=*ls_name++;
    }
    //printf("pause2 %s",p_name);
    p_name=old_pname;   
    if(ls_name[0]==0)
    {
        //printf("lsname null");
        return NULL;
    }
    //printf("pname:%s lsname:%s\n",p_name,ls_name);
    bool brk=true;
    while (brk)
    {
    brk=false;
    if(!ls_name)break;
    if(strlen(ls_name)>=2)
    {
        
    if(ls_name[0]=='/'&&ls_name[1]=='.'&&(ls_name[2]=='/'||ls_name[2]=='\0'))
    {
        ls_name+=2;
        //printf("new :%s\n",ls_name);
        brk=true;
    }
    }
    if(strlen(ls_name)>=3)
    { 
    if(ls_name[0]=='/'&&ls_name[1]=='.'&&ls_name[2]=='.'&& (ls_name[3]=='/'||ls_name[3]=='\0'))
    {
        char *ls=(char*)((uint32_t)ls_name+3);
        char *ls_buf=strdup(ls);
        memset(p_name,0,MAX_FILE_NAME_LEN);
        //char *pls_buf=strdup(last_parent);
        //strcpy(p_name,ls_buf);
        //printf("ls buf:%s ?:%s\n",ls_buf,p_name);
        ls_name=path_parse(ls_buf,p_name);
        //printf("ls:%s\n",ls_name);
        free(ls_buf);
        brk=true;
    }
    }
    }
    //printf("ret ls:%s\n",ls_name);
    return ls_name;
    
}

//从路径中获得父路径
// /a/b.txt
//return: /a *child : b.txt
char *get_pdir_path(char *path,char *child)
{
    char *ret=calloc(1,strlen(path));
    memset(ret,0,strlen(path));
    char *src_path=path;
    char *last_split=NULL;
    for(int i=0;i<strlen(src_path);i++)
    {
        if(src_path[i]=='/')last_split=&src_path[i];
        //*ret++=src_path[i];
    }
    if(!last_split)
    {
        free(ret);
        return NULL;
    }
    memcpy(ret,path,(uint32)last_split-(uint32)path+1);
    //ret[strlen(ret)]='/';
    strcpy(child,last_split+1);
    return ret;
}
 char *get_min_path(char *path)
{
    //简化路径 （将../和 ./简化）
    char *ret_path=calloc(1,512);
    char *p_path[64]={0};
    char *ls_path=path;
    ret_path[0]='/';
    while (1)
    {
        /* code */
        char *name=path_parse(ls_path,p_path);
        
        if(!name)
        {
            //printf("in min func null\n");
            if(strlen(p_path)!=0)
            {
                strcat(ret_path,p_path);
                return ret_path;
            }
        }else
        {
            //printf("name:%s p_path:%s cur:%s\n",name,p_path,ret_path);
            strcat(ret_path,p_path);
            strcat(ret_path,"/");
            ls_path=name;
        }
    }
    
}
static qu_file_t*new_file()
{
    qu_file_t*re=calloc(1,sizeof(qu_file_t));
    
    return re;

}

qu_file_t*qu_file_create(qufs_desc_t*fs,char *path,enum QU_FILE_ERR *err)
{
    char file_name[MAX_FILE_NAME_LEN];
    char *pdir_name=get_pdir_path(path,file_name);
    
    if(!pdir_name)
    {
        *err=ERR_BAD_PATH;
        return NULL;
    }
    //console_clean();
    printf("pdir:%s child:%s 0x%x\n",pdir_name,file_name,fs);
    //while(1);
    int32 inode_idx=qu_inode_bitmap_alloc(fs);
    printf("inode:%d\n",inode_idx);
    //inode_idx=qu_inode_bitmap_alloc(fs);
    //printf("inode:%d\n",inode_idx);
    /**
     * 此处操作
     * 首先，先根据inode_bitmap找到一块在inode_list里的
     * 空inode的编号
     * 然后初始化一个inode
     * 最后根据这个inode同步到磁盘
     */
    printk("p1");
    //while(1);
    if(inode_idx==-1)
    {
        free(pdir_name);
        *err=ERR_NULL_INODE;
    }
    qu_inode_t*file_inode=calloc(1,sizeof(qu_inode_t));
    if(!file_inode)
    {
        free(pdir_name);
        ubitmap_unset_page(fs->data_bitmap,inode_idx);
        *err=ERR_NULL_INODE;
        return NULL;
    }
    printk("p2");
    int32 disk_lba=qu_bitmap_alloc(fs);
    if(disk_lba==-1)
    {
        free(pdir_name);
        free(file_inode);
        *err=ERR_NO_DATA_BLOCK;
        return NULL;
    }
    printk("p3");
    qu_inode_init(file_inode,disk_lba,inode_idx);
    printk("p4");
    
    qu_dir_entry_t*ent= qu_dir_new_entry(file_name,inode_idx,FILE_FILE);
    if(!ent)
    {
        free(pdir_name);
        free(file_inode);
        ubitmap_unset_page(fs->data_bitmap,disk_lba-fs->sb->data_start_lba);
        *err=ERR_MEM_FAIL;
        return NULL;
    }
    mem_cleaner_t clean;
    int8 e;
    //qu_file_t *buf;
    qu_file_t*parent_dir= qu_file_search(fs,pdir_name,&e);
    
    if(e!=ERR_SUCCESS)
    {
        *err=e;
        free(pdir_name);
        free(file_inode);
        free(ent);
        ubitmap_unset_page(fs->data_bitmap,disk_lba-fs->sb->data_start_lba);
        return NULL;
    }
    if(!qu_dir_sync_entry(fs,parent_dir,ent,&clean))
    {
        *err=ERR_SYNC_FAIL;
        free(pdir_name);
        free(file_inode);
        //free(parent_dir);
        free(ent);
        ubitmap_unset_page(fs->data_bitmap,disk_lba-fs->sb->data_start_lba);
        mem_cleaner_clean(&clean,false);
        return NULL;
    }
    
    qu_inode_sync(fs,file_inode);
    qu_inode_sync(fs,parent_dir->inode);
    qu_bitmap_sync(fs,inode_idx,QU_BITMAP_INODE);

    
    
    
    //free(pdir_name);
    //free(file_inode);
    //free(parent_dir);
    free(ent);
    //注册文件
    //重新打开,刷新缓冲区
    
    //strcat(pdir_name,"/");
    strcat(pdir_name,file_name);
    e= qu_file_reg(fs,file_inode,pdir_name);
    //printf("reg file:%s\n",pdir_name);
    if(e!=ERR_SUCCESS)
    {
        *err=e;
        free(file_inode);
        return NULL;
    }
    qu_file_t*ret= qu_file_get(fs,pdir_name);
    free(pdir_name);
    if(!ret)
    {
        *err=ERR_GETFILE_FAIL;
        return NULL;
    }
    *err=ERR_SUCCESS;
    return ret;
}


qu_file_t *qu_file_search(qufs_desc_t*fs,char *path,enum QU_FILE_ERR *err)
{
    //lsat_open=NULL;
    if(!strcmp(path,"/")||!strcmp(path,"/.")||!strcmp(path,"/.."))
    {
        //根目录
        return qu_file_get(fs,"/");
    }
    uint32 len=strlen(path);
    if(path[0]!='/'||len<=1)
    {
        return NULL;
    }
    
    //先判断路径是否已经被打开过
    // /a/d1/../d1/./t.txt
    
        //简化路径 （将../和 ./简化）
    char *min_path=get_min_path(path);
    qu_file_t*f_ptr= qu_file_get(fs,path);
    if(f_ptr)
    {
        //已经打开了，直接返回
        printk("file has open!\n");
        return f_ptr;
    }
    char *ret_path=calloc(1,512);
    char *last_ret_path=calloc(1,512);
    char p_path[64]={0};
    char last_p_path[64]={0};
    
    char *ls_path=path;
    ret_path[0]='/';
    //console_clean();
    /**
     * 流程
     * 首先，会检测路径是否已经被打开（取最简路径）
     * 如果没有被打开，则逐一目录打开
     */
    while (1)
    {
        /* code */

        //f_ptr=qu_file_get(fs,ret_path);//逐一查看是否被打开
        //printf("check if is open:%s &%s\n",ret_path,last_ret_path);
        char *name=path_parse(ls_path,p_path);
        
        if(!name)
        {
            //printf("in min func null\n");
            if(strlen(p_path)!=0)
            {
                //printf("has p_path %s to %s\n",ret_path,p_path);
                qu_dir_entry_t*ent= qu_dir_getentry(fs,ret_path,p_path);
                if(!ent)
                {
                    f_ptr=NULL;
                    //printf("P\n");
                    break;
                }
                else{
                    
                    strcat(ret_path,p_path);
                    if(ent->type==FILE_DIR)
                    {
                        strcat(ret_path,"/");
                    }
                    //printf("reg as name:%s ent:%s %d\n",ret_path,ent->file_namep,ent->inode_idx);
                    uint8_t e=qu_file_reg(fs,qu_get_inode(fs,ent->inode_idx),ret_path);
                    if(e!=ERR_SUCCESS)
                    {
                        *err=e;
                        f_ptr=NULL;
                        free(ret_path);
                        free(last_ret_path);
                        return NULL;
                    }
                    //printf("pause1\n");
                    f_ptr=qu_file_get(fs,ret_path);
                    break;
                }
            }else{
                //TODO:说明打开的是某个目录
                //例如 '/'
                //
                if(ret_path[0]=='/'&&ret_path[1]=='\0')
                {
                    //更目录
                    f_ptr= qu_file_get(fs,"/");//根目录必然被打开过
                    break;
                }
                else
                {
                    //printf("no p_path %s to %s\n",last_ret_path,last_p_path);
                    //不是根目录，是某个子目录
                    //不可能未被打开过(之前遍历时打开过)
                    qu_dir_entry_t *ent=qu_dir_getentry(fs,last_ret_path,last_p_path);
                    if(!ent)
                    {
                        f_ptr=NULL;
                        *err=ERR_BAD_PATH;
                        free(ret_path);
                        free(last_ret_path);
                        return NULL;
                    }

                    uint8_t e=qu_file_reg(fs,qu_get_inode(fs,ent->inode_idx),ret_path);
                    if(e!=ERR_SUCCESS)
                    {
                        f_ptr=NULL;
                    }
                    else f_ptr=qu_file_get(fs,ret_path);
                    break;
                }
            }
        }else
        {
            //printf("name:%s p_path:%s cur:%s\n",name,p_path,ret_path);
            //这里需要完成目录打开的工作
            f_ptr=NULL;
            qu_dir_entry_t*ent=qu_dir_getentry(fs,ret_path,p_path);
            if(!ent)break;
            //lsat_open=ent;
            memset(last_p_path,0,64);
            strcpy(last_p_path,p_path);
            strcpy(last_ret_path,ret_path);
            strcat(ret_path,p_path);
            
            strcat(ret_path,"/");
            //规定：所有目录名带'/'
            //例如 /a/ 
            //方便编程
            uint8_t e=qu_file_reg(fs,qu_get_inode(fs,ent->inode_idx),ret_path);
            if(e!=ERR_SUCCESS)
            {
                *err=e;
                free(ret_path);
                free(last_ret_path);
                return NULL;
            }
            //lsat_open=qu_file_get(fs,ret_path);
            //printf("open dir:%s reg file ok\n",ent->file_namep);
            ls_path=name;
        }
    }
    //printf("%x %x\n",ret_path,last_ret_path);
    free(ret_path);
    //printf("2");
    //printf("%x %x\n",ret_path,last_ret_path);
    free(last_ret_path);
    //printf("whatup");
    if(!f_ptr)
    {
        *err=ERR_BAD_PATH;
        //printf("bad:(\n");
        return NULL;
    }
    *err=ERR_SUCCESS;
    return f_ptr;
}
void qu_file_close(qufs_desc_t*fs,char *name)
{
    hashmap_remove(fs->file_map,name);
}
enum QU_FILE_ERR qu_file_reg(qufs_desc_t*fs,qu_inode_t*inode,char *path)
{
    if(!inode)return ERR_NULL_INODE;
    char *name=path;
    any_t ls=NULL;
    printk("file map 0x%x\n",fs->file_map);
    //if(hashmap_get(&fs->file_map,GetHash(name,strlen(path)+1)))return ERR_SUCCESS;
    if(hashmap_get(fs->file_map,name,&ls)==MAP_OK)return ERR_SUCCESS;

    printk("not exsites! %s\n",name);
    qu_file_t*f=new_file();
    f->data.enable=true;
    if(!f)return ERR_MEM_FAIL;
    uint32 sect_cnt=0;
    //console_clean();
    printk("inode info:idx: %d  sect:%d\n",inode->inode_idx,inode->data_sects[0]);
    f->data.data=qu_inode_getdata(fs,inode,&sect_cnt,true);
    if(!f->data.data)
    {
        free(f);
        return ERR_NO_DATA_IN_INODE;
    }
    printk("getting data! %x\n",f->data.data);
    //printbins(f->data.data,32);
    //qu_dir_entry_t *entry=f->data.data;
    //printk("%s %d \n",entry->file_namep,entry->inode_idx);
    //while(1);
    if(!sect_cnt)
    {
        //inode内无data数据
        free(f);
        return ERR_NO_DATA_IN_INODE;
    }
    f->data.data_page_sz=DIV_ROUND_UP(sect_cnt,2);
    if(!f->data.data)
    {
        printk("fail to read data!\n");
        f->data.enable=false;
        f->data.data_page_sz=0;
    }
    memcpy(f->name,name,strlen(name)+1);
    f->inode=inode;
    printk("inode info:idx: %d  sect:%d\n",inode->inode_idx,inode->sz);
    //insertEntry(&fs->file_map,GetHash(name,strlen(name)+1),f);
    if(hashmap_put(fs->file_map,name,f)==MAP_OMEM)
    {
        free(f);
        return ERR_MEM_FAIL;
    }
    printk("success!\n");
    return ERR_SUCCESS;
}

qu_file_t* qu_file_get(qufs_desc_t*fs,char *path)
{
    char *name=path;
    qu_file_t*f=NULL;
    hashmap_get(fs->file_map,name,&f);
    return f;
}

int32_t qu_bitmap_alloc(qufs_desc_t*fs)
{
    int32_t idx=ubitmap_scan(1,fs->data_bitmap);
    if(idx==-1)return -1;
    ubitmap_set_page(fs->data_bitmap,idx);
    return fs->sb->data_start_lba+idx;
}
void qu_bitmap_sync(qufs_desc_t*fs,uint32_t bit_idx,uint8_t bitmap_type)
{
    uint32_t off_sec=bit_idx/4096;
    uint32_t off_sz=off_sec*BLOCK_SIZE;

    uint32 lba;
    uint8 *bitmap_ready;
    switch (bitmap_type)
    {
    case QU_BITMAP_DATA:
        /* code */
        lba=fs->sb->bitmap_lba+off_sec;
        bitmap_ready=fs->data_bitmap->bits+off_sz;
        break;
    
    case QU_BITMAP_INODE:
        lba=fs->sb->inode_bitmap_lba+off_sec;
        bitmap_ready=fs->inode_bitmap->bits+off_sz;
        break;
    }
    ata_write(ata_selected_dev,lba,1,bitmap_ready);
}