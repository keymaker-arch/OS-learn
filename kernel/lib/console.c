#include "print.h"
#include "console.h"
#include "sync.h"
#include "stdint.h"


static struct lock console_lock;


void console_init(){
	lock_init(&console_lock);
}


void console_put_string(char* string){
	lock_acquire(&console_lock);
	put_string(string);
	lock_release(&console_lock);
}


void console_put_uint_hex(uint32_t hex){
	lock_acquire(&console_lock);
	put_uint_hex(hex);
	lock_release(&console_lock);
}













