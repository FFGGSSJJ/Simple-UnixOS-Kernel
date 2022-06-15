.text

.global scheduler

.align 4
scheduler:
    pushl   %ebp
    movl    %esp, %ebp
    # callee saved register
    pushl   %ebx
    pushl   %esi
    pushl   %edi

    # store current process kernel esp
    pushl   %esp
    call    store_current_shched_esp
    addl    $4, %esp
    # check return value in %eax
    cmpl    $-1, %eax
    jne     next_step

    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp
    ret

    next_step:
    call    schedule_handler

    # switch kernel stack (change to next process esp)
    call    get_next_shched_esp
    movl    %eax, %esp

    # restore next process registers
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp

    ret
