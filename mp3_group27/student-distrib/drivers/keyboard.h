/* keyboard.h - constants and function interface for keyboard.c */
/**
 * @file keyboard.h
 * @author: Xiaomin Qiu
 * @modifier: Guanshujie Fu
 * @description: headers for keyboard operation
 * @creat_date: 2022.5. - 
 *             2022.5.19 - move scancode_set and flags from header out to .c
 */
#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define TEST_MODE 1

#include "i8259.h"
#include "terminal.h"
#include "../lib.h"
#include "../types.h"
// constants
#define KB_DATA    0x60
#define KB_BUF_SIZE    128

volatile unsigned int KB_BUF_INDEX;

/* Keyboard buffer is 128 bytes */
unsigned char keyboard_buffer[KB_BUF_SIZE];

#if (TEST_MODE == 1)
    uint8_t key_pressed;
#endif

#define NUM_MODE 4
#define NUM_SCAN 0x3A
#define KB_IRQ 1


#define CAPS_PRESS      0x3A
#define L_SHIFT_PRESS     0x2A
#define L_SHIFT_RELEASE   0xAA
#define CTRL_PRESS      0x1D
#define CTRL_RELEASE    0x9D
#define R_SHIFT_PRESS     0x36
#define R_SHIFT_RELEASE   0xB6
#define ENTER_PRESS     0x1C
#define BACKSPACE       0x0E
#define TAB             0x0F
#define TAB_SPACE       4

#define ALT_PRESS       0x38
#define ALT_RELEASE     0xB8

#define F1_PRESS        0x3B
#define F2_PRESS        0x3C
#define F3_PRESS        0x3D

#define terminal_0 0
#define terminal_1 1
#define terminal_2 2



/* initialize keyboard for interrupt */
extern void keyboard_init(void);

/* handles interrupt and echos characters */
extern void keyboard_handler(void);


#endif
