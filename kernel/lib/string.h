#ifndef __LIB_STRING_H
#define __LIB_STRING_H

void memset(void* dst,uint8_t value,uint32_t size);
void memcpy(void* dst,void* src,uint32_t size);
void memcmp(void* ptr1,void* ptr2,uint32_t size);
char* strcpy(char* dst,char* src);
uint32_t strlen(char* str);
int strcmp(char* ptr1,char* ptr2);
char* strchr(char* str,char ch);
char* strrchr(char* str,char ch);
char* strcat(char* dst,char* src);
uint32_t strchrs(char* str,char ch);


#endif