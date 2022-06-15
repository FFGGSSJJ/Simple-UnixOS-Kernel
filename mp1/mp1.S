
.data					# section declaration

        # Useful offset constants for accessing members of a 
        # struct mp1_blink_struct structure
        LOCATION   = 0    
        ON_CHAR    = 2
        OFF_CHAR   = 3 
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0

.text					# section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX
mp1_poke:
	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)       
	ret



# void mp1_rtc_tasklet(u_long arg);
#
# Interface: Stack-based arguments (not C-style)
#    Inputs: arg
#   Outputs: %eax
# Registers: ESI -- store the poiners to linked list
#            EBX -- store countdown, status
#            EDI -- not used
mp1_rtc_tasklet:
        pushl %ebp              # start new stack frame for mp1_rtc_tasklet
        movl %esp, %ebp
        pushl %ebx
        pushl %esi
        pushl %edi

        movl mp1_list_head, %esi        # esi <- ptr
loop_rtc:
        cmpl $0, %esi           # check if it reaches the end
        je  ret_endlist

        # decrement countdown
        xorl %ebx, %ebx
        movw COUNTDOWN(%esi), %bx
        subw $1, %bx
        movw %bx, COUNTDOWN(%esi)

        cmpl $0, %ebx          # check if countdown is 0
        je change_status
        # countdown > 0, move to next
        movl NEXT(%esi), %esi
        jmp loop_rtc

change_status:
        movw STATUS(%esi), %bx
        cmpl $1, %ebx   
        je  turn_off     # if status == 1, current is on and need to be off
        jmp turn_on      # else, turn on
turn_off:
        xorl %eax, %eax
        movw LOCATION(%esi), %ax    # ax <- location offset
        cmpw $1999, %ax
        ja   ret_err            # if > 80*25-1
        cmpw $0, %ax
        jb   ret_err            # if < 0
        shll $1, %eax         # double the offset
        movb OFF_CHAR(%esi), %cl# turn off
        call mp1_poke
        # update countdown to off_length
        movw OFF_LENGTH(%esi), %cx
        movw %cx, COUNTDOWN(%esi)
        # update status to 0 and move to next
        movw $0, STATUS(%esi)
        movl NEXT(%esi), %esi
        jmp loop_rtc
turn_on:
        xorl %eax, %eax
        movw LOCATION(%esi), %ax    # ax <- location offset
        cmpw $1999, %ax
        ja   ret_err            # if > 80*25-1
        cmpw $0, %ax
        jb   ret_err            # if < 0
        shll $1, %eax         # double the offset
        movb ON_CHAR(%esi), %cl # turn on
        call mp1_poke
        # update countdown to on_length
        movw ON_LENGTH(%esi), %cx
        movw %cx, COUNTDOWN(%esi)
        # update status to 1 and move to next
        movw $1, STATUS(%esi)
        movl NEXT(%esi), %esi
        jmp loop_rtc
ret_endlist:
        # has loop to the end of list
        xorl %eax, %eax
        popl %edi       # restore callee saved registers
        popl %esi
        popl %ebx
        leave           # restore ebp, esp
        ret
ret_err:
        # err occurs as location exceeds range
        movl $-1, %eax
        popl %edi       # restore callee saved registers
        popl %esi
        popl %ebx
        leave           # restore ebp, esp
        ret


# void mp1_ioctl(u_long arg, cmd);
#
# Interface: Stack-based arguments (not C-style)
#    Inputs: arg
#            cmd
#   Outputs: %eax
# Registers: EDX -- store cmd and compare
mp1_ioctl:
        movl 8(%esp), %edx      # set edx = cmd, where is ebp?
        cmpl $3, %edx           # compare cmd with 3
        ja   ret_invalid_cmd        # if cmd > 3
        subl $3, %edx
        cmpl $-3, %edx          # compare cmd-3 with -3
        jl   ret_invalid_cmd        # if cmd - 3 < -3 => cmd < 0

        addl $3, %edx
        jmp  *jump_table(,%edx,4)       # jump[cmd*4]
	ret
ret_invalid_cmd:
        xorl %eax, %eax         # eax <- 0
        movl $-1, %eax          # eax <- -1
        ret     # no need for leave since ebp and esp not changed

