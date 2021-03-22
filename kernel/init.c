#include "timer.h"
#include "interrupt.h"
#include "memory.h"
#include "thread.h"
#include "print.h"
#include "console.h"
#include "keyboard.h"


void init_all(){
	init_interrupt();
	timer_init();
	keyboard_init();



	mem_init();
	thread_enviroment_init();
	console_init();
	put_string("kernel init done\n");
}