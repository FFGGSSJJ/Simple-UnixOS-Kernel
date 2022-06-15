
# asm_linkage.S
# 
# Ahthor: Guanshujie Fu
# Modifier: Zitai Kong
# Description: functions for asm linkages used in idt
# Creat Date: 2022.3.18 - add linkages rtc_handler_linkage and keyboard_handler_linage
#             2022.4.9  - add linkages for system call
#             2022.4.22 - add linkages pit_handler_linkage
#
#define ASM 1
#include "asm_linkage.h"
.globl rtc_handler_linkage, keyboard_handler_linkage, pit_handler_linkage
.globl system_call_linkage


jump_table:
.long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn




# rtc_handler_linkage
#   Description: asm linkage for rtc_handler
#   Input: none
#   Output: none
#   Notice: iret is required as it is returned from an interrupt
#
rtc_handler_linkage:
    pushal
    call rtc_handler
    popal
    iret



# keyboard_handler_linkage
#   Description: asm linkage for keyboard_handler
#   Input: none
#   Output: none
#   Notice: iret is required as it is returned from an interrupt
#
keyboard_handler_linkage:
    pushal
    call keyboard_handler
    popal
    iret



# pit_handler_linkage
#   Description: asm linkage for pit_handler
#   Input: none
#   Output: none
#   Notice: iret is required as it is returned from an interrupt
#
pit_handler_linkage:
    pushal
    call pit_handler
    popal
    iret



# system_call_linkage
#   Description: asm linkage for system call (INT 0x80)
#   Input: eax - system call num; 
#          ebx - 1st arg; 
#          ecx - 2nd arg; 
#          edx - 3rd arg;
#   Output: none
#   Notice: iret is required as it is returned from an system call
#
system_call_linkage:
    # store registers
    pushl %esp
    pushl %ebp
    pushl %esi
    pushl %edi
    pushfl

    # push arguments on stack for C convention
    pushl %edx
    pushl %ecx
    pushl %ebx
    
    # check system call (1-10) number in eax
    cmpl $1, %eax
    jl system_call_fail
    cmpl $10, %eax
    jg system_call_fail

system_call_do:
    # call corresponding system call function
    addl $-1, %eax
    call *jump_table(, %eax, 4)       # jump[cmd*4]

system_call_stack_pop:
    popl %ebx
    popl %ecx
    popl %edx
    popfl
    popl %edi
    popl %esi
    popl %ebp
    popl %esp
    iret

system_call_fail:
    popl %ebx
    popl %ecx
    popl %edx
    popfl
    popl %edi
    popl %esi
    popl %ebp
    popl %esp
    # set return value as -1 since system call fails
    movl $-1, %eax
    iret
