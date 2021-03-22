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

[bits 32]
section .text
global put_char
put_char:
    pushad  ;back up context
    mov ax,SELECTOR_VIDEO
    mov gs,ax

    ;retrive cursor position from CRT
    mov dx,0x03d4
    mov al,0x0e
    out dx,al
    mov dx,0x03d5
    in al,dx
    mov ah,al
    mov dx,0x03d4
    mov al,0x0f
    out dx,al
    mov dx,0x03d5
    in al,dx        ;cursor position store in ax
    
    mov bx,ax
    mov ecx,[esp+36]    ;char to print
    cmp cl,0xd  ;CR
    jz .is_CR
    cmp cl,0xa  ;LF
    jz .is_LF
    cmp cl,0x8  ;BS
    jz .is_BS
    jmp .put_one_char
    
    .is_BS:
        dec bx
        shl bx,1
        mov byte [gs:bx],' '
        inc bx
        mov byte [gs:bx],0x07
        shr bx,1
        jmp .set_cursor
    
    .put_one_char:
        shl bx,1
        mov byte [gs:bx],cl
        inc bx
        mov byte [gs:bx],0x07
        shr bx,1
        inc bx
        cmp bx,2000     ;last position of this screen
        jl .set_cursor
        jnl .roll_screen
    
    .is_LF:
    .is_CR:
        xor dx,dx
        mov ax,bx
        mov si,80
        div si
        sub bx,dx
        add bx,80
        cmp bx,2000
        jl .set_cursor
    
    .roll_screen:
        cld
        mov ecx,960
        mov esi,0xc00b80a0   ;1st char of 2nd line
        mov edi,0xc00b8000   ;1st char of 1st line
        rep movsd
        mov ebx,3840    ;fill last line with blank
        mov ecx,80
        .cls:
            mov word [gs:ebx],0x0720
            add ebx,2
            loop .cls
            mov bx,1920 ;reset cursor to 1st char of last line
    
    ;write cursor position to CRT
    .set_cursor:
        mov dx,0x03d4
        mov al,0x0e
        out dx,al
        mov dx,0x03d5
        mov al,bh
        out dx,al
        mov dx,0x03d4
        mov al,0x0f
        out dx,al
        mov dx,0x03d5
        mov al,bl
        out dx,al
    popad
    ret











