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