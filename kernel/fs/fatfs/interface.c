/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2022-01-18 22:21:23
 * @LastEditTime: 2022-01-19 21:58:54
 */
#include"fs/fatfs/interface.h"
#include"qstring.h"
#include"kstdio.h"
#include"mem/malloc.h"
#include"fs/fatfs/ff.h"
#include"console.h"
#define FS_MAGIC 0xFAF4
static void itoa_single(int val, char *c)
{
    if(val==0)*c='0';
    else if(val==1)*c='1';
    else if(val==2)*c='2';
    else if(val==3)*c='3';
    else if(val==4)*c='4';
    else if(val==5)*c='5';
    else if(val==6)*c='6';
    else if(val==7)*c='7';
    else if(val==8)*c='8';
    else if(val==9)*c='9';
}
static void* fs_open(char *name,uint32 flags)
{
    printf("pause open! %x %s\n",mount_info.fs->info.magic,name);
    if(mount_info.fs->info.magic!=FS_MAGIC)return NULL;
    printf("pause 2");
    int pdrv=mount_info.data;
    printf("pause 3");
    //console_clean();
    FIL *fp=malloc(sizeof(FIL));
    printf("init fp");
    if(!fp)return NULL;
    char *new_path=calloc(1,strlen(name)+2);
    printf("seems to be okay! name:%s\n",name);

    memcpy(new_path+1,name,strlen(name));
    
    if(pdrv>=10)
    {
        free(fp);
        free(new_path);
        return NULL;
    }
    itoa_single(pdrv,&new_path[0]);
    new_path[1]=':';
    printf("new path:%s\n",new_path);
    FRESULT res=f_open(fp,new_path,flags);
    if(res!=FR_OK)
    {
        printf("RET:%d\n",res);
        free(fp);
        return NULL;
    }
    printf("RET:%d\n",res);
    return fp;
}
static int fs_read(void* f,char* buff,uint32 sz)
{

}
static void fs_close(void* fd)
{

}
static uint32 fs_write(void* f,char*buf,uint32 size)
{

}
static uint32 fs_get_size(void* f)
{

}
static void fs_lseek(void* f, uint32 offset ,uint8 flag)
{
    
}

//device
//创建设备
//void fs_print_root();
fs_interface_t* fatfs_interface_init(int pdrv)
{
    fs_interface_t* fs=calloc(1,sizeof(fs_interface_t));
    if(!fs)return NULL;
    strcpy(fs->info.name,"FATFS");
    strcpy(fs->info.info,"FATFS a general fs");
    fs->info.magic=FS_MAGIC;
    fs->info.version=1;
    fs->methods.open=fs_open;
    FATFS *sys_fs=calloc(1,sizeof(FATFS));
    if(!sys_fs)
    {
        printf("MEM FAIL TO ALLOC FOR FATFS!");
        while(1);
    }
    FRESULT res=f_mount(sys_fs,"0:",1);
    char *buf=malloc_pages(1);
    if(!buf)
    {
        printf("MEM FAIL TO ALLOC A PAGE FOR FATFS!");
        while(1);
    }
    if(res==FR_NO_FILESYSTEM)
    {
        printf("there is no file system in the disk\n now init it!...\n");
        res=f_mkfs("0:",0,buf,4096);
        printf("mkfs res:%d\n",res);
        res=f_mount(NULL,"0:",1);
        res=f_mount(sys_fs,"0:",1);
    }
    if(res!=0)
    {
        printf("FAIL !\n");
        while(1);
    }
    mount_info.data=0;
    mount_info.fs=fs;
    return fs;
}