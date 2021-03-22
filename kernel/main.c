#include "print.h"
#include "thread.h"
#include "init.h"
#include "interrupt.h"
#include "console.h"
#include "ioqueue.h"
#include "keyboard.h"

void thread_test_a(void*);
void thread_test_b(void*);


int main(void){
	init_all();

	thread_start("k_thread_a", 31, thread_test_a, "argA_");
	thread_start("k_thread_b", 10, thread_test_b, "argB_");
	intr_enable();
	put_char('\n');
	// while(1){
	// 	console_put_string("main ");
	// }
	while(1);
	return 0;
}


void thread_test_a(void* arg){
	arg = (char*)arg;
	while(1){
		enum intr_status status = intr_disable();
		if(!ioq_empty(&kbd_queue)){
			console_put_string(arg);
			char byte = ioq_getchar(&kbd_queue);
			console_put_char(byte);
		}
		intr_set_status(status);
	}
}

void thread_test_b(void* arg){
	arg = (char*)arg;
	while(1){
		enum intr_status status = intr_disable();
		if(!ioq_empty(&kbd_queue)){
			console_put_string(arg);
			char byte = ioq_getchar(&kbd_queue);
			console_put_char(byte);
		}
		intr_set_status(status);
	}
	
}