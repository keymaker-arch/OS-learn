#include "print.h"
#include "interrupt.h"

void panic_spin(char* filename,int line,const char* func,const char* condition){
	intr_disable();		//turn off kernel interrupt
	put_string("\n\n\n!!!ERROR!!!\n\n\n");

	put_string("filename:");put_string(filename);put_string("\n");
	put_string("line:");put_uint(line);put_string("\n");
	put_string("function:");put_string((char*)func);put_string("\n");
	put_string("condition:");put_string((char*)condition);put_string("\n");

	while(1);		//hang program
}