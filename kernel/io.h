#ifndef __LIB_IO_H
#define __LIB_IO_H
#include "stdint.h"

//write one byte to port
static inline void outb(uint16_t port,uint8_t data){
	asm volatile ("outb %b0,%w1"::"a"(data),"Nd"(port));
}

//write word_cnt of words in addr to port
static inline void outsw(uint16_t port,const void* addr,uint32_t word_cnt){
	asm volatile ("cld;rep outsw":"+S"(addr),"+c"(word_cnt):"d"(port));
}

//read byte from port
static inline uint8_t inb(uint16_t port){
	uint8_t data;
	asm volatile ("inb %w1,%0":"=a"(data):"Nd"(port));
	return data;
}

//read word_cnt of word from port and write to addr
static inline void insw(uint16_t port,const void* addr,uint32_t word_cnt){
	asm volatile ("cld;rep insw":"+D"(addr),"+c"(word_cnt):"d"(port):"memory");
}

#endif