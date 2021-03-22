%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
    jmp loader_start
    
    ;GDT
    GDT_BASE:
        dd 0x00000000
        dd 0x00000000
    CODE_DESC:
        dd 0x0000FFFF
        dd DESC_CODE_HIGH4
    DATA_STACK_DESC:
        dd 0x0000FFFF
        dd DESC_DATA_HIGH4
    VIDEO_DESC:
        dd 0x80000007
        dd DESC_VIDEO_HIGH4
    GDT_SIZE equ $ - GDT_BASE
    GDT_OFFSET equ GDT_SIZE - 1 ;calc offset(length) of GDT
    times 60 dq 0
 
   
    ;GDT pointor
    gdt_ptr dw GDT_OFFSET
            dd GDT_BASE
            
    ;ARDS
    TOTAL_MEM dd 0
    ARDS_BUFF times 244 db 0
    ARDS_COUNT dw 0
    
    loader_message db '2 loader of protected mode'
    check_mem_fail_message db 'check mem failed'
    check_mem_succeed_message db 'check mem succeed'
    
loader_start:
    ;print string
    mov sp,LOADER_BASE_ADDR
    mov bp,loader_message
    mov cx,0x1A
    mov ax,0x1301
    mov bx,0x001f
    mov dx,0x1800
    int 0x10
    
    ;memory check:int 0x15,eax=e820
    xor ebx,ebx
    mov edx,0x534d4150  ;signature
    mov di,ARDS_BUFF
    .mem_check_loop:
        mov eax,0x0000e820  ;int 0x15 subfunc 0xe820
        mov ecx,20  ;ARDS structure len
        int 0x15        
        jc .mem_check_fail
        add di,cx
        inc word [ARDS_COUNT]
        cmp ebx,0   ;check last ARDS
        jnz .mem_check_loop
    ;find largest mem area
    mov cx,[ARDS_COUNT]
    mov ebx,ARDS_BUFF
    xor edx,edx ;largest mem
    .find_largest_mem_loop:
        mov eax,[ebx]   ;base addr
        add eax,[ebx+8] ;length
        add ebx,20  ;next ARDS structure
        cmp edx,eax
        jge .next_loop
        mov edx,eax
        .next_loop:
            loop .find_largest_mem_loop
    jmp .mem_check_succeed
    
    .mem_check_fail:
        mov bp,check_mem_fail_message
        mov cx,0x10
        mov ax,0x1301
        mov bx,0x001f
        mov dx,0x1700
        int 0x10
        jmp $
    .mem_check_succeed:
        mov [TOTAL_MEM],edx
        mov bp,check_mem_succeed_message
        mov cx,0x11
        mov ax,0x1301
        mov bx,0x001f
        mov dx,0x1700
        int 0x10       
    
    ;enter protected mode
    ;open A20
    mov dx,0x92
    in al,dx
    or al,00000010b
    out dx,al
    
    ;load GDT register
    lgdt [gdt_ptr]
    
    ;set PE bit in cr0 register
    mov eax,cr0
    or eax,0x00000001
    mov cr0,eax
    
    jmp dword SELECTOR_CODE:p_mode_start
    
    
[bits 32]
p_mode_start:
    mov ax,SELECTOR_DATA
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,LOADER_STACK_TOP
    mov ax,SELECTOR_VIDEO
    mov gs,ax
    
    ;move TOTAL_MEM to physical 0x800, for kernel memory init
    mov eax,[TOTAL_MEM]
    mov [ds:TOTAL_MEM_STORE_ADDR],eax
    
    ;print string
    mov byte [gs:160],'p'
    mov byte [gs:162],'r'
    mov byte [gs:164],'o'
    mov byte [gs:166],'t'
    mov byte [gs:168],'e'
    mov byte [gs:170],'c'
    mov byte [gs:172],'t'
    mov byte [gs:174],'e'
    mov byte [gs:176],'d'
    mov byte [gs:178],' '
    mov byte [gs:180],'m'
    mov byte [gs:182],'o'
    mov byte [gs:184],'d'
    mov byte [gs:186],'e'
    
    ;read kernel from disk to buff
    mov eax,KERNEL_START_SECTOR
    mov ebx,KERNEL_BIN_BASE_ADDR
    mov ecx,KERNEL_SECTOR_COUNT
    call rd_disk_m_32
    
    ;start pagination
    call setup_page
    
    sgdt [gdt_ptr]  ;save gdt reg
    mov ebx,[gdt_ptr+2]
    or dword [ebx+0x18+4],0xc0000000    ;video memory:map to kernel space
    add dword [gdt_ptr+2],0xc0000000    ;GDT addr
    add esp,0xc0000000                  ;stack addr
    
    mov eax,PDE_table_ADDR
    mov cr3,eax     ;PDE table address to cr3 reg
    
    mov eax,cr0
    or eax,0x80000000
    mov cr0,eax     ;turn on PG bit, start memory pagination
    
    lgdt [gdt_ptr]  ;reload GDT in kernel space
    
    mov ax,SELECTOR_VIDEO
    mov gs,ax
    mov byte [gs:320],'v'
    mov byte [gs:322],'i'
    mov byte [gs:324],'r'
    mov byte [gs:326],'t'
    mov byte [gs:328],'u'
    mov byte [gs:330],'a'
    mov byte [gs:332],'l'
    
    jmp SELECTOR_CODE:enter_kernel
