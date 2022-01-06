/*
 * @Description: 列表
 * @Author: QIUFUYU
 * @Date: 2021-09-30 19:42:05
 * @LastEditTime: 2021-09-30 19:43:11
 */
#ifndef _H_LIST
#define _H_LIST
#include "types.h"

#define offset(struct_type,member) (int)(&((struct_type*)0)->member)
#define elem2entry(struct_type, struct_member_name, elem_ptr) \
	 (struct_type*)((int)elem_ptr - offset(struct_type, struct_member_name))

/**********   定义链表结点成员结构   ***********
*结点中不需要数据成元,只要求前驱和后继结点指针*/
typedef struct list_elem {
   struct list_elem* prev; // 前躯结点
   struct list_elem* next; // 后继结点
}list_elem_t;

/* 链表结构,用来实现队列 */
typedef struct list {
/* head是队首,是固定不变的，不是第1个元素,第1个元素为head.next */
   struct list_elem head;
/* tail是队尾,同样是固定不变的 */
   struct list_elem tail;
}list_t;

/* 自定义函数类型function,用于在list_traversal中做回调函数 */
typedef bool (function)(struct list_elem*, int arg);

void list_init (struct list*);
void list_insert_before(struct list_elem* before, struct list_elem* elem);
void list_push(struct list* plist, struct list_elem* elem);
void list_iterate(struct list* plist);
void list_append(struct list* plist, struct list_elem* elem);  
void list_remove(struct list_elem* pelem);
struct list_elem* list_pop(struct list* plist);
bool list_empty(struct list* plist);
uint32 list_len(struct list* plist);
struct list_elem* list_traversal(struct list* plist, function func, int arg);
bool elem_find(struct list* plist, struct list_elem* obj_elem);
#endif