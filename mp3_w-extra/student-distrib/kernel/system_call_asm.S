.text

.global transit_to_user
.global jump_to_execute_ret

.align 4
transit_to_user:
    pushl   %ebp
    movl    %esp, %ebp
    # callee saved register
    pushl   %ebx
    pushl   %esi
    pushl   %edi

    # store current process kernel esp into current (active) pcb
    pushl   %esp
    call    store_execute_esp
    addl    $4, %esp

    # push IRET context to kernel stack
    # get function input arguments
    movl    8(%ebp), %eax   # user_esp
    movl    12(%ebp), %ecx  # user_eip

    pushl   $0x002B         # user DS
    pushl   %eax            # user ESP
    pushl   $0x202          # EFLAGS [ IF ]

    pushl   $0x0023         # user CS
    pushl   %ecx

    iret

execute_ret:
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp
    sti                     # corresponds to cli in halt
    ret

jump_to_execute_ret:
    andl    $0x0, %eax
    movw    8(%esp), %ax    # return value of execute (status)
    movl    4(%esp), %esp   # redirect esp to parent's kernel stack
    jmp     execute_ret
