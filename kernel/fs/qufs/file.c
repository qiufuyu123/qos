/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-12-10 21:13:39
 * @LastEditTime: 2022-01-08 15:42:09
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
qu_file_t *qu_file_search(qufs_desc_t*fs,char *path,enum QU_FILE_ERR *err)
{
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
        printf("file has open!\n");
        return f_ptr;
    }
    char *ret_path=calloc(1,512);
    char *last_ret_path=calloc(1,512);
    char p_path[64]={0};
    char last_p_path[64]={0};
    
    char *ls_path=path;
    ret_path[0]='/';
    console_clean();
    /**
     * 流程
     * 首先，会检测路径是否已经被打开（取最简路径）
     * 如果没有被打开，则逐一目录打开
     */
    while (1)
    {
        /* code */

        //f_ptr=qu_file_get(fs,ret_path);//逐一查看是否被打开
        //printf("check if is open:%s\n",ret_path);
        char *name=path_parse(ls_path,p_path);
        
        if(!name)
        {
            //printf("in min func null\n");
            if(strlen(p_path)!=0)
            {
                printf("has p_path %s to %s\n",ret_path,p_path);
                qu_dir_entry_t*ent= qu_dir_getentry(fs,ret_path,p_path);
                if(!ent)
                {
                    f_ptr=NULL;
                    break;
                }
                else{
                    
                    strcat(ret_path,p_path);
                    if(ent->type==FILE_DIR)
                    {
                        strcat(ret_path,"/");
                    }
                    printf("reg as name:%s ent:%s %d\n",ret_path,ent->file_namep,ent->inode_idx);
                    uint8_t e=qu_file_reg(fs,qu_get_inode(fs,ent->inode_idx),ret_path);
                    if(e!=ERR_SUCCESS)
                    {
                        *err=e;
                        f_ptr=NULL;
                        free(ret_path);
                        free(last_ret_path);
                        return NULL;
                    }
                    printf("pause1\n");
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
                    printf("no p_path %s to %s\n",last_ret_path,last_p_path);
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
            printf("open dir:%s reg file ok\n",ent->file_namep);
            ls_path=name;
        }
    }
    free(ret_path);
    free(last_ret_path);
    if(!f_ptr)
    {
        *err=ERR_BAD_PATH;
        
        return NULL;
    }
    *err=ERR_SUCCESS;
    return f_ptr;
}
enum QU_FILE_ERR qu_file_reg(qufs_desc_t*fs,qu_inode_t*inode,char *path)
{
    if(!inode)return ERR_NULL_INODE;
    char *name=path;
    any_t ls=NULL;
    printf("file map 0x%x\n",fs->file_map);
    //if(hashmap_get(&fs->file_map,GetHash(name,strlen(path)+1)))return ERR_SUCCESS;
    if(hashmap_get(fs->file_map,name,&ls)==MAP_OK)return ERR_SUCCESS;

    printf("not exsites! %s\n",name);
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
    printbins(f->data.data,32);
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