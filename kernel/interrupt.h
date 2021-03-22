#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#include "stdint.h"

void init_interrupt();

enum intr_status{
	INTR_OFF=0,
	INTR_ON
};

enum intr_status intr_get_status();
enum intr_status intr_enable();
enum intr_status intr_disable();
void intr_set_status(enum intr_status status);

void regist_handler(uint8_t int_Vector, void* function_ptr, char*);

#endif