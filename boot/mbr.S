%include "boot.inc"
SECTION MBR vstart=0x7c00
    ;initial ds,es,ss,fs reg and stack
    mov ax,0
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov fs,ax
    mov sp,0x7c00
    mov ax,0xb800
    mov gs,ax
    
    ;clear screen
    mov ax,0x600
    mov bx,0x700
    mov cx,0
    mov dx,0x184f
    int 0x10
    
    ;print string
    mov byte [gs:0x00], '1'
    mov byte [gs:0x01], 0xa4
    mov byte [gs:0x02], ' '
    mov byte [gs:0x03], 0xa4
    mov byte [gs:0x04], 'M'
    mov byte [gs:0x05], 0xa4
    mov byte [gs:0x06], 'B'
    mov byte [gs:0x07], 0xa4
    mov byte [gs:0x08], 'R'
    mov byte [gs:0x09], 0xa4
    
    mov eax,LOADER_START_SECTOR
    mov bx,LOADER_BASE_ADDR
    mov cx,LOADER_SECTOR_COUNT
    call rd_disk_m_16
    
    jmp LOADER_BASE_ADDR
    
;read loader from disk to mem
rd_disk_m_16:
    mov esi,eax
    mov di,cx
    
    mov al,cl
    mov dx,0x1f2  
    out dx,al    ;sector count register
    
    mov eax,esi
    mov cl,8
    mov dx,0x1f3
    out dx,al    ;LBA low
    shr eax,cl
    mov dx,0x1f4
    out dx,al    ;LBA mid
    shr eax,cl
    mov dx,0x1f5
    out dx,al    ;LBA high
    shr eax,cl
    and al,0x0f
    or al,0xe0
    mov dx,0x1f6   
    out dx,al    ;device register
    
    mov al,0x20     ;read command
    mov dx,0x1f7
    out dx,al    ;command register
    
.check_disk_ready:
    nop             ;sleep
    mov dx,0x1f7
    in al,dx        ;status register
    and al,0x88     
    cmp al,0x08     ;check BSY bit
    jnz .check_disk_ready
    
    mov ax,di       ;512 bytes per sector, read one word each time
    mov dx,256
    mul dx
    mov cx,ax       
    mov dx,0x1f0    ;data register
.go_on_read:
    in ax,dx        ;loader base addr
    mov [bx],ax
    add bx,2
    loop .go_on_read
    ret
    
    times 510-($-$$) db 0
    db 0x55,0xaa
    
    