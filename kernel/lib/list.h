#ifndef __LIB_KERNEL_LIST_H
#define __LIB_KERNEL_LIST_H
#include "stdint.h"
struct list_elem{
	struct list_elem* prev;
	struct list_elem* next;
};

struct list{
	struct list_elem head;
	struct list_elem tail;
};

typedef bool (list_callback_func)(struct list_elem*, int arg);

void list_init(struct list* list);
void list_insert_before(struct list_elem* target, struct list_elem* elem);
void list_push(struct list* list, struct list_elem* elem);
void list_append(struct list* list, struct list_elem* elem);
void list_remove(struct list_elem* elem);
struct list_elem* list_pop(struct list* list);
bool elem_find(struct list* list, struct list_elem* elem);
bool list_empty(struct list* list);
struct list_elem* list_traversal(struct list* list, list_callback_func function, int arg);
uint32_t list_len(struct list* list);



#endif