# void mp1_mp1_ioctl_addioctl(u_long arg);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: arg -- pointer to mp1_blink_struct
#    
#   Outputs: %eax -- 0 for success and -1 for fail
# Registers: EBX -- store arg pointer
#            ESI -- store the pointer after malloc
mp1_ioctl_add:
        pushl %ebp              # start new stack frame for mp1_ioctl_add
        movl %esp, %ebp
        pushl %ebx              # callee save registers
        pushl %esi

        movl 8(%ebp), %ebx      # ebx <- arg
        cmpl $0, %ebx
        je  ret_null

        # first allocate space for the structure
        pushl $STRUCT_SIZE      # push size argument for mp1_malloc
        call mp1_malloc
        addl $4, %esp           # pop argument, now %eax contains a pointer
        movl %eax, %esi         # esi <- eax

        cmpl $0, %esi            # check if the pointer is NULL
        je  ret_null

        # then copy the struct into the allocated sapce
        # mp1_copy_from_user (void *to, const void *from, unsigned long n)
        pushl $STRUCT_SIZE
        pushl %ebx      # *from
        pushl %esi      # *to
        call mp1_copy_from_user
        addl $12, %esp

        cmpl $0, %eax
        jne  ret_and_free

        # set countdown and status
        xorl %eax, %eax         # eax <- 0
        movw ON_LENGTH(%esi), %ax       # unsined char 16 bits
        movw %ax, COUNTDOWN(%esi)        
        movw $1, STATUS(%esi)

        # insert to the head of the list
        movl mp1_list_head, %eax
        movl %eax, NEXT(%esi)
        movl %esi, mp1_list_head

        # check location
        xorl %eax, %eax         # eax <- 0
        movw LOCATION(%esi), %ax        # 16 bits, ax <- location
        cmpw $1999, %ax
        ja   ret_and_free       # if > 80*25-1
        cmpw $0, %ax
        jb   ret_and_free       # if < 0

        # call mp1_poke ???
        movb ON_CHAR(%esi), %cl
        addl %eax, %eax
        call mp1_poke

        # all done, return 0
        xorl %eax, %eax         # eax <- 0
        # restore registers
        popl %esi
        popl %ebx
        leave
	ret

ret_null:
        xorl %eax, %eax         # eax <- 0
        movl $-1, %eax          # eax <- -1
        # restore registers
        popl %esi
        popl %ebx
        leave
        ret
ret_and_free:
        # free malloc space
        pushl %esi
        call mp1_free
        addl $4, %esp

        xorl %eax, %eax         # eax <- 0
        movl $-1, %eax          # eax <- -1
        # restore registers
        popl %esi
        popl %ebx
        leave
        ret


# void mp1_ioctl_remove(u_long arg);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: arg -- location of a mp1_blink_struct
#    
#   Outputs: %eax -- 0 for success and -1 for fail
# Registers: EAX -- store previous pointer
#            EBX -- store arg location
#            ESI -- store the pointer of linked list
#            EDI -- store the location in each node
mp1_ioctl_remove:
        pushl %ebp              # start new stack frame for mp1_ioctl_add
        movl %esp, %ebp
        pushl %ebx              # callee save registers
        pushl %esi
        pushl %edi

        movl mp1_list_head, %esi# esi <- ptr
        xorl %ebx, %ebx
        movl 8(%ebp), %ebx      # ebx <- arg
        # check location argument
        cmpl $1999, %ebx
        ja   ret_fail           # if > 80*25-1
        cmpl $0, %ebx
        jb   ret_fail           # if < 0

        xorl %eax, %eax         # eax <- 0
rm_loop:
        # check pointer
        cmpl $0, %esi
        je   ret_fail
        # start search
        xorl %edi, %edi
        movw LOCATION(%esi), %di# di <- location
        cmpl %ebx, %edi
        je   find_rm            # if find the location

        movl %esi, %eax         # store current ptr in eax
        movl NEXT(%esi), %esi   # move to next
        jmp  rm_loop

find_rm:
        # check if eax is NULL, which means the first ptr is target
        cmpl $0, %eax
        je   remove_ptr
        # link two nodes
        movl NEXT(%esi), %ebx
        movl %ebx, NEXT(%eax) 
        jmp free_ret  
remove_ptr:
        # update head ptr
        movl NEXT(%esi), %ebx
        movl %ebx, mp1_list_head
free_ret:    
        # free %esi
        pushl %esi
        call mp1_free
        addl $4, %esp
        # return 0
        xorl %eax, %eax
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret
ret_fail:
        movl $-1, %eax
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret




