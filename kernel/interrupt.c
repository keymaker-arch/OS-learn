#include "print.h"
#include "stdint.h"
#include "io.h"
#include "interrupt.h"
#include "global.h"

#define PIC_M_CTRL 0x20		//control port of master PIC
#define PIC_M_DATA 0x21		//data port of master PIC
#define PIC_S_CTRL 0xa0		//control port of master PIC
#define PIC_S_DATA 0xa1		//data port of master PIC

//interrupt control
#define EFLAGS_IF 0x200		//eflags IF bit
#define GET_EFLAGS(EFLAG_VAR) asm volatile ("pushfl;popl %0":"=g"(EFLAG_VAR))

struct intr_gate_desc{
	uint16_t func_offset_low;
	uint16_t selector;
	uint8_t dcount;		//0 for intr gate
	uint8_t attribute;
	uint16_t func_offset_high;
};

//IDT
static struct intr_gate_desc IDT[IDT_DESC_COUNT];

//interrupt entry function, defined in interrupt_entry.S
extern void* intr_entry_table[IDT_DESC_COUNT];

//interrupt name
char* intr_name[IDT_DESC_COUNT];

//interrupt handle function addr array
void* intr_handler_table[IDT_DESC_COUNT];

static void create_IDT_desc(struct intr_gate_desc* desc, uint8_t attr, void* func){
	desc->func_offset_low=(uint32_t)func & 0x0000ffff;
	desc->selector=SELECTOR_KERNEL_CODE;
	desc->dcount=0;
	desc->attribute=attr;
	desc->func_offset_high=((uint32_t)func & 0xffff0000) >> 16;
}

//fill in IDT
static void init_IDT(){
	int i;
	for(i=0;i<=IDT_DESC_COUNT;i++){
		create_IDT_desc(&IDT[i],IDT_DESC_ATTR_DPL0,intr_entry_table[i]);
	}
	put_string("IDT init done\n");
}

//init PIC,accept clock interrupt
static void init_PIC(){
	//init master PIC
	outb(PIC_M_CTRL,0x11);	//ICW0:edge trigger,cascade,need ICW4
	outb(PIC_M_DATA,0x20);	//ICW1:set intitial vector 0x20(IR[0～7]->int 0x20~0x27)
	outb(PIC_M_DATA,0x04);	//ICW3:IR2 connect to slave PIC
	outb(PIC_M_DATA,0x03);	//ICW4:fully nested mode,no buffer,auto EOI,x86

	//init slave PIC
	outb(PIC_S_CTRL,0x11);	
	outb(PIC_S_DATA,0x20);	//ICW1:set intitial vector 0x28(IR[8～15]->int 0x28~0x2f)
	outb(PIC_S_DATA,0x02);	//ICW3:connect to master PIC IR2
	outb(PIC_S_DATA,0x03);

	//turn on IR0 on master PIC,accept clock interrupt
	outb(PIC_M_DATA,0xfc);
	outb(PIC_S_DATA,0xff);

	put_string("PIC init done\n");

}

//load IDT addr and limit
static void load_IDT_reg(){
	uint64_t IDT_reg_content=((uint64_t)(uint32_t)IDT << 16) | (sizeof(IDT)-1);
	asm volatile ("lidt %0"::"m"(IDT_reg_content));
	put_string("load IDT register done\n");
}

static void general_interrupt_handler(uint8_t int_Vector){
	// spurious interrupt, escape
	if(int_Vector == 0x27 || int_Vector == 0x2f){
		return;
	}

	// print exception message
	set_cursor(0);
	int tmp_pos = 0;
	while(tmp_pos < 400){
		put_char(' ');tmp_pos++;
	}
	set_cursor(0);
	put_string("!!!!!!!!!!   exception message   !!!!!!!!!!");
	set_cursor(88);
	put_string(intr_name[int_Vector]);
	if(int_Vector == 14){
		int page_fault_vaddr = 0;
		asm volatile ("movl %%cr2, %0" : "=r"(page_fault_vaddr));
		put_string("\npage fault for virtual address:");put_uint_hex(page_fault_vaddr);
	}
	put_string("\n!!!!!!!!!!   exception message end !!!!!!!!!!");
	while(1);
	
}



void test_handler(){
	put_string("clock interrupt\n");	
}


// write general interrupt handler function addr to intr_handler array, for init. every handler function 
// will be installed in corresponding module 
static void install_general_intr_handler(){
	
	for (int i = 0; i <= IDT_DESC_COUNT; ++i){
		intr_name[i]="None";
		intr_handler_table[i] = general_interrupt_handler;
	}
	intr_name[0]="#DE Devide Error";
	intr_name[1]="#DB Debug";
	intr_name[2]="NMI Interrupt";
	intr_name[3]="#BP";
	intr_name[4]="#OF Overflow";
	intr_name[5]="#BR BOUND Range Exceeded";
	intr_name[6]="#UD UnDefined Opcode";
	intr_name[7]="#NM No Mached Coprocessor";
	intr_name[8]="#DF";
	intr_name[9]="#MF";
	intr_name[10]="#TS";
	intr_name[11]="#NP";
	intr_name[12]="#SS";
	intr_name[13]="#GP";
	intr_name[14]="#PF";
	intr_name[15]="_reserved";
	intr_name[16]="#MF";
	intr_name[17]="#AC";
	intr_name[18]="#MC";
	intr_name[19]="#XM";

	// intr_handler_table[0x20] = test_handler;
	put_string("install general interrupt handler done\n");
}


// write function pointer to intr_handler_table, write intr_name to intr_name, called
// by each module to install their intr handler
void regist_handler(uint8_t int_Vector, void* function_ptr, char* intr_name){
	intr_handler_table[int_Vector] = function_ptr;
	intr_name[int_Vector] = *intr_name;
}



void init_interrupt(){
	init_IDT();
	install_general_intr_handler();
	init_PIC();
	load_IDT_reg();
	put_string("interrupt init done\n\n\n");
}




/*turn on/off interrupt*/
enum intr_status intr_get_status(){
	uint32_t eflags=0;
	GET_EFLAGS(eflags);
	return (eflags & EFLAGS_IF)?INTR_ON:INTR_OFF; 
}

//enalbe interrupt and return old status
enum intr_status intr_enable(){
	if(INTR_ON==intr_get_status()){
		return INTR_ON;
	}else{
		asm volatile ("sti");
		return INTR_OFF;
	}
}


enum intr_status intr_disable(){
	if(INTR_ON==intr_get_status()){
		asm volatile ("cli");
		return INTR_ON;
	}else{
		return INTR_OFF;
	}
}


void intr_set_status(enum intr_status status){
	if(status != intr_get_status()){
		if(status == INTR_ON){
			asm volatile ("sti");
		}else{
			asm volatile ("cli");
		}
	}
}




