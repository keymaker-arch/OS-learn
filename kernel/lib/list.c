#include "list.h"
#include "interrupt.h"
#include "stdint.h"


void list_init(struct list* list){
	list->head.prev = NULL;
	list->head.next = &list->tail;
	list->tail.prev = &list->head;
	list->tail.next = NULL;
}


void list_insert_before(struct list_elem* target, struct list_elem* elem){
	// atom operation
	enum intr_status status = intr_disable();
	elem->prev = target->prev;
	elem->next = target;
	target->prev->next = elem;
	target->prev = elem;
	
	intr_set_status(status);
}


void list_push(struct list* list, struct list_elem* elem){
	list_insert_before(list->head.next, elem);
}


void list_append(struct list* list, struct list_elem* elem){
	list_insert_before(&list->tail, elem);
}


void list_remove(struct list_elem* elem){
	// atom operation
	enum intr_status status = intr_disable();
	elem->prev->next = elem->next;
	elem->next->prev = elem->prev;
	intr_set_status(status);
}


struct list_elem* list_pop(struct list* list){
	struct list_elem* elem = list->head.next;
	list_remove(elem);
	return elem;
}


bool elem_find(struct list* list, struct list_elem* elem){
	struct list_elem* tmp = list->head.next;
	while(tmp != &list->tail){
		if(elem == tmp){
			return true;
		}
		tmp = tmp->next;
	}
	return false;
}


bool list_empty(struct list* list){
	return list->head.next == &list->tail ? true : false;
}


struct list_elem* list_traversal(struct list* list, list_callback_func function, int arg){
	if(list_empty(list)) return NULL;

	struct list_elem* elem = list->head.next;
	while(elem != &list->tail){
		if(function(elem, arg)){
			return elem;
		}
		elem = elem->next;
	}
	return NULL;
}


uint32_t list_len(struct list* list){
	uint32_t count=0;
	struct list_elem* elem = list->head.next;
	while(elem != &list->tail){
		count++;
		elem = elem->next;
	}
	return count;
}