# void mp1_ioctl_find(u_long arg);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: arg -- pointer of a mp1_blink_struct
#    
#   Outputs: %eax -- 0 for success and -1 for fail
# Registers: EAX -- store previous pointer
#            EBX -- store the target pointer
#            ESI -- store the malloc ptr in kernel space
#            EDI -- store the location in arg ptr
mp1_ioctl_find:
        pushl %ebp              # start new stack frame for mp1_ioctl_add
        movl %esp, %ebp
        pushl %ebx              # callee save registers
        pushl %esi
        pushl %edi

        # first allocate space for the structure
        pushl $STRUCT_SIZE      # push size argument for mp1_malloc
        call mp1_malloc
        addl $4, %esp           # pop argument, now %eax contains a pointer
        cmpl $0, %eax           # eax <- malloc ptr
        je   ret_fail_d
        movl %eax, %esi         # esi <- malloc ptr in kernel space
        
        # copy the arg ptr from user space to knernel space
        movl 8(%ebp), %ebx      # ebx <- arg ptr in user space
        cmpl $0, %ebx           # check if the arg ptr is valid
        je   ret_find_fail
        # then copy the struct into the allocated sapce
        # mp1_copy_from_user (void *to, const void *from, unsigned long n)
        pushl $STRUCT_SIZE
        pushl %ebx      # *from -- arg ptr
        pushl %esi      # *to -- malloc ptr
        call mp1_copy_from_user
        addl $12, %esp

        cmpl $0, %eax   # check if copy success
        jne  ret_find_fail

        # check the value of location
        xorl %edi, %edi
        movw LOCATION(%esi), %di     # di <- location
        cmpl $1999, %edi
        ja   ret_find_fail           # if > 80*25-1
        cmpl $0, %edi
        jb   ret_find_fail           # if < 0

        # start search by calling search_list
        # no caller saved register is used and hence I omit the caller save part
        pushl %edi
        call search_list
        addl $4, %esp
        cmpl $-1, %eax
        je   ret_find_fail
        # now eax contains the target ptr
        movl %eax, %ebx         # %ebx <- target ptr

        # call mp1_copy_to_user (void *to, const void *from, unsigned long n)
        pushl $STRUCT_SIZE
        pushl %ebx      # from -- target ptr
        pushl 8(%ebp)   # to -- arg ptr, not malloc ptr
        call mp1_copy_to_user
        addl $12, %esp

        # check if copy succes
        cmpl $0, %eax
        jne  ret_find_fail

        # return 0 and free malloc ptr
        # free malloc space
        pushl %esi
        call mp1_free
        addl $4, %esp

        xorl %eax, %eax
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret
ret_find_fail:
        # free malloc space
        pushl %esi
        call mp1_free
        addl $4, %esp
ret_fail_d:
        # return -1
        movl $-1, %eax
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret



# void mp1_ioctl_sync(u_long arg);
#
# Interface: Stack-based arguments (not C-style)
#    Inputs: arg -- two 2-byte u_short integers
#    
#   Outputs: %eax -- 0 for success and -1 for fail
# Registers: EAX -- store first location and then store the correspondent ptr
#            EBX -- store second location and then store the correspondent ptr
#            ESI -- store the pointer of linked list
#            EDI -- store temp val 
mp1_ioctl_sync:
        pushl %ebp              # start new stack frame for mp1_ioctl_add
        movl %esp, %ebp
        pushl %ebx              # callee save registers
        pushl %esi
        pushl %edi

        xorl %edi, %edi
        movl 8(%ebp), %edi      # edi <- u_long int
        movl mp1_list_head, %esi# esi <- list ptr
        # extract integers into eax and ebx
        xorl %ebx, %ebx
        xorl %eax, %eax
        movl %edi, %eax         # eax <- 32-bit int, ax <- sec location
        movw %ax, %bx           # bx <- sec locatino
        shrl $16, %eax          # ax <- first location
        andl $0x0000FFFF, %eax  # make sure only %ax contains location bit
        #xorl %ecx, %ecx         # clear flag to 0
