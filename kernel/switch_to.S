[bits 32]
section .text
global switch_to
;switch_to(current_pcb_ptr, next_pcb_ptr)
switch_to:
    push esi
    push edi
    push ebx
    push ebp
    
    ;save current esp to current pcb.self_kstack
    mov eax, [esp+20]
    mov [eax], esp

    ;set esp to next pcb.self_kstack
    mov eax, [esp+24]
    mov esp, [eax]
    
    pop ebp
    pop ebx
    pop edi
    pop esi
    ret
