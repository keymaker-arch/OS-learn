#include "ioqueque.h"


void ioqueque_init(struct ioqueque* ioq){
	lock_init(ioq->lock);
	ioq->producer = ioq->consumer = NULL;
	ioq->head = ioq->tail = 0;
}

static int32_t next_pos(int32_t pos){
	return (pos + 1) % buffsize;
}

bool ioq_empty(struct ioqueque* ioq){
	return ioq->head == ioq->tail;
}

bool ioq_full(struct ioqueque* ioq){
	return next_pos(ioq->head) == ioq->tail;
}

static void ioq_block(struct pcb_struct** waiter){
	*waiter = get_pcb_ptr();
	thread_block(TASK_BLOCKED);
}

static void ioq_unblock(struct pcb_struct** waiter){
	thread_unblock(*waiter);
	*waiter = NULL;
}


// call by consumer
void ioq_getchar(struct ioqueque* ioq){
	while(ioq_empty(ioq)){
		lock_acquire(&ioq->lock);
		ioq_block(&ioq->consumer);	// set ioq consumer to current thread
		lock_release(&ioq->lock);
	}

	char byte = ioq->buf[ioq->tail];
	ioq->tail = next_pos(ioq->tail);

	if(ioq->producer != NULL){
		ioq_unblock(&ioq->producer);
	}
	return byte;
}


// call by producer
void ioq_putchar(struct ioqueque* ioq, char byte){
	while(ioq_full(ioq)){
		lock_acquire(&ioq->lock);
		ioq_block(&ioq->producer);
		lock_release(&ioq->lock);
	}

	ioq->buf[ioq->head] = byte;
	ioq->head = next_pos(ioq->head);

	if(ioq->consumer != NULL){
		ioq_unblock(&ioq->consumer);
	}
}

