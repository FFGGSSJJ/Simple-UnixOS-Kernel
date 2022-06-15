/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

// Added Global variables

/*| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |*/
/*| R | L | D | U | c | b | a | s |*/
unsigned int button_;
/* Used by TUX_INIT -> tuxctl_ldisc_put() -> tuxctl_ldisc_write_wakeup() -> tty->driver->write()*/		
unsigned char init_vars[2] = {MTCP_BIOC_ON, MTCP_LED_USR};
/* Used by tuxctl_ioctl. When one cmd is executed, lock it and prevent other cmd. */
static spinlock_t tux_lock;
/* */
int ack;
unsigned long prev_ledarg;
int tuxctl_ioctl_led(struct tty_struct* tty, unsigned long arg);



/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
	unsigned long flags;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

	switch (a) {
	case MTCP_ACK:
		spin_lock_irqsave(&tux_lock, flags);
		ack = 1;
		spin_unlock_irqrestore(&tux_lock, flags);
		break;

	case MTCP_POLL:		// will not be used as we enable MTCP_BIOC_ON
		
		spin_lock_irqsave(&tux_lock, flags);
		button_ |= (0x0F & ~b);
		button_ |= (0x09 & ~c)<<4;
		button_ |= (0x02 & ~c)<<5;
		button_ |= (0x04 & ~c)<<3;
		spin_unlock_irqrestore(&tux_lock, flags);
		break;

	case MTCP_BIOC_EVENT:

		spin_lock_irqsave(&tux_lock, flags);
		button_ &= 0x00;
		button_ |= (0x0F & ~b);
		button_ |= (0x09 & ~c)<<4;
		button_ |= (0x02 & ~c)<<5;
		button_ |= (0x04 & ~c)<<3;
		spin_unlock_irqrestore(&tux_lock, flags);
		break;
	case MTCP_RESET:
		tuxctl_ioctl(tty, NULL, TUX_INIT, 0);
		tuxctl_ioctl_led(tty, prev_ledarg);
		break;
	default:
		break;
	}

    /*printk("packet : %x %x %x\n", a, b, c); */
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
	unsigned long flags;
	int check;
    switch (cmd) {
	
	/* TUX_INIT
	 *	Description: send command into tx_buf, driver call MTCP_BIOC_ON, MTCP_LED_USR to set tux controller
	 *	Needed Args: tty_struct* tty
	 *				| put to tx_buf  |	  |   tx_buf to temp buf	|    | temp buf to driver|
	 *  TUX_INIT => tuxctl_ldisc_put() => tuxctl_ldisc_write_wakeup() => tty->driver->write()
	 * 	Side Effects: none. ACK will be returned from tux using call_back
	 *  *Assume that any user-level code that interacts with your device will call this ioctl before any others.
	 */ 
	case TUX_INIT:

		spin_lock_irqsave(&tux_lock, flags);
		button_ = 0x00;
		spin_unlock_irqrestore(&tux_lock, flags);

		tux_lock = SPIN_LOCK_UNLOCKED;
		ack = 0;
		check = tuxctl_ldisc_put(tty, init_vars, 2);
		if(check > 0)	return -EINVAL;
		return 0;
	
	/* TUX_BUTTONS
	 *	Description: sets the bits of the low byte corresponding to the currently pressed buttons
	 *	Needed Args: tty_struct* tty
	 *				 arg -- a pointer to a 32-bit integer
	 *				 button_ -- current button_ values
	 *	Side Effects: return button value (button_) in kernel to a user pointer (arg)
	 *				 
	 */
	case TUX_BUTTONS:
		if (arg == NULL)	return -EINVAL;
		else {
			// set the bits of the low byte in arg pointer corresponding to the button_
			// i.e. return the button status to the user

			spin_lock_irqsave(&tux_lock, flags);
			check = copy_to_user((int*)arg, &button_, 4);
			spin_unlock_irqrestore(&tux_lock, flags);

			if (check != 0) return -EINVAL;
			else return 0;	
		}

	/* TUX_SET_LED
	 *	Description: similar to init, it sends commands into tx_buf. Driver calls MTCP_LED_SET to set led in tux
	 *	Needed Args: tty_struct* tty
	 *				 arg -- a 32-bit integer
	 *	Side Effects: none. ACK will be returned from tux using call_back
	 */
	case TUX_SET_LED:
		// These two lines should be removed when test in input.c
		spin_lock_irqsave(&tux_lock, flags);
		if (!ack) {
			spin_unlock_irqrestore(&tux_lock, flags);
			return -EINVAL;	// if ack == 0, some other cmd has not been returned. i.e. other cmd id being executed
		} ack = 0;
		spin_unlock_irqrestore(&tux_lock, flags);
		return tuxctl_ioctl_led(tty, arg) == 0 ? 0 : -EINVAL;
	case TUX_LED_ACK:
		return 0;
	case TUX_LED_REQUEST:
		return 0;
	case TUX_READ_LED:
		return 0;
	default:
	    return -EINVAL;
    }
}


/*
; 	Mapping from 7-segment to bits
; 	The 7-segment display is:
;		  _A
;		F| |B
;		  -G
;		E| |C
;		  -D .dp
;
; 	The map from bits to segments is:
; 
; 	__7___6___5___4____3___2___1___0__
; 	| A | E | F | dp | G | C | B | D | 
; 	+---+---+---+----+---+---+---+---+
*/
int tuxctl_ioctl_led(struct tty_struct* tty, unsigned long arg)
{
	unsigned char buf[LED_BUF_SIZE];
	unsigned char dps[LED_NUM];		// dp for led0-3
	unsigned char led[LED_NUM];		// led val for led0-3
	int i;
	prev_ledarg = arg;
	buf[0] = MTCP_LED_SET;
	buf[1] = LED_MASK;

	for (i = 0; i < LED_NUM; i++) {
		led[i] = (arg >> (i*LED_NUM)) & LED_MASK;
		dps[i] = (arg >> 24) & (0x01 << i);
	}
	for (i = 2; i < LED_BUF_SIZE; i++) {
		buf[i] = dps[i-2]==0 ? LED_SEG[(int)led[i-2]] : (LED_SEG[(int)led[i-2]] | 0x10);
		buf[i] = ((arg >> (16+i-2))&(0x01)) == 0x01 ? buf[i]:0x00;
	}
	//if (((arg>>16)&(LED_MASK)) == 0x07)	buf[5] = 0x00;
	return tuxctl_ldisc_put(tty, buf, LED_BUF_SIZE);
}