enter_kernel:
    call kernel_init
    mov esp,0xc009f000
    call clear_screen_for_kernel
    jmp KERNEL_ENTRY_POINT

    
;pagination function    
setup_page:
    ;clear PDE table area
    mov ecx,0x1000
    mov esi,0
    .clear_PDE_loop:
        mov byte [PDE_table_ADDR + esi],0
        inc esi
        loop .clear_PDE_loop
    
    ;create PTE0, map virture 0x0~0xfffff(1M) to physical 0x0~0xfffff(1M)
    mov ecx,256 ;256 pages=1M, is enough
    mov esi,0   ;page index
    mov edx, PG_US_U | PG_RW_W | PG_P   ;PT0 index0 map to physical 0x0
    mov eax,PDE_table_ADDR
    add eax,0x1000
    mov ebx,eax ;addr and attribute of PT0
    .create_PTE0:
        mov [ebx+esi*4],edx
        add edx,0x1000  ;physical addr of next page
        inc esi         ;next PTE index
        loop .create_PTE0
    
    ;create PDE
    mov eax,PDE_table_ADDR
    or eax,PG_US_U | PG_RW_W | PG_P
    mov [PDE_table_ADDR + 4092],eax ;last PDE(PDE1023) points to PDE table addr
    add eax,0x1000  ;addr and attribute of PT0
    mov [PDE_table_ADDR],eax    ;PDE0 points to PT0
    mov [PDE_table_ADDR + 0xc00],eax    ;map virture 3G-3G+4M(kernel) to physical 0x0~0xfffff
    
    ;;fill kernel PDE
    ;mov eax,PDE_table_ADDR
    ;add eax,0x2000  
    ;or eax,PG_US_U | PG_RW_W | PG_P ;addr and attribute of PT1
    ;mov ebx,PDE_table_ADDR
    ;mov esi,769
    ;mov ecx,254     ;kernel area:PDE769~PDE1022    
    ;.fill_kernel_PDE_loop:
    ;    mov [ebx+esi*4],eax
    ;    inc esi
    ;    add eax,0x1000  ;next PT addr
    ;    loop .fill_kernel_PDE_loop
    ret
    
;read disk mode 32 bits. eax:disk start sector, ebx:target memory addr, ecx:disk sector count
rd_disk_m_32:
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
    mov [ebx],ax
    add ebx,2
    loop .go_on_read
    ret
  
;copy kernel to kernel space 
kernel_init:
    xor eax,eax
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    
    mov ebx,[KERNEL_BIN_BASE_ADDR + 28]     ;e_phoff, offset of 1st program header
    add ebx,KERNEL_BIN_BASE_ADDR            ;addr of 1st program header
    mov dx,[KERNEL_BIN_BASE_ADDR + 42]      ;e_phentsize, size of program header
    mov cx,[KERNEL_BIN_BASE_ADDR + 44]      ;e_phnum, number of program header

    .cpy_segment_loop:
        cmp byte [ebx],0
        je .PTNULL              ;PT_NULL, no load
        
        push dword [ebx+16]     ;p_filesz
        mov eax,[ebx+4]         ;p_offset
        add eax,KERNEL_BIN_BASE_ADDR
        push eax                ;physical addr of segment,source addr
        push dword [ebx+8]      ;virture addr of segment,dst addr
        call .mem_cpy
        add esp,12
        .PTNULL:
        add ebx,edx             ;next program header addr        
        loop .cpy_segment_loop
    ret
        
        ;mem_cpy(dst,src,size), arg in stack
        .mem_cpy:
            push ebp
            mov ebp,esp
            cld
            push ecx    ;ecx is needed for outer loop:cpy_segment_loop
            
            mov edi,[ebp + 8]   ;dst
            mov esi,[ebp + 12]  ;src
            mov ecx,[ebp + 16]  ;size
            rep movsb
            
            pop ecx
            pop ebp
            ret

;clear all output and set cursor at (0,0)
clear_screen_for_kernel:
    mov ecx,2000
    mov ebx,0
    .cls:
        mov word [gs:ebx],0x0720
        add ebx,2
        loop .cls
    mov ebx,0
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
    ret
    










