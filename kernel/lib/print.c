#include "print.h"
#include "io.h"


void put_string(char* message){
	for (int i = 0; message[i]; ++i)
	{
		put_char(message[i]);
	}
}


void put_uint(unsigned int num){
	int tmp,divider=1000000000,flag=0;
	while(divider!=0){
		tmp=num/divider;
		if(tmp | flag){
			put_char(tmp+48);
			flag=1;
		} 
		num=num-tmp*divider;
		divider=divider/10;
	}
}


void put_uint_hex(unsigned int num){
	int remainder[8];
	int i=0;
	int tmp;
	char hex[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	while(num>0){
		tmp=num%16;
		remainder[i++]=tmp;
		num=num/16;
	}
	put_string("0x");
	if(i == 0){
		put_char('0');
	}else{
		for(i=i-1;i>=0;i--){
			put_char(hex[remainder[i]]);
		}
	}

}


// set cursor to pos
void set_cursor(uint16_t pos){
	// low 8 bits
	outb(0x03d4, 0x0f);
	outb(0x03d5, (uint8_t)pos);

	// high 
	outb(0x03d4, 0x0e);
	outb(0x03d5, (uint8_t)(pos >> 8));
}


// to implement keyboard print '\t'
void keyboard_put_char(char keyboard_char){
	if(keyboard_char == '\t'){
		put_string("    ");
	}else{
		put_char(keyboard_char);
	}
}

