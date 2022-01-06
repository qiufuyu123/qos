/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-12-10 20:55:51
 * @LastEditTime: 2021-12-19 12:26:29
 */
#include"hashmap.h"
#include"mem/malloc.h"
#include"qmath.h"
#include"types.h"
#include"kstdio.h"
int keyToIndex(char* key)
{
    int index , len , i;
    if (key == NULL)return -1;

    len = strlen(key);
    index = (int)key[0];
    for (i = 0; i<len; ++i) {
        index *= 1103515245 + (int)key[i];
    }
    index >>= 27;
    index &= (BUCKETCOUNT - 1);
    printk("really hash:%d",index);
    return index;
}
void initHashTable(table* t)
{
	
    int i;
    if (t == NULL)return;

    for (i = 0; i < BUCKETCOUNT; ++i) {
        t->bucket[i].key = NULL;
        t->bucket[i].value = NULL;
        t->bucket[i].next = NULL;
    }
}

//释放哈希表
void freeHashTable(table* t)
{
    int i;
    entry* e,*ep;
    if (t == NULL)return;
    for (i = 0; i<BUCKETCOUNT; ++i) {
        e = &(t->bucket[i]);
        while (e->next != NULL) {
            ep = e->next;
            e->next = ep->next;
            //free(ep->key);
            free(ep->value);
            free(ep);
        }
    }
}

//向哈希表中插入数据
int insertEntry(table* t , uint32 key , char* value)
{
    int index , vlen1 , vlen2;
    entry* e , *ep;

    if (t == NULL || key == NULL || value == NULL) {
        return -1;
    }

    index = key;
    if (t->bucket[index].key == NULL) {
        t->bucket[index].key = key;
        t->bucket[index].value = value;
    }
    else {
        e = ep = &(t->bucket[index]);
        while (e != NULL) { //先从已有的找
            if (e->key == key) {
                //找到key所在，替换值
                //vlen1 = strlen(value);
                //vlen2 = strlen(e->value);
               /* if (vlen1 > vlen2) {
                    free(e->value);
                    e->value = (char*)malloc(vlen1 + 1);
                }*/
                e->value=value;
                return index;   //插入完成了
            }
            ep = e;
            e = e->next;
        } // end while(e...

        //没有在当前桶中找到
        //创建条目加入
        e = (entry*)malloc(sizeof (entry));
        e->key = key;
        e->value = value;
        e->next = NULL;
        ep->next = e;
    }
    return index;
}
//在哈希表中查找key对应的value
//找到了返回value的地址，没找到返回NULL
const char* findValueByKey(const table* t , uint32 key)
{
    int index;
    const entry* e;
    if (t == NULL || key == NULL) {
        return NULL;
    }
    index = key;
    e = &(t->bucket[index]);
    if (e->key == NULL) return NULL;//这个桶还没有元素
    while (e != NULL) {
        if (key==e->key) {
            return e->value;    //找到了，返回值
        }
        e = e->next;
    }
    return NULL;
}
//在哈希表中查找key对应的entry
//找到了返回entry，并将其从哈希表中移除
//没找到返回NULL
entry* removeEntry(table* t , uint32 key)
{
    int index;
    entry* e,*ep;   //查找的时候，把ep作为返回值
    if (t == NULL || key == NULL) {
        return NULL;
    }
    index = key;
    e = &(t->bucket[index]);
    while (e != NULL) {
        if (key==e->key) {
            //如果是桶的第一个
            if (e == &(t->bucket[index])) {
                //如果这个桶有两个或以上元素
                //交换第一个和第二个，然后移除第二个
                ep = e->next;
                if (ep != NULL) {
                    entry tmp = *e; //做浅拷贝交换
                    *e = *ep;//相当于链表的头节点已经移除
                    *ep = tmp;  //这就是移除下来的链表头节点
                    ep->next = NULL;
                }
                else {//这个桶只有第一个元素
                    ep = (entry*)malloc(sizeof(entry));
                    *ep = *e;
                    e->key = e->value = NULL;
                    e->next = NULL;
                }
            }
            else {
                //如果不是桶的第一个元素
                //找到它的前一个(这是前面设计不佳导致的多余操作)
                ep = &(t->bucket[index]);
                while (ep->next != e)ep = ep->next;
                //将e从中拿出来
                ep->next = e->next;
                e->next = NULL;
                ep = e;
            }
            return ep;
        }// end if(strcmp...
        e = e->next;
    }
    return NULL;
}