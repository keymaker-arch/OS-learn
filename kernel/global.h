#ifndef __KERNEL_GLOBAL_H
#define __KERNEL_GLOBAL_H

#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3

#define TI_GDT 0
#define TI_LDT 1

//kernel space selectors
#define SELECTOR_KERNEL_CODE ((1<<3)+(TI_GDT<<2)+RPL0)
#define SELECTOR_KERNEL_DATA ((2<<3)+(TI_GDT<<2)+RPL0)
#define SELECTOR_KERNEL_STACK ((2<<3)+(TI_GDT<<2)+RPL0)
#define SELECTOR_KERNEL_VIDEO ((3<<3)+(TI_GDT<<2)+RPL0)

//IDT desc attribute(IDT desc high word bit8~bit15)
#define IDT_DESC_P 1
#define IDT_DESC_DPL0 0
#define IDT_DESC_DPL3 3
#define IDT_DESC_32_TYPE 0xE

#define IDT_DESC_ATTR_DPL0 (IDT_DESC_P<<7)+(IDT_DESC_DPL0<<5)+(IDT_DESC_32_TYPE)
#define IDT_DESC_ATTR_DPL3 (IDT_DESC_P<<7)+(IDT_DESC_DPL3<<5)+(IDT_DESC_32_TYPE)

//interruption num
#define IDT_DESC_COUNT 34	//interrupt num

// total memory from loader, stored in 0x800(physical), defined in boot.inc
#define MEM_TOTAL_STORE_ADDR 0x800


#endif