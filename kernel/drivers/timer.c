#include "timer.h"
#include "io.h"
#include "print.h"
#include "stdint.h"
#include "thread.h"
#include "interrupt.h"
#include "debug.h"
#include "thread.h"


uint32_t ticks;		// total ticks since interrupt open


static void frequency_set(uint8_t counter_port, uint8_t counter_id, uint8_t rwl, uint8_t mode, uint16_t value){
	// write control port 0x43
	outb(CONTROL_PORT, (uint8_t)(counter_id<<6 | rwl<<4 | mode<<1));

	// write counter port, set initial value, high 8 bits and low respectively
	outb(counter_port, (uint8_t)value);
	outb(counter_port, (uint8_t)value >> 8);
}


static void intr_clock_handler(){
	struct pcb_struct* current_thread = get_pcb_ptr();
	ASSERT(current_thread->stack_magic == 0x20000125);
	current_thread->ticks_total++;
	ticks++;

	if(current_thread->ticks == 0){
		schedule();
	}else{
		current_thread->ticks--;
	}

}


void timer_init(){
	frequency_set(COUNTER0_PORT, COUNTER0_ID, READ_WRITE_LATCH, WORK_MODE, COUNTER0_VALUE);
	regist_handler(0x20, intr_clock_handler, "clock interrupt handler");
	put_string("timer init done\n");
}


