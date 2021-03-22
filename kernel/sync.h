#ifndef __LIB_SYNC_H
#define __LIB_SYNC_H

#include "stdint.h"
#include "thread.h"
#include "list.h"

struct semaphore{
	uint8_t value;
	struct list waiters;
};


struct lock{
	struct pcb_struct* holder;
	struct semaphore semaphore;
	uint32_t holder_repeat_n;
};

void lock_acquire(struct lock* lock);
void lock_release(struct lock* lock);
void lock_init(struct lock* lock);
#endif