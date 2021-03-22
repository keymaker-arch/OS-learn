#ifndef __IOQUEQUE_H
#define __IOQUEQUE_H

#include "stdint.h"
#include "thread.h"
#include "sync.h"

#define buffsize 64

struct ioqueque{
	struct lock lock;
	struct pcb_struct* producer;
	struct pcb_struct* consumer;
	uint8_t buf[buffsize];
	int32_t head;
	int32_t tail;
};

void ioqueque_init(struct ioqueque* ioq);
bool ioq_empty(struct ioqueque* ioq);
bool ioq_full(struct ioqueque* ioq);
void ioq_getchar(struct ioqueque* ioq);
void ioq_putchar(struct ioqueque* ioq, char byte);


#endif