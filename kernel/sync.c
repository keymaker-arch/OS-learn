#include "sync.h"
#include "list.h"
#include "thread.h"
#include "debug.h"
#include "interrupt.h"


void sema_init(struct semaphore* sema, uint8_t value){
	sema->value = value;
	list_init(&sema->waiters);
}


void lock_init(struct lock* lock){
	lock->holder = NULL;
	lock->holder_repeat_n = 0;
	sema_init(&lock->semaphore, 1);
}


void sema_down(struct semaphore* sema){
	enum intr_status intr_status = intr_disable();
	struct pcb_struct* __pcb_ptr = get_pcb_ptr();
	while(sema->value == 0){
		ASSERT(!elem_find(&sema->waiters, &__pcb_ptr->general_tag));
		list_append(&sema->waiters, &__pcb_ptr->general_tag);
		thread_block(TASK_BLOCKED);
		// the thread goes to bock_list, and shift to ready_list when lock is released
		// waiting to run. when running the thread checks sema->value, if is 0(the lock is acquired by  
		// another thread when this thread is in ready_list), block itself again
	}
	sema->value--;
	ASSERT(sema->value == 0);
	intr_set_status(intr_status);
}


void sema_up(struct semaphore* sema){
	enum intr_status intr_status = intr_disable();
	ASSERT(sema->value == 0);
	if(!list_empty(&sema->waiters)){
		struct pcb_struct* thread_blocked = elem2pcb(list_pop(&sema->waiters));
		thread_unblock(thread_blocked);
	}
	sema->value++;
	ASSERT(sema->value == 1);
	intr_set_status(intr_status);
}


void lock_acquire(struct lock* lock){
	struct pcb_struct* __pcb_ptr = get_pcb_ptr();
	if(lock->holder != __pcb_ptr){
		sema_down(&lock->semaphore);
		lock->holder = __pcb_ptr;
		ASSERT(lock->holder_repeat_n == 0);
		lock->holder_repeat_n = 1;
	}else{
		lock->holder_repeat_n++;
	}
}


void lock_release(struct lock* lock){
	ASSERT(lock->holder == get_pcb_ptr());
	if(lock->holder_repeat_n > 1){
		lock->holder_repeat_n--;
		return;
	}
	ASSERT(lock->holder_repeat_n == 1);
	// holder and holder_repeat should be reset before sema_up(), in case interrupt happened, providing such 
	// case the semaphore is still 0
	lock->holder = NULL;
	lock->holder_repeat_n = 0;
	sema_up(&lock->semaphore);
}





