/**
 * @file: rtc.c
 * 
 * @ahthor: Zitai Kong
 * @modifier: Guanshujie Fu
 * @description: functions for rtc initialization and rate set
 * @creat_date: 2022.3. - 
 *             2022.3.20 - rtc_switch tests
 * @reference: https://wiki.osdev.org/RTC
 *            
 */

#include "rtc.h"

// The flag denotes rtc interrupts happening 
// volatile int32_t rtc_flag;
int32_t rtc_flag[TERMINAL_NUM] = {0};
int32_t rtc_count[TERMINAL_NUM] = {0};
int32_t rtc_max_count[TERMINAL_NUM] = {32,32,32}; // 32 is the default count

#define my_rate 32

/* 
 * rtc_init
 *  DESCRIPTION: Initialize real time clock
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: enable rtc IRQ 
 */
void rtc_init(void)
{
    /* set the flag
     * since cli_and_save needs long type, use 32 bits */
    // uint32_t flag;

    /* create the critical section */
	// cli_and_save(flag);     // disable interrupts

    /* disable rtc showing */
    turn_off_rtc();

    // clear rtc_flag to make read ends
    // rtc_flag = 0;   

    /* Avoiding NMI and Other Interrupts While Programming */
    // disable other interrupts - done by cli()

    // disable NMI
    // taken from osdev/NMI 
    // outb(0x70, inb(0x70) | 0x80);
    // inb(0x71);
    outb(inb(IDX_PORT) | NMI_DISABLE, IDX_PORT);

    /* Turn on IRQ8 */
    outb(SREG_B,IDX_PORT);              // select register B, and disable NMI
    char prev=inb(DATA_PORT);	        // read the current value of register B
    outb(SREG_B,IDX_PORT);              // set the index again (a read will reset the index to register D)
    outb(prev | BIT_SIX, DATA_PORT);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
                                        // This enables periodic interrupt
    enable_irq(RTC_IRQ);                      // Turn on IRQ8

    /* Set to base interrupt rate */
    //set_irate(BASE_RATE);
    int32_t frequency = my_rate;
    int32_t rate = 0;
    char prev2;
    // find rate by applying
    // frequency =  32768 >> (rate-1);
	frequency = TOP_RATE / frequency;
	while(frequency > 0)
	{
		rate = rate + 1;
		frequency = frequency >> 1;
	}
    rate &= UP_MASK;			// rate must be above 2 and not over 15

    // disable interrupts and NMI
    // cli();

    // set interrupt rate
    outb(SREG_A,IDX_PORT);		                // set index to register A, disable NMI
    prev2=inb(DATA_PORT);	                // get initial value of register A
    outb(SREG_A,IDX_PORT);		                // reset index to A
    outb((prev2 & DOWN_MASK) | rate, DATA_PORT); //write only our rate to A. Note, rate is the bottom 4 bits.

    // enable interrupts
    // sti();

    /* enable NMI */
    // taken from osdev/NMI 
    // outb(0x70, inb(0x70) & 0x7F);
    // inb(0x71);
    //outb(inb(IDX_PORT) & NMI_ENABLE, IDX_PORT);

    /* end the critical section */
	// restore_flags(flag);
	// sti();
}


/* 
 * rtc_handler
 *  DESCRIPTION: handles rtc interrupt
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: execute test_interrupt handler
 */
void rtc_handler(void)
{
        // cli();

        /* Execute test_interrupt handler */
        // if (get_rtc_switch() == 1)
        //     test_interrupts();
        // printf("RTC interrupt happens!\n");

        /* get another interrupt */
        outb(SREG_C,IDX_PORT);  // select register C
        // This line read the data from data port but throw it away
        inb(DATA_PORT);		    // just throw away contents
        send_eoi(RTC_IRQ);

        // clear rtc_flag to make read ends
        // rtc_flag = 0;
        int i;
        for (i = 0; i < TERMINAL_NUM; i++) {
            if (rtc_flag[i] == 1)
            {
                if (rtc_count[i] == 0)
                {
                    rtc_count[i] = rtc_max_count[i];
                    rtc_flag[i] = 0;
                }
                rtc_count[i]--;
            }
            // rtc_flag[i] = 0;
        }

        // sti();

        /* send eoi to end the interrupt */
    
}


/* 
 * rtc_open
 *  DESCRIPTION: open rtc file
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: 0 if success, -1 if fail
 *  SIDE EFFECTS: open the rtc file
 */
