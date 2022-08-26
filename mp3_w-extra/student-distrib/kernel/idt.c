/**
 * @file idt.c
 * @author: Xiaomin Qiu
 * @modifier: Guanshujie Fu, Zitai Kong
 * @description: functions for idt, including exceptions
 * @creat_date: 2022.3. - 
 *             2022.3.18 - add linkages to idt
 *             2022.3.22 - add system call  entry
 *             2022.4.22 - add pit
 *             2022.4.30 - add mouse
 */



#include "idt.h"


/* DIVIDE_BY_ZERO
 * 
 * Deal with exception: DIVIDE_BY_ZERO
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void DIVIDE_BY_ZERO(){
    cli();
    printf("\nException Happened: Divide by Zero\n");
    sti();
    halt(EXCEPTION_STATUS);
}

/* DEBUG
 * 
 * Deal with exception: DEBUG
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void DEBUG(){
	cli();
    printf("Exception Happened: Debug Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* NON_MASKABLE
 * 
 * Deal with exception: NON_MASKABLE
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void NON_MASKABLE(){
	cli();
	clear();
    printf("Exception Happened: Non Maskable Interrupt Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* BREAKPOINT
 * 
 * Deal with exception: BREAKPOINT
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void BREAKPOINT(){
	cli();
    printf("Exception Happened: Breakpoint Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* OVERFLOW
 * 
 * Deal with exception: OVERFLOW
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void OVERFLOW(){
	cli();
	clear();
    printf("Exception Happened: Overflow Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* BOUND_RANGE_EXCEEDED
 * 
 * Deal with exception: BOUND_RANGE_EXCEEDED
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void BOUND_RANGE_EXCEEDED(){
	cli();
	clear();
    printf("Exception Happened: Bound Range Exceeded Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* INVALID_OPCODE_EXCEPTION
 * 
 * Deal with exception: INVALID_OPCODE_EXCEPTION
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void INVALID_OPCODE_EXCEPTION(){
	cli();
	clear();
    printf("Exception Happened: Invalid Opcode Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* DEVICE_NOT_AVAILABLE
 * 
 * Deal with exception: DEVICE_NOT_AVAILABLE
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void DEVICE_NOT_AVAILABLE(){
	cli();
	clear();
    printf("Exception Happened: Device Not Available Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* DOUBLE_FAULT
 * 
 * Deal with exception: DOUBLE_FAULT
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void DOUBLE_FAULT(){
	cli();
	clear();
    printf("Exception Happened: Double Fault Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* COPROCESSOR_SEGMENT_OVERRUN
 * 
 * Deal with exception: COPROCESSOR_SEGMENT_OVERRUN
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void COPROCESSOR_SEGMENT_OVERRUN(){
	cli();
	clear();
    printf("Exception Happened: Coprocessor Segment Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* INVALID_TSS
 * 
 * Deal with exception: INVALID_TSS
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void INVALID_TSS(){
	cli();
	clear();
    printf("Exception Happened: Invalid TSS Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* SEG_NOT_PRESENT
 * 
 * Deal with exception: SEG_NOT_PRESENT
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void SEG_NOT_PRESENT(){
	cli();
	clear();
    printf("Exception Happened: Segment Not Present");
    sti();
    halt(EXCEPTION_STATUS);
}

/* STACK_SEGMENT_FAULT
 * 
 * Deal with exception: STACK_SEGMENT_FAULT
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void STACK_SEGMENT_FAULT(){
	cli();
	clear();
    printf("Exception Happened: Stack Fault Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* GENERAL_PROTECTION_FAULT
 * 
 * Deal with exception: GENERAL_PROTECTION_FAULT
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void GENERAL_PROTECTION_FAULT(){
	cli();
    printf("Exception Happened: General Protection Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* PAGE_FAULT
 * 
 * Deal with exception: PAGE_FAULT
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void PAGE_FAULT(){
	cli();
    printf("\nException Happened: Page Fault Exception\n");
    sti();
    halt(EXCEPTION_STATUS);
}

/* FLOAT_FAULT
 * 
 * Deal with exception: FLOAT_FAULT
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void FLOAT_FAULT(){
	cli();
	clear();
    printf("Exception Happened: Floating Point Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* ALIGNMENT_CHECK
 * 
 * Deal with exception: ALIGNMENT_CHECK Fault
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void ALIGNMENT_CHECK(){
	cli();
	clear();
    printf("Exception Happened: Alignment Check Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* MACHINE_CHECK
 * 
 * Deal with exception: MACHINE_CHECK Fault
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void MACHINE_CHECK(){
	cli();
	clear();
    printf("Exception Happened: Machine Check Exception");
    sti();
    halt(EXCEPTION_STATUS);
}

/* SIMD_FLOATING_POINT
 * 
 * Deal with exception: SIMD_FLOATING_POINT
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print exception
 */
