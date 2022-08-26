#ifndef _MOUSE_H
#define _MOUSE_H
/* Constants */
#include "../lib.h"
#include "../types.h"
#include "i8259.h"
#include "vbe.h"

// ports of PS2 mouse
#define MOUSE_PORT 0x60     //the data port
#define CHECK_PORT 0x64
#define CURSOR_PORT 0x3C4

// Mouse Packet Info
/*
; Format of First 3 Packet Bytes sent by mouse
; The first byte
; Y overflow|X overflow|Y sign bit|X sign bit|Always 1|Middle Btn|Right Btn|Left Btn
; The second byte
; X movement
; The third byte
; Y movement
*/
#define Y_OVERFLOW   0x80
#define X_OVERFLOW   0x40
#define Y_SIGN       0x20
#define X_SIGN       0x10
#define ALWAYS_ONE   0x08
#define MID_BUTTON   0x04
#define RIGHT_BUTTON 0x02
#define LEFT_BUTTON  0x01

// Mouse Command Set
#define ACK 0xFA
#define RESET 0xFF
#define PRE_COMMAND 0xD4
#define GET_STATUS  0x20
#define ENABLE_PACKET 0xF4

// mouse irq number
#define MOUSE_IRQ 12

// GUI & VGA related parameters
// modify them after merge
#define VGA_WIDTH 720
#define VGA_HEIGHT 400
#define BLACK 0xFF000000
#define WHITE 0xFFFFFFFF
#define CURSOR_WIDTH 12
#define CURSOR_HEIGHT 12

// masks
#define BIT1MASK    1
#define BIT2MASK    2
#define BIT6MASK    0x20
#define BIT9MASK    0x100
#define CURSOR_SHIFT 5
#define X_MASK      0x0010
#define Y_MASK      0x0011
#define LOW_BYTE_MASK 0xFFFFFF00
#define MOUSE_COUNTER   10

/* Externally-visible functions */
extern void mouse_init(void);
extern void mouse_handler(void);
void wait_output_to_mouse();
void wait_input_to_mouse();

/* Externally-visible variables */
// global variables
extern uint32_t mouse_enabled;
// cursor data
extern int16_t cursor_x;
extern int16_t cursor_y;
extern int16_t prev_cursor_x;
extern int16_t prev_cursor_y;
// mouse button data
extern uint32_t left_pressed;
extern uint32_t right_pressed;
extern uint32_t middle_pressed;

#endif
