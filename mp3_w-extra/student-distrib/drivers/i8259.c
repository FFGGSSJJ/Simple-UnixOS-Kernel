/**
 * @file: i8259.c
 * 
 * @ahthor: Zitai Kong
 * @modifier: Guanshujie Fu
 * @description: functions for rtc initialization and rate set
 * @creat_date: 2022.3.19 - IMPORTANT: initialize masks
 * @reference: https://wiki.osdev.org/PIC
 *             ece391 lecture codes
 */


/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "../lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* some predefined magic numbers */
#define ALL_MASK    0xFF   // active low
#define SLAVE_IRQ   2

/*
 * i8259_init
 * Description: Initialize the 8259 PIC
 * Input: none
 * Output: none
 * Return value: none
 * Side Effect: Initialize the 8259 PIC
 * 
*/
/* Initialize the 8259 PIC */
void i8259_init(void) {
    // The init framework fits that in lec10

    /* set the flag
     * since cli_and_save needs long type, use 32 bits */
    // uint32_t flag;

    /* create the critical section */
	// cli_and_save(flag);

    /* mask all interrupts */
    master_mask = ALL_MASK;
    slave_mask = ALL_MASK;
    outb(ALL_MASK, MASTER_8259_PORT + 1);   // mask all of 8259A-1 0x21
	outb(ALL_MASK, SLAVE_8259_PORT + 1);    // mask all of 8259A-2 0xA1

    /* initalize the master PIC */
	outb(ICW1, MASTER_8259_PORT);               // ICW1: select 8259A-1 init 0x11->0x20
	outb(ICW2_MASTER, MASTER_8259_PORT + 1);    // ICM2: 8259A-1 IR0-7 mapped to 0x20-0x27
	outb(ICW3_MASTER, MASTER_8259_PORT + 1);    // ICM3: 8259A-1 has a secondary on IRQ2
	outb(ICW4, MASTER_8259_PORT + 1);           // ICM4: since no auto_eoi use 0x01

    /* initalize the slave PIC */
	outb(ICW1, SLAVE_8259_PORT);                // ICW1: select 8259A-2 init 0x11->0x20
	outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);      // ICM2: 8259A-2 IR0-7 mapped to 0x28-0x2F
	outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);      // ICM3: 8259A-2 is a secondary on primary's IRQ2
	outb(ICW4, SLAVE_8259_PORT + 1);            // ICM4: use normal eoi

    /* enable the slave PIC connection */
	enable_irq(SLAVE_IRQ);

	/* end the critical section */
	// restore_flags(flag);
	// sti();

}


/*
 *
 * enable_irq
 * Description: Enable (unmask) the specified IRQ
 * Input: the number of IRQ to be enabled
 * Output: none
 * Return value: none
 * Side Effect: Enable (unmask) the specified IRQ
 * 
*/
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {

    /* set the flag
     * since cli_and_save needs long type, use 32 bits */
    // uint32_t flag;

    /* create the critical section */
	// cli_and_save(flag);

    // enable uses mask & ~(1 << irq_num) pattern
    // For master pic
    if (irq_num >= 0 && irq_num <= 7) {
        master_mask = master_mask & (~(1 << irq_num));
		outb(master_mask, (MASTER_8259_PORT+1));
    }
    // For slave pic
    else if (irq_num >= 8 && irq_num <= 15) {
		irq_num = irq_num - 8;  // 8 is the master IRQ number
        slave_mask = slave_mask & (~(1 << irq_num));
        outb(slave_mask, (SLAVE_8259_PORT+1));
    }
    // else do nothing

    /* end the critical section */
	// restore_flags(flag);
	// sti();

}


/*
 *
 * disable_irq
 * Description: Disable (mask) the specified IRQ
 * Input: the number of IRQ to be disabled
 * Output: none
 * Return value: none
 * Side Effect: Disable (mask) the specified IRQ
 * 
*/
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {

    /* set the flag
     * since cli_and_save needs long type, use 32 bits */
    // uint32_t flag;

    /* create the critical section */
	// cli_and_save(flag);

    // disable uses mask | (1 << irq_num) pattern
    // For master pic
    if (irq_num >= 0 && irq_num <= 7) {
        master_mask = master_mask | (1 << irq_num);
		outb(master_mask, (MASTER_8259_PORT+1));
    }
    // For slave pic
    else if (irq_num >= 8 && irq_num <= 15) {
		irq_num = irq_num - 8;  // 8 is the master IRQ number
        slave_mask = slave_mask | (1 << irq_num);
        outb(slave_mask, (SLAVE_8259_PORT+1));
    }
    // else do nothing

    /* end the critical section */
	// restore_flags(flag);
	// sti();
}

/*
 *
 * send_eoi
 * Description: Send end-of-interrupt signal for the specified IRQ
 * Input: the number of IRQ to send EOI
 * Output: none
 * Return value: none
 * Side Effect: Send end-of-interrupt signal for the specified IRQ
 * 
*/
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {

    /* set the flag
     * since cli_and_save needs long type, use 32 bits */
    // uint32_t flag;

    /* create the critical section */
	// cli_and_save(flag);

    /* EOI gets OR'd with irq_num
     * and sent out to the PIC
     * to declare the interrupt finished */

    // For the master PIC
    if(irq_num >= 0 && irq_num <= 7) {
		outb((EOI | irq_num), MASTER_8259_PORT);
	}
    // For the slave PIC
    // it is necessary to issue the command to both PIC chips
	else if(irq_num >= 8 && irq_num <= 15)
	{
        irq_num = irq_num - 8;              // 8 is the master IRQ number
		outb((EOI | irq_num), SLAVE_8259_PORT);
		outb((EOI | 2), MASTER_8259_PORT);    // 2 is the IRQ2 of master PIC
	}

    /* end the critical section */
	// restore_flags(flag);
	// sti();
}
