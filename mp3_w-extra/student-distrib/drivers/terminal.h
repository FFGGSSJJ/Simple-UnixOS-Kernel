/**
 * @file: terminal.h
 * 
 * @ahthor: Xiaomin Qiu, Zitai Kong
 * @modifier: 
 * @description: function headers and variables for terminal driver
 * @creat_date: 2022.3.
 */
#include "../types.h"
#include "../lib.h"
#include "../drivers/keyboard.h"
#include "../kernel/paging.h"
#include "../kernel/pcb.h"
#include "vbe.h"
#include "statusbar.h"

#ifndef _TERMINAL_H
#define _TERMINAL_H

/* constants */
#define BUFFER_SIZE         128
#define SCREEN_SIZE         0x1000      // the size for video memory
#define EMPTY               0x0
#define TERM_VID_BEGIN      0x00300000      // 3MB as the start of the terminal video memory
#define VID_SIZE            0x00001000      // 4KB size for video memory
#define TERMINAL_NUM        3
// some from lib.c
#define NUM_COLS            80
#define NUM_ROWS            25
#define ATTRIB              0x7
#define SHIFT_12            12

#define HITORY_BUF_SIZE     100

typedef struct terminal
{ 
    /* whether the terminal is reading from keystrokes */
    volatile int32_t TERMINAL_READ_FLAG;
    /* stand for whether this terminal has been opened */
    volatile int32_t terminal_active;
    /* store user's keystrokes to the terminal */
    int32_t tid;
    char buffer[BUFFER_SIZE];
    /* track the position of latest char in the buffer */
    int buf_index;
    /* store cursor position */
    int32_t cursor_x;
    int32_t cursor_y;
    int32_t put_mode;
    int32_t rtc_flag;
    int32_t rtc_rate;

    char history[HITORY_BUF_SIZE][BUFFER_SIZE];
    int32_t history_num;
    int32_t history_index;
    int32_t history_size;

    /* screen buffer to store the video content for current terminal */
    uint32_t* screen_buffer;
} terminal_t;

void terminal_switch(int32_t new_ter);
void search_history(int32_t flag);
void clear_history();

terminal_t terminal1;
terminal_t multi_terminals[TERMINAL_NUM];
/* stand for the current displaying terminal */
volatile int current_active_termid;

/* Externally-visible functions */

/* terminal_open: open the terminal and do initializations */
int32_t terminal_open(void);
/* terminal_close: close the terminal */
int32_t terminal_close(void);
/* write to the terminal from arg buf */
int32_t terminal_write(int32_t fd, char *buf, int32_t nbytes);
/* read from the terminal. */
int32_t terminal_read(uint32_t inode_num, int32_t position, char *buf, int32_t nbytes);
/* initialize terminals */
int32_t terminal_init();
/* get avtive terminal */
terminal_t* get_active_terminal();
/* switch terminal*/
void terminal_switch(int32_t terminal_id);

#define process_terminal &multi_terminals[get_active_pcb()->terminalid]

#endif