void SIMD_FLOATING_POINT(){
	cli();
	clear();
    printf("Exception Happened: SIMD Floating Poicase" );
    sti();
    halt(EXCEPTION_STATUS);
}

/* SYSTEM_CALL
 * 
 * Deal with system call
 * Inputs: None
 * Outputs: None
 * Side Effects: stop processing; blue screen; print syscall
 */
void SYSTEM_CALL(){
	cli();
    printf("\nSystem Call Happened\n" );
    while(1);
    sti();
}




/* interrupt_init
 * 
 * Initialize the IDT.
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void interrupt_init(void){
    int i;
    for (i = 0; i < NUM_VEC; i++){
        //0x14 - 0x 19 not use
        if (i < 32 && i > 19) {
            idt[i].present = 0;
            continue;
        } 
        // 0b1110 : 64-bit Interrupt Gate
        // 0b1111 : 64-bit Trap Gate
        //reserved3: 0 - interupt gate(32-255,2,14); 1 - trap gate (0-31)
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = (i < 32 && i != 2) ? 1 : 0;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved0 = 0;
        //size: 0:16 bit; 1: 32bit
        idt[i].size      = 1;
        //DPL: A 2-bit value which defines the CPU Privilege Levels which are allowed to access this interrupt via the INT instruction. 
        //Hardware interrupts ignore this mechanism.
        idt[i].dpl       = DPL_KERNEL; 
        //P: Present bit. Must be set (1) for the descriptor to be valid.
        idt[i].present   = 1;
    }


    // Seg idt entries
    SET_IDT_ENTRY(idt[0], DIVIDE_BY_ZERO);
	SET_IDT_ENTRY(idt[1], DEBUG);
	SET_IDT_ENTRY(idt[2], NON_MASKABLE);
	SET_IDT_ENTRY(idt[3], BREAKPOINT);            idt[3].dpl = DPL_USER;
	SET_IDT_ENTRY(idt[4], OVERFLOW);              idt[4].dpl = DPL_USER;
	SET_IDT_ENTRY(idt[5], BOUND_RANGE_EXCEEDED);  idt[5].dpl = DPL_USER;
	SET_IDT_ENTRY(idt[6], INVALID_OPCODE_EXCEPTION);
	SET_IDT_ENTRY(idt[7], DEVICE_NOT_AVAILABLE);
	SET_IDT_ENTRY(idt[8], DOUBLE_FAULT);
	SET_IDT_ENTRY(idt[9], COPROCESSOR_SEGMENT_OVERRUN);
	SET_IDT_ENTRY(idt[10], INVALID_TSS);
	SET_IDT_ENTRY(idt[11], SEG_NOT_PRESENT);
	SET_IDT_ENTRY(idt[12], STACK_SEGMENT_FAULT);
	SET_IDT_ENTRY(idt[13], GENERAL_PROTECTION_FAULT);
	SET_IDT_ENTRY(idt[14], PAGE_FAULT);
	SET_IDT_ENTRY(idt[16], FLOAT_FAULT);
	SET_IDT_ENTRY(idt[17], ALIGNMENT_CHECK);
	SET_IDT_ENTRY(idt[18], MACHINE_CHECK);
    SET_IDT_ENTRY(idt[19], SIMD_FLOATING_POINT);

    // Temp added for system call.
    SET_IDT_ENTRY(idt[SYSCALL], system_call_linkage);
    idt[SYSCALL].dpl = DPL_USER;
    idt[SYSCALL].reserved3 = 1;

    // linked in idt_linkage
    SET_IDT_ENTRY(idt[PIT], pit_handler_linkage); 
    SET_IDT_ENTRY(idt[RTC], rtc_handler_linkage); 
    SET_IDT_ENTRY(idt[KEYBOARD], keyboard_handler_linkage);
    SET_IDT_ENTRY(idt[MOUSE], mouse_handler_linkage);
}
