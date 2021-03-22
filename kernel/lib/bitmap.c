#include "bitmap.h"
#include "stdint.h"
#include "string.h"
#include "print.h"
#include "debug.h"

void bitmap_init(struct bitmap* bm_ptr){	
	memset(bm_ptr->bits_ptr,0,bm_ptr->bitmap_bytes_len);
}


//return true if bm_ptr[bit_index] equals 1
uint8_t bitmap_index_test(struct bitmap* bm_ptr,uint32_t bit_index){
	uint32_t byte_index=bit_index/8;
	uint32_t offset_in_byte=bit_index%8;
	return (bm_ptr->bits_ptr[byte_index]) & (BITMAP_MASK<<offset_in_byte);
}


//find a countinuous area containing COUNT bits, return index of the start bit of the area, return -1 if fail
int bitmap_scan(struct bitmap* bm_ptr,uint32_t count){
	//find a free byte
	uint32_t index_byte=0;
	while((0xff==bm_ptr->bits_ptr[index_byte]) && (index_byte < bm_ptr->bitmap_bytes_len)){
		index_byte++;
	}
	ASSERT(index_byte < bm_ptr->bitmap_bytes_len);
	if(index_byte == bm_ptr->bitmap_bytes_len){
		return -1;
	}

	//find free bit in free byte
	uint32_t index_bit=0;
	while((bm_ptr->bits_ptr[index_byte]) & (BITMAP_MASK<<index_bit)){
		index_bit++;
	}

	uint32_t index_bit_start=index_byte*8 + index_bit;
	if(count==1) return index_bit_start;
	uint32_t bits_left=bm_ptr->bitmap_bytes_len * 8 - index_bit_start;
	uint32_t index_next_bit=index_bit_start+1;
	uint32_t count_found=1;
	index_bit_start = -1;

	while(bits_left-- > 0){
		if(!(bitmap_index_test(bm_ptr,index_next_bit))){
			count_found++;
		}else{
			count_found=0;
		}

		if(count_found==count){
			index_bit_start=index_next_bit+1-count;
			break;
		}
		index_next_bit++;
	}
	ASSERT(index_bit_start != -1);
	return index_bit_start;
}

// set bit_ptr[index_bit]=value
void bitmap_set_bit(struct bitmap* bm_ptr,uint32_t index_bit,uint8_t value){
	ASSERT((value==0) || (value==1));
	uint32_t index_byte=index_bit/8;
	index_bit=index_bit%8;
	if(value){
		bm_ptr->bits_ptr[index_byte] |= (BITMAP_MASK<<index_bit);
	}else{
		bm_ptr->bits_ptr[index_byte] &= ~(BITMAP_MASK<<index_bit);
	}
}

