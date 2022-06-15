/**
 * @file: terminal.c
 * 
 * @ahthor: Xiaomin Qiu, Zitai Kong
 * @modifier: 
 * @description: functions for terminal driver
 * @creat_date: 2022.3.
 */

#include "terminal.h"

/*
 * terminal_open
 *  DESCRIPTION: open the terminal and do initializations
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: 0 if success, -1 if fail
 *  SIDE EFFECTS: open the terminal file
 */
int32_t terminal_open(void)
{
    int i;
    terminal_t* current_terminal = get_active_terminal();

    // mark the terminal is on
    if (current_terminal->terminal_active == 1)
    {
        //printf("There exists active terminals!\n");
        return -1;
    }
    else
    {
        current_terminal->terminal_active = 1;
    }

    // update cursor
    current_terminal->cursor_x = 0;
    current_terminal->cursor_y = 0;
    update_cursor(0,0);

    // clear screen
    clear();

    // clear input buffer
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        current_terminal->buffer[i] = EMPTY;
    }

    return 0;
}

/*
 * terminal_close
 *  DESCRIPTION: close the terminal
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: -1 if success
 *  SIDE EFFECTS: close the terminal file
 */
int32_t terminal_close(void)
{
    terminal_t* current_terminal = get_active_terminal();
    int i;

    // mark the terminal is off
    current_terminal->terminal_active = 0;

    // clear screen
    //clear();

    // clear input buffer
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        current_terminal->buffer[i] = EMPTY;
    }

    return -1;
}

/*
 * terminal_read
 *   DESCRIPTION: read from the terminal. Keystroke will be stored into keyboard
 *                buffer and echo on the screen. Reading terminates when caller
 *                presses ENTER. the maximum # of bytes read is 128 and the last
 *                byte is '\n'. Caller needs to allocate space for buf
 *   INPUTS: buf: destination of character reading. Must be at least nbytes large
 *        nbytes: # of bytes read from terminal
 *   OUTPUTS: echoes for keystroke
 *   RETURN VALUE: # of elements read (including '\n')
 *   SIDE EFFECTS: arg buf is filled. Keyboard buffer is cleared.
 */
int32_t terminal_read(uint32_t inode_num, int32_t position, char *buf, int32_t nbytes)
{
    cli();
    terminal_t* current_terminal = &multi_terminals[get_active_pcb()->terminalid];
    if (buf == NULL || nbytes < 0)
        return -1;
    
    /* signal interrupt handler to store char into buffer */
    current_terminal->TERMINAL_READ_FLAG = 1;

    //int32_t bytes_num = (nbytes < (int32_t)current_terminal.buf_index) ? nbytes : (int32_t)current_terminal.buf_index;
    /* clear keyboard buffer */
    current_terminal->buf_index = 0;
    /* wait until interrupt handler clear TERMINAL_READ_FLAG
     * i.e. ENTER is pressed by user
     */
    sti();

    while (current_terminal->TERMINAL_READ_FLAG);

    int32_t i;
    int32_t bytes_num = 0;
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        if (current_terminal->buffer[i] == '\n' || current_terminal->buffer[i] == '\r')
        {
            current_terminal->terminal_active = 1;
            //if (bytes_num > i)
            if(i < BUFFER_SIZE)
                bytes_num = i + 1;
            
            bytes_num = (nbytes < bytes_num) ? nbytes : bytes_num;
            /* copy from keyboard buffer to caller's buffer*/
            (void)strncpy((int8_t *)buf, (int8_t *)current_terminal->buffer, bytes_num);
            memset((void *)current_terminal->buffer, 0, (uint32_t)BUFFER_SIZE);
            break;
        }
        bytes_num = bytes_num + 1;
    }

    return bytes_num;
}

/*
 * terminal_write
 *   DESCRIPTION: write to the terminal from arg buf
 *   INPUTS: buf: soruce of character writing. at least nbytes large
 *        nbytes: num of bytes written to terminal
 *   OUTPUTS: characters displayed on terminal
 *   RETURN VALUE: # of bytes on success
 *   SIDE EFFECTS: none
 */
int32_t terminal_write(int32_t fd, char *buf, int32_t nbytes)
{
    /* failure: invalid buf */
    if (buf == NULL || nbytes < 0)
    {
        return -1;
    }
    cli();
    /* iterate over char in buf and put it on screen */
    int32_t idx;
    for (idx = 0; idx < nbytes; idx++)
    {
        if (!buf[idx])
            continue;
        // display
        putc(buf[idx]);
    }
    //printf("\n");
    sti();
    return nbytes;
}

/*
 *  terminal_switch(int32_t terminal_id);
 *    DESCRIPTION: switch the terminal
 *   INPUTS: terminal_id: switch to terminals[terminal_id]
 *   OUTPUTS: change the running_terminal to terminals[terminal_id]
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none

 */
void terminal_switch(int32_t terminal_id){
    cli();
    if (current_active_termid == terminal_id ){
        sti();
        return;
    }
    terminal_t* current_terminal = get_active_terminal();

    /* the order of code for following part is tricky. Read carefully before modifying */
    /* make sure current displaying terminal's vidmap is correct. i.e. 0xB8000 -> 0xB8000, buffer -> buffer */
    update_usr_vidmem(current_active_termid);
    /* store current video into the buffer */
    memcpy((void *)current_terminal->screen_buffer, (void *) PHYSICAL_VMEM_BEGIN, VID_SIZE);
    /* restore next terminal's screen buffer into video memory */
    memcpy((void *) PHYSICAL_VMEM_BEGIN, (void *)multi_terminals[terminal_id].screen_buffer, VID_SIZE);

    /* update the switched video */
    current_active_termid = terminal_id;
    update_usr_vidmem(terminal_id);
    current_terminal = get_active_terminal();

    /* update cursor */
    update_cursor(current_terminal->cursor_x, current_terminal->cursor_y);

    /* IMPORTANT */
    /* this function is needed to make the video display correct */
    /* I don't know why. I spent 6 hours for the display bug. It is fixed. */
    fresh_video();

    sti();
}


/*
 * int32_t get_active_terminal()
 *   DESCRIPTION: return currently active terminal
 *   INPUTS: none
 *   OUTPUTS:
 *   RETURN VALUE: the ptr to current terminal
 *   SIDE EFFECTS: none

 */
terminal_t* get_active_terminal()
{
    return &multi_terminals[current_active_termid];
}


/*
 * int32_t terminal_init()
 *   DESCRIPTION: initialize terminal
 *   INPUTS: none
 *   OUTPUTS:
 *   RETURN VALUE:0
 *   SIDE EFFECTS: none

 */
int32_t terminal_init()
{
    // initialize terminals
    int i;
    for (i = 0; i < TERMINAL_NUM; i++) {
        multi_terminals[i].tid = i;
        multi_terminals[i].TERMINAL_READ_FLAG = 1;
        multi_terminals[i].terminal_active = 0;
        multi_terminals[i].buf_index = 0;
        multi_terminals[i].cursor_x = 0;
        multi_terminals[i].cursor_y = 0;
        multi_terminals[i].put_mode = 1;
        multi_terminals[i].screen_buffer = (uint32_t*) (TERM_VID_BEGIN + VID_SIZE*i);
    }
    current_active_termid = 0;
    return 0;
}
