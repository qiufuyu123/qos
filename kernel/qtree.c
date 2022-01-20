/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2022-01-18 20:37:16
 * @LastEditTime: 2022-01-19 21:21:28
 */
#include"qtree.h"
#include"mem/malloc.h"
//初始化一个树
static tree_node_t* new_node(void *data)
{
    tree_node_t*re=calloc(1,sizeof(tree_node_t));
    if(!re)return NULL;
    re->data_map=hashmap_new();
    if(!re->data_map)
    {
        free(re);
        return NULL;
    }
    re->next=re->prev=NULL;
    re->data=data;
    return re;
}
qtree_t* qtree_new()
{
    qtree_t *re=calloc(1,sizeof(qtree_t));
    if(!re)return NULL;
    re->node_cnt=1;
    re->root_node=new_node(NULL);
    if(!re->root_node)
    {
        free(re);
        return NULL;
    }
    return re;
}

//向树的一个节点中添加项
int qtree_add_node(tree_node_t*node,char *name,void *addr)
{
    if(!node||!name||!addr)return QTREE_NULL_PARM;
    if(hashmap_put(node->data_map,name,addr)!=MAP_OK)
    {
        return QTREE_HASHMAP_FAIL;
    }
    return QTREE_OK;
}

//添加一个节点层
int qtree_add_layer(tree_node_t*node,char *name,void *data)
{
    if(!node||!name)return QTREE_NULL_PARM;
    tree_node_t *addr=new_node(data);
    if(!addr)return QTREE_MEM_FAIL;
    if(hashmap_put(node->data_map,name,addr)!=MAP_OK)
    {
        return QTREE_HASHMAP_FAIL;
    }
    //绑定
    node->next=addr;
    addr->prev=node;
    return QTREE_OK;
}
void *qtree_find(tree_node_t*node,char *name)
{
    if(!node||!name)return QTREE_NULL_PARM;
    any_t re=NULL;
    hashmap_get(node->data_map,name,&re);
    return re;
}