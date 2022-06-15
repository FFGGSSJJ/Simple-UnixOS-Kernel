// All necessary declarations for the Tux Controller driver must be in this file

#ifndef TUXCTL_H
#define TUXCTL_H

#define TUX_SET_LED _IOR('E', 0x10, unsigned long)
#define TUX_READ_LED _IOW('E', 0x11, unsigned long*)
#define TUX_BUTTONS _IOW('E', 0x12, unsigned long*)
#define TUX_INIT _IO('E', 0x13)
#define TUX_LED_REQUEST _IO('E', 0x14)
#define TUX_LED_ACK _IO('E', 0x15)

/* see TUX_SET_LED*/
//

/* Size of cmd sending to tx_buf in driver*/
#define LED_BUF_SIZE 6
#define LED_NUM 4
#define LED_MASK  0x0F
/* LED representation of 0-F*/
#define ZERO 0xE7
#define ONE  0x06
#define TWO  0xCB
#define THREE 0x8F
#define FOUR 0x2E
#define FIVE 0xAD
#define SIX 0xED
#define SEVEN 0x86
#define EIGHT 0xEF
#define NINE 0xAE
#define A_ 0xEE
#define B_ 0x6D
#define C_ 0xE1
#define D_ 0x4F
#define E_ 0xE9
#define F_ 0xE8
unsigned char LED_SEG[16] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, A_, B_, C_, D_, E_, F_};

#endif

