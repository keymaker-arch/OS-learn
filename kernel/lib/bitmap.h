#ifndef __LIB_BITMAP_H
#define __LIB_BITMAP_H

#include "stdint.h"
#define BITMAP_MASK 1	//bit in use

struct bitmap
{
	uint32_t bitmap_bytes_len;
	char* bits_ptr;		//bitmap pointer
};


void bitmap_init(struct bitmap* bm_ptr);

//return true if bm_ptr[bit_index] equals 1
uint8_t bitmap_index_test(struct bitmap* bm_ptr,uint32_t bit_index);

//find a countinuous area containing COUNT bits, return index of the start bit of the area, return -1 if fail
int bitmap_scan(struct bitmap* bm_ptr,uint32_t count);

// set bit_ptr[index_bit]=value
void bitmap_set_bit(struct bitmap* bm_ptr,uint32_t index_bit,uint8_t value);
#endif