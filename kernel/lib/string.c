#include "debug.h"
#include "stdint.h"

void memset(void* dst,uint8_t value,uint32_t size){
	ASSERT(dst!=NULL);
	char* _dst=(char*)dst;
	while(size-->0)
		*_dst++=value;
}


void memcpy(void* dst,void* src,uint32_t size){
	ASSERT(src!=NULL && dst!=NULL);
	char* _dst=(char*)dst;
	char* _src=(char*)src;
	while(size-->0)
		*_dst++=*_src;
}


int memcmp(void* ptr1,void* ptr2,uint32_t size){
	ASSERT(ptr1!=NULL && ptr2!=NULL);
	char* _ptr1=ptr1;
	char* _ptr2=ptr2;
	while(size-->0){
		if(*_ptr1!=*_ptr2){
			return (*_ptr1>*_ptr2)?1:-1;
		}
		_ptr1++;
		_ptr2++;
	}
	return 0;
}



char* strcpy(char* dst,char* src){
	ASSERT(dst!=NULL && src!=NULL);
	char* r=dst;
	while((*dst++=*src++));
	return r;
}


uint32_t strlen(char* str){
	ASSERT(str!=NULL);
	char* p=str;
	while(*p++);
	return(p-str-1);
}



int strcmp(char* ptr1,char* ptr2){
	ASSERT(ptr1!=NULL && ptr2!=NULL);
	while(*ptr1!=0 && *ptr1==*ptr2){
		ptr1++;ptr2++;
	}
	return *ptr1<*ptr2 ? -1 : *ptr1>*ptr2;
}


char* strchr(char* str,char ch){
	ASSERT(str!=NULL);
	while(*str!=0){
		if(*str==ch){
			return str;
		}
		str++;
	}
	return NULL;
}


char* strrchr(char* str,char ch){
	ASSERT(str!=NULL);
	char* last_chr=NULL;
	while(*str!=0){
		if(*str==ch){
			last_chr=str;
		}
		str++;
	}
	return last_chr;
}


char* strcat(char* dst,char* src){
	ASSERT(dst!=NULL && src!=NULL);
	char* tmp_ptr=dst;
	while(*tmp_ptr++);
	tmp_ptr--;
	while((*tmp_ptr++=*src++));
	return dst;
}


uint32_t strchrs(char* str,char ch){
	ASSERT(str!=NULL);
	uint32_t count=0;
	while(*str!=0){
		if(*str==ch) count++;
		str++;
	}
	return count;
}

