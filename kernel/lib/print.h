#ifndef __LIB_PRINT_H
#define __LIB_PRINT_H
#include "stdint.h"
void put_char(uint8_t char_ascii);
void put_string(char* message);
void put_uint(unsigned int num);
void put_uint_hex(unsigned int num);

void set_cursor(uint16_t pos);
void keyboard_put_char(char);
#endif