/**
 * @file keyboard.c
 * @author: Xiaomin Qiu
 * @modifier: Guanshujie Fu
 * @description: functions for keyboard operation
 * @creat_date: 2022.3. - 
 *             2022.3.19 - move scancode_set and flags from header to here
 *             2022.3.20 - 
 *             2022.4.26 - add tab fill function
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
            tabpressed();
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
        case UP_PRESS:
            search_history(UP);
            break;
        case DOWN_PRESS:
            search_history(DOWN);
            break;
        default :
            if(CTRL_FLAG) {
                //clear the screen when ctrl+l pressed
                if(scancode_set[SCAN_MODE][scancode]=='l'||scancode_set[SCAN_MODE][scancode]=='L'){
                    clear();
                    clear_history();
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

//https://github.com/RicciZ/ECE391
/* tabpressed - print a space if the buf is empty
 *              complete the file name
 *
 * Inputs: None
 * Outputs: None
 * Side Effects: change the buffer and print file name or space
 */
void tabpressed(){
    terminal_t* current_terminal = get_active_terminal();
    if (current_terminal->buf_index == 0 || current_terminal->buffer[current_terminal->buf_index-1] == ' ')
    {
        if (current_terminal->buf_index < KB_BUF_SIZE - 1){
            current_terminal->buffer[current_terminal->buf_index] = ' ';
            current_terminal->buf_index++;
            putk(' '); 
        }
        return;
    }
    else{
        // store the pre-part of the file name
        uint8_t pre[BUFFER_SIZE];
        uint8_t temp[BUFFER_SIZE];

        int i = current_terminal->buf_index-1;
        int j = 0;
        while (i >= 0 && current_terminal->buffer[i] != ' ') temp[j++] = current_terminal->buffer[i--];
        j--;
        i = 0;
        while (j >= 0) pre[i++] = temp[j--];
        pre[i] = '\0';
        int match_len = i;

        extern boot_blk_t* boot_blk_ptr;
        dentry_t den;
        int match = 0;
        int match_count = 0;
        int match_i = 0;

        // compare with each file name
        for (i = 0; i < boot_blk_ptr->dir_count; i++){
            // store file name in temp
            if (read_dentry_by_index(i, &den)) return;
            strncpy((int8_t*)temp, den.filename, filename_len_max);
            ((char*)temp)[filename_len_max] = '\0';
            int32_t len = (int32_t)strlen((int8_t*)temp);
            ((char*)temp)[len] = '\0';

            int match_flag = 1;
            for (j = 0; pre[j] != '\0'; j++){
                if (pre[j] != temp[j]){
                    match_flag = 0;
                    break;
                }
            }
            if (match_flag){
                match = 1;
                match_i = i;
                match_count++;
            }
            if (match_count > 1) return;
        }

        // if match, print it
        if (match){
            read_dentry_by_index(match_i, &den);
            strncpy((int8_t*)temp, den.filename, filename_len_max);
            ((char*)temp)[filename_len_max] = '\0';
            int32_t len = (int32_t)strlen((int8_t*)temp);
            ((char*)temp)[len] = '\0';
            for (i = 0, j = 0; temp[j] != '\0'; i++,j++){
                if (i < match_len) continue;
                current_terminal->buffer[current_terminal->buf_index] = temp[j];
                putk(temp[j]);
                current_terminal->buf_index++;
            }
        }
    }    

}