int32_t rtc_open(void)
{
    // int32_t retval;
    // retval = set_rate(BOTTOM_RATE);
    // if (retval == -1) {
    //     return -1;
    // } else {
    //     return 0;
    // }
    return 0;
}



/* 
 * rtc_read
 *  DESCRIPTION: read data from rtc 
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: 0 always after an interrupt occurs
 *  SIDE EFFECTS: none
 */
int32_t rtc_read(uint32_t inode_num, int32_t position, void* buf, int32_t nbytes)
{
    /* it is tricky because we have no idea when the IF is clear
     * we have to add a sti here to enable rtc interrupt */
    // sti();
    // set a flag
    int32_t tid = (process_terminal)->tid;
    rtc_flag[tid] = 1;
    // wait until the interrupt handler cleans it
    while(rtc_flag[tid]);

    // return 0 always after an interrupt occurs
    return 0;
}


/* 
 * rtc_write
 *  DESCRIPTION: write data to rtc device
 *  INPUTS: fd - the rtc file descriptor
 *          buf - pointer to a buffer containing the rate
 *          nbytes - no. of bytes to write 
 *  OUTPUTS: none
 *  RETURN VALUE: # of bytes written if success
 *                -1 if fail
 *  SIDE EFFECTS: set the interrupt rate
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)
{
    int32_t rate;
    int32_t retval;

    // accept only a 4-byte int
    if (nbytes != 4) {
        return -1;
    }

    // check if the ptr to the rate data is correct
    if (buf == NULL) {
        return -1;
    }

    // disable all interrupts
    // cli();

    // set the interrupt rate
    rate = *(int32_t*) buf;
    if (-1 == (retval = set_rate(rate)))
        return -1;

    // sti();

    return nbytes;
}


/* 
 * rtc_close
 *  DESCRIPTION: close rtc file descriptor and make it available
 *               for return from later calls to open
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: 0 on success, -1 on failure
 *  SIDE EFFECTS: none
 */
int32_t rtc_close(void)
{
    // rtc_close do not need to do anything
    int32_t tid = (process_terminal)->tid;
    rtc_max_count[tid] = 32; // 32 is the default count
    return 0;
}


/*
 * set_rate
 * DESCRIPTION: Set the interrupt rate of rtc driver
 * INPUTS: irate - the interrupt rate
 * OUTPUTS : none
 * RETURN VALUE: 0 on success, -1 on failure
 * SIDE EFFECT: change interrupt rate
*/
int32_t set_rate(int32_t irate)
{
    /* set the flag
     * since cli_and_save needs long type, use 32 bits */
    // uint32_t flag;

    /* create the critical section */
	// cli_and_save(flag);     // disable interrupts

    // check whether the interrupt rate is valid
    // check range
    if ((irate < BOTTOM_RATE) || (irate > RATE1024)) {
        printf("Warning: Interrupt rate is out of range!\n");
        return -1;
    }
    // check power of 2
    switch (irate)
    {
    case BOTTOM_RATE:
        break;
    case RATE4:
        break;
    case RATE8:
        break;
    case RATE16:
        break;
    case RATE32:
        break;
    case RATE64:
        break;
    case RATE128:
        break;
    case RATE256:
        break;
    case RATE512:
        break;
    case RATE1024:
        break;
    default:
        printf("Warning: Interrupt rate should be power of 2!\n");
        return -1;
    }

    // record in rtc_max_count
    int32_t tid = (process_terminal)->tid;
    rtc_max_count[tid] = my_rate/irate;

    // int32_t frequency = irate;
    // int32_t rate = 0;
    // char prev;
    // find rate by applying
    // frequency =  32768 >> (rate-1);
	// frequency = TOP_RATE / frequency;
	// while(frequency > 0)
	// {
	// 	rate = rate + 1;
	// 	frequency = frequency >> 1;
	// }
    // rate &= UP_MASK;			// rate must be above 2 and not over 15

    // disable interrupts and NMI
    // cli();

    // set interrupt rate
    // outb(SREG_A,IDX_PORT);		                // set index to register A, disable NMI
    // prev=inb(DATA_PORT);	                // get initial value of register A
    // outb(SREG_A,IDX_PORT);		                // reset index to A
    // outb((prev & DOWN_MASK) | rate, DATA_PORT); //write only our rate to A. Note, rate is the bottom 4 bits.

    // enable interrupts
	// restore_flags(flag);
	// sti();

    return 0;
}
