;loader
LOADER_BASE_ADDR equ 0x900
LOADER_START_SECTOR equ 0x2
LOADER_SECTOR_COUNT equ 10
LOADER_STACK_TOP equ LOADER_BASE_ADDR
TOTAL_MEM_STORE_ADDR equ 0x800

;GDT descriptor attribute
DESC_G_4K	equ	10000000_00000000_00000000b	;granularity
DESC_DB_32	equ	01000000_00000000_00000000b	;length of op code and addr 16/32 bits
DESC_L_32	equ	00000000_00000000_00000000b	;length of code 32/64 bits
DESC_AVL 	equ	00000000_00000000_00000000b	;user available
DESC_OFFSET_CODE2	equ	1111_00000000_00000000b	;2nd part of code segment offset
DESC_OFFSET_DATA2	equ 1111_00000000_00000000b	;2nd part of data segment offset
DESC_OFFSET_VIDEO2	equ 0000_00000000_00000000b  ;2nd part of video mem offset
DESC_P      equ 10000000_00000000b   ;present in memory
DESC_DPL0   equ 00000000_00000000   ;ring 0
DESC_DPL1   equ 00100000_00000000b   ;ring 1
DESC_DPL2   equ 01000000_00000000b   ;ring 2
DESC_DPL3   equ 01100000_00000000b   ;ring 3
DESC_S_SYS  equ 00000000_00000000b   ;system segment(hardware code and data)
DESC_S_NSYS equ 00010000_00000000b   ;non-system segment(software code and data)
DESC_TYPE_CODE  equ 1000_00000000b   ;excutable,unconforming,unreadble,not accessed
DESC_TYPE_DATA  equ 0010_00000000b   ;unexcutable,unconforming,readble,not accessed

;pre-defined segments
DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_DB_32 + DESC_L_32 + DESC_AVL + DESC_OFFSET_CODE2 + DESC_P + DESC_DPL0 + DESC_S_NSYS + DESC_TYPE_CODE 
DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_DB_32 + DESC_L_32 + DESC_AVL + DESC_OFFSET_DATA2 + DESC_P + DESC_DPL0 + DESC_S_NSYS + DESC_TYPE_DATA 
DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_DB_32 + DESC_L_32 + DESC_AVL + DESC_OFFSET_VIDEO2 + DESC_P + DESC_DPL0 + DESC_S_NSYS + DESC_TYPE_DATA + 0x0b 

;selector attribute
TI_GDT equ 000b ;table indicator
TI_LDT equ 100b 
RPL0 equ 00b    ;request previlege level
RPL1 equ 01b
RPL2 equ 10b
RPL3 equ 11b

;define selectors
SELECTOR_CODE equ (0x0001 << 3) + TI_GDT + RPL0
SELECTOR_DATA equ (0x0002 << 3) + TI_GDT + RPL0
SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0

;pagination
PDE_table_ADDR equ 0x100000     ;PDE table load addr
PG_P equ 1b         ;present in physical memory
PG_RW_R equ 00b     ;read only
PG_RW_W equ 10b     ;write and read
PG_US_U equ 000b    ;access previlege:supervisor
PG_US_S equ 100b    ;access previlege:user

;kernel
KERNEL_START_SECTOR equ 20
KERNEL_SECTOR_COUNT equ 200
KERNEL_BIN_BASE_ADDR equ 0x70000
KERNEL_ENTRY_POINT equ 0xc0001500


