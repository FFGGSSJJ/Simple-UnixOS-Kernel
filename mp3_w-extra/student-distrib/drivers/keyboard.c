/**
 * @file keyboard.c
 * @author: Xiaomin Qiu
 * @modifier: Guanshujie Fu
 * @description: functions for keyboard operation
 * @creat_date: 2022.5. - 
 *             2022.3.19 - move scancode_set and flags from header to here
 *             2022.3.20 - 
 */

#include "keyboard.h"




char scancode_set[NUM_MODE][NUM_SCAN] = {
/* mode 0: CAPS_FLAG =0 and SHIFT_FLAG =0 */  
    {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', 0, 0, 'a', 's', 'd', 'f', 'g', 'h', 
    'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, 0,0,' '
    },
/* mode 1: CAPS_FLAG = 0 and SHIFT_FLAG = 1 */
    {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', 0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{', '}', 0, 0, 'A', 'S', 'D', 'F', 'G', 'H', 
    'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' '
    },
/* mode 2: CAPS_FLAG = 1 and SHIFT_FLAG = 0*/
    {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', 0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '[', ']', 0, 0, 'A', 'S', 'D', 'F', 'G', 'H', 
    'J', 'K', 'L', ';', '\'', '`', 0, '\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', 0, 0, 0,' '
    },

/* mode 3: CAPS_FLAG = 1 and SHIFT_FLAG = 1 */
    {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '{', '}', 0, 0, 'a', 's', 'd', 'f', 'g', 'h', 
    'j', 'k', 'l', ':', '\"', '~', 0, '|', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', '<', '>', '?', 0, 0, 0, ' '
    }
    };
    
/* 
 * keyboard_init
 *  DESCRIPTION: initialize keyboard
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: enable keyboard IRQ 
 */
volatile unsigned char CAPS_FLAG = 0;
volatile unsigned char L_SHIFT_FLAG = 0;
volatile unsigned char R_SHIFT_FLAG = 0;
volatile unsigned char CTRL_FLAG = 0;
volatile unsigned char ALT_FLAG = 0;
volatile unsigned char SCAN_MODE = 0;


void keyboard_init(void){
    CAPS_FLAG = 0;
    L_SHIFT_FLAG = 0;
    R_SHIFT_FLAG = 0;
    CTRL_FLAG = 0;
    ALT_FLAG = 0;
    enable_irq(KB_IRQ);
}


/* keyboard_handler - handles the keyboard interrupt
 *                  output the input character
 *
 * Inputs: None
 * Outputs: None
 * Side Effects: puts char on screen
 */
void keyboard_handler(void){
    // cli();
    uint8_t scancode = inb(KB_DATA);     // get the scancode
    int32_t i;
    terminal_t* current_terminal = get_active_terminal();
    SCAN_MODE = (CAPS_FLAG<<1) | (R_SHIFT_FLAG | L_SHIFT_FLAG);

    switch(scancode) {
        
        case TAB:
            //tab is defined as one space here.
            if (current_terminal->buf_index < KB_BUF_SIZE - 1){
                current_terminal->buffer[current_terminal->buf_index] = ' ';
                current_terminal->buf_index++;
                printf("%c",' '); 
            }
            break;
        
        case CTRL_RELEASE:
            CTRL_FLAG = 0;
            break;
        case ALT_RELEASE:
            ALT_FLAG = 0;
            break;
        case L_SHIFT_RELEASE:
            L_SHIFT_FLAG = 0;
            break;
        case R_SHIFT_RELEASE:
            R_SHIFT_FLAG= 0;
            break;
        case CTRL_PRESS:
            CTRL_FLAG = 1 ;
            break;
        case ALT_PRESS: 
            ALT_FLAG = 1;
            break;
        case R_SHIFT_PRESS:
            R_SHIFT_FLAG = 1;
            key_pressed = R_SHIFT_PRESS;
            break;
        case L_SHIFT_PRESS:
            L_SHIFT_FLAG = 1;
            break;
        case CAPS_PRESS:
            CAPS_FLAG ^= 1;
            break;
        case BACKSPACE:
            if (current_terminal->TERMINAL_READ_FLAG) {
                if (current_terminal->buf_index == 0)
                    break;
                else
                    current_terminal->buf_index--;
            }
            key_pressed = BACKSPACE;
            backspace();
            break;

        case ENTER_PRESS:
            current_terminal->TERMINAL_READ_FLAG = 0;
            current_terminal->buffer[current_terminal->buf_index]='\n';
            current_terminal->buf_index = 0;
            //printf("%c", '\n');
            putk('\n');
            // record key
            key_pressed = ENTER_PRESS;
            break;
        case F1_PRESS:
            if(ALT_FLAG){
                terminal_switch(terminal_0);
            }
            break;
        case F2_PRESS:
            if(ALT_FLAG){
                terminal_switch(terminal_1);
            }
            break;
        case F3_PRESS:
            if(ALT_FLAG){
                terminal_switch(terminal_2);
            }
            break;

        default :
            if(CTRL_FLAG) {
                //clear the screen when ctrl+l pressed
                if(scancode_set[SCAN_MODE][scancode]=='l'||scancode_set[SCAN_MODE][scancode]=='L'){
                    clear();
                    current_terminal->cursor_x = 0;
                    current_terminal->cursor_y = 0;
                    update_cursor(0,0);
                    //printf(current_terminal.buffer);
                    for (i = 0 ; i < current_terminal->buf_index; i++){
                        putk(current_terminal->buffer[i]);
                    }
                }
                break;
            }else{  

                if(scancode_set[SCAN_MODE][scancode]==0 || (scancode > NUM_SCAN)) break;
                if(ALT_FLAG && scancode_set[SCAN_MODE][scancode] == '1'){
                    terminal_switch(terminal_0);
                    break;
                }
                if(ALT_FLAG && scancode_set[SCAN_MODE][scancode] == '2'){
                    terminal_switch(terminal_1);
                    break;
                }
                if(ALT_FLAG && scancode_set[SCAN_MODE][scancode] == '3'){
                    terminal_switch(terminal_2);
                    break;
                }
                if (current_terminal->buf_index < KB_BUF_SIZE-1){
                    current_terminal->buffer[current_terminal->buf_index] = scancode_set[SCAN_MODE][scancode];// pig
                    current_terminal->buf_index++;
                    putk(scancode_set[SCAN_MODE][scancode]);
                    // record key pressed
                    key_pressed = scancode_set[SCAN_MODE][scancode];
                }
            }
    }
    //  sti();   // end of critical section
    send_eoi(KB_IRQ);   // send EOI to activate interrupt
}
