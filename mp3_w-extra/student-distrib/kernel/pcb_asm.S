.text

.global get_active_pcb

.align 4
get_active_pcb:
    movl    %esp, %eax
    andl    $0xffffe000, %eax
    ret