first_match:
        # search first location
        cmpl $0, %esi
        je sync_fail            # one location not find

        xorl %edi, %edi
        movw LOCATION(%esi), %di# store current location into edi
        call location_check
        cmpl $-1, %edi
        je   sync_fail
        
        cmpl %eax, %edi         # try to match first location
        je   store_first        # current location matches first one

        movl NEXT(%esi), %esi   # move to next
        jmp  first_match

        # search first location by calling search_list
        pushl %eax
        call  search_list
store_first:
        movl %esi, %eax         # store the first pointer into eax
        movl mp1_list_head, %esi# reset the pointer to linked list head
second_match:
        # search second location
        cmpl $0, %esi
        je sync_fail            # location not find
        
        xorl %edi, %edi
        movw LOCATION(%esi), %di# store current location into edi
        call location_check
        cmpl $-1, %edi
        je   sync_fail
        #cmpl $1999, %edi
        #ja   sync_fail           # if > 80*25-1
        #cmpl $0, %edi
        #jb   sync_fail           # if < 0

        cmpl %ebx, %edi
        je  sync_cp             # second match, jump to copy

        movl NEXT(%esi), %esi   # move to next
        jmp second_match
sync_cp:
        movl %esi, %ebx         # store the second pointer into ebx
        # eax: first pointer; ebx: second pointer
        # copy from first into second
        xorl %edi, %edi
        movw ON_LENGTH(%eax), %di       # copy ON_LENGTH
        movw %di, ON_LENGTH(%ebx)

        xorl %edi, %edi
        movw OFF_LENGTH(%eax), %di       # copy OFF_LENGTH
        movw %di, OFF_LENGTH(%ebx)

        xorl %edi, %edi
        movw COUNTDOWN(%eax), %di       # copy COUNTDOWN
        movw %di, COUNTDOWN(%ebx)

        xorl %edi, %edi
        movw STATUS(%eax), %di          # copy STATUS
        movw %di, STATUS(%ebx)

        # call mp1_poke for second location %ebx
        xorl %edi, %edi
        xorl %eax, %eax
        xorl %ecx, %ecx
        movw LOCATION(%ebx), %ax        # ax <- location
        shll $1, %eax                   # double the offset
        # check status
        movw STATUS(%ebx), %di          # di <- status
        cmpl $0, %edi                   # if status = 0, display off_char
        je   off_poke
on_poke:
        movb ON_CHAR(%esi), %cl
        call mp1_poke
        jmp  sync_succ
off_poke:
        movb OFF_CHAR(%esi), %cl
        call mp1_poke
sync_succ:
        xorl %eax, %eax
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret
sync_fail:
        movl $-1, %eax
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret



# u_long int search_list(u_long arg);
#
# Interface: Stack-based arguments (in C-style)
#    Inputs: location
#    
#   Outputs: %eax -- -1 for out range and 0 for null
# Registers: EAX -- temp val used and output val
#            EBX -- not used
#            ESI -- list pointer
#            EDI -- arg location
#            No caller saved registers used.
search_list:
        pushl %ebp              # start new stack frame for search_list
        movl %esp, %ebp
        pushl %ebx              # callee save registers
        pushl %esi
        pushl %edi


        movl mp1_list_head, %esi# esi <- list ptr
        xorl %edi, %edi
        movl 8(%ebp), %edi      # edi <- u_short int
search_loop:
        # check esi ptr
        cmpl $0, %esi
        je   ret_search_fail
        xorl %eax, %eax
        movw LOCATION(%esi), %ax
        # check location range
        cmpl $1999, %eax
        ja   ret_search_fail           # if > 80*25-1
        cmpl $0, %eax
        jb   ret_search_fail           # if < 0
        # check location match
        cmpl %eax, %edi         
        je   ret_search_succ
        # move to next
        movl NEXT(%esi), %esi   
        jmp  search_loop

ret_search_succ:
        movl %esi, %eax         # eax <- target ptr 
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret
ret_search_fail:
        movl $-1, %eax
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret





# void location_check(u_long arg); NOT USED
#
# Interface: Reg-based arguments (not C-style)
#    Inputs: %edi -- location
#    
#   Outputs: %edi -- -1 for out range
# Registers: EAX -- store first location and then store the correspondent ptr
location_check:
        cmpl $1999, %edi
        ja   overrange           # if > 80*25-1
        cmpl $0, %edi
        jb   overrange           # if < 0
        ret
overrange:
        movl $-1, %edi
        ret


# jump table used for mp1_ioctl
jump_table:
.long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync


.end
