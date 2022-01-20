/*
 * @Description: 树结构的实现
 * @Author: QIUFUYU
 * @Date: 2022-01-18 20:17:58
 * @LastEditTime: 2022-01-18 21:56:31
 */
#ifndef _H_QTREE
#define _H_QTREE
#include"hashmap.h"

//树结构示意图

//      { hashmap0  }       -----root_node (layer)
//      /     |     \      
//   {map 1}{map 2}{map 3}  -----node 1
//   ..       ..      .. 

//即由链表组成的hashmap链式结构

#define QTREE_OK 0
#define QTREE_MEM_FAIL 1
#define QTREE_EXIST_LAYER 2
#define QTREE_HASHMAP_FAIL 3
#define QTREE_NULL_PARM 4
typedef struct tree_node
{
    //树节点
    struct tree_node *next;
    struct tree_node *prev;
    void *data;//保留数据
    map_t data_map;
}tree_node_t;


typedef struct qtree
{
    uint32_t node_cnt;//节点总数
    //uint32_t layer_cnt;//层个数
    tree_node_t *root_node;//根层
}qtree_t;


//初始化一个树
qtree_t* qtree_new();

//向树的一个节点中添加项
int qtree_add_node(tree_node_t*node,char *name,void *addr);

//添加一个节点层
int qtree_add_layer(tree_node_t*node,char *name,void *data);

void *qtree_find(tree_node_t*node,char *name);
#endif