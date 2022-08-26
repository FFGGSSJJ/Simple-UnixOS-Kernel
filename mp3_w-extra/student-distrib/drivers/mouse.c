#include "mouse.h"
// #include vga
// #include gui

// global variables
uint32_t mouse_enabled = 0;
// cursor data
int16_t cursor_x;
int16_t cursor_y;
int16_t prev_cursor_x;
int16_t prev_cursor_y;
// mouse button data
uint32_t left_pressed;
uint32_t right_pressed;
uint32_t middle_pressed;
// packet data
//static uint8_t prev_packet = 0;
uint8_t counter;

// function declaration
void write_to_port(uint8_t data, uint8_t port);
uint8_t read_from_mouse(void);
void set_cursor(int cursor_x, int cursor_y);


/* 
 * write_to_port
 *  DESCRIPTION: write one byte to mouse's port
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: none
 */
void write_to_port(uint8_t data, uint8_t port)
{
    // First check whether the second bit of port 0x64 is cleared
    // If not, wait
    wait_output_to_mouse();
    while((inb(CHECK_PORT) & BIT2MASK) != 0);
    // If cleared, write data to port 0x60 or 0x64
    outb(data,port);
}

/* 
 * read_from_mouse
 *  DESCRIPTION: read from mouse data port
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: the byte read from mouse data port
 *  SIDE EFFECTS: none
 */
uint8_t read_from_mouse(void)
{
    uint8_t retval;
    // First check whether the first bit of port 0x64 is set
    // If not, wait
    wait_input_to_mouse();
    while((inb(CHECK_PORT) & BIT1MASK) != 1);
    // If cleared, read data from port 0x60
    retval = inb(MOUSE_PORT);
    return retval;
}

/* 
 * set_cursor
 *  DESCRIPTION: set mouse cursor position
 *  INPUTS: cursor_x - position x
 *          cursor_y - position y
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: set mouse cursor
 */
void set_cursor(int cursor_x, int cursor_y)
{
    // get valid bits
    int x = (cursor_x << CURSOR_SHIFT) | X_MASK;
    int y = (cursor_y << CURSOR_SHIFT) | Y_MASK;

    // set cursor
    outw(x, CURSOR_PORT);
    outw(y, CURSOR_PORT);

}

/* 
 * mouse_init
 *  DESCRIPTION: Initialize mouse
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: 
 */
void mouse_init(void)
{
    // local variables
    uint8_t status;

    /* reset the mouse */
    // Before send anything, first sending 0xD4 to port 0x64
    write_to_port(PRE_COMMAND, CHECK_PORT);
    // write reset command to data port
    write_to_port(RESET, MOUSE_PORT);
    // wait for acknowledge
    while (read_from_mouse() != ACK);
    
    /* set compaq status byte */
    // get current compaq status byte
    // send command byte to port 0x64
    write_to_port(GET_STATUS, CHECK_PORT);
    // set status byte, which is the 2nd bit
    status = read_from_mouse() | BIT2MASK;
    // clear the command byte
    status = status & (~GET_STATUS);
    // set the status byte
    write_to_port(MOUSE_PORT, CHECK_PORT);
    write_to_port(status, MOUSE_PORT);

    /* set cursor */
    cursor_x = 0;
    cursor_y = 0;
    prev_cursor_x = 0;
    prev_cursor_y = 0;
    set_cursor(cursor_x, cursor_y);
    vbe_store(0, 0);

    /* set button press status */
    left_pressed = 0;
    right_pressed = 0;
    middle_pressed = 0;
    counter = 0;

    /* enable packet streaming */
    // The mouse starts sending automatic packets when the mouse moves or is clicked
    // Before send anything, first sending 0xD4 to port 0x64
    write_to_port(PRE_COMMAND, CHECK_PORT);
    // write reset command to data port
    write_to_port(ENABLE_PACKET, MOUSE_PORT);
    // wait for acknowledge
    while (read_from_mouse() != ACK);

    /* Connect to PIC */
    enable_irq(MOUSE_IRQ);
    mouse_enabled = 1;
    //printf("mouse init success\n");
    sti();
    return;
}

/* 
 * mouse_handler
 *  DESCRIPTION: handles interrupt
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: 
 */
void mouse_handler(void)
{
    //printf("mouse interrupt happens!\n");
    /* Overall check */
    // Check if mouse data port is available
    // bit 1 of port 0x64 will be 1 if ready to read
    if ((inb(CHECK_PORT)&BIT1MASK)==0){
        //printf("Mouse data port is not ready to read.\n");
        send_eoi(MOUSE_IRQ);
        return;
    }
    // Check id keyboard interrupt is happening
    // bit 5 of port 0x64 will be 0 if so, avoid crashing with kb
    if ((inb(CHECK_PORT)&BIT6MASK)==0){
        //printf("Keyboard interrupt happens.\n");
        send_eoi(MOUSE_IRQ);
        return;
    }

    /* Check behavior of mouse*/
    // read in the information packet
    uint8_t packet = inb(MOUSE_PORT);
    // check if something happens
    if (packet == ACK) {
        // nothing happens
        send_eoi(MOUSE_IRQ);
        return;
    } else {
        int32_t mouse_x;
        int32_t mouse_y;

        /* store the previous mouse coordinate */
        prev_cursor_x = cursor_x;
        prev_cursor_y = cursor_y;
        /* Check through the info packet*/
        // check if the alignment is wrong
        //if((packet & ALWAYS_ONE) != 1){
            //printf("Mouse packet is not aligned.\n");
            //send_eoi(MOUSE_IRQ);
            //return;
        //}
        // check if overflow happpens
        if((packet & X_OVERFLOW) || (packet & Y_OVERFLOW)){
            //printf("Mouse overflow.\n");
            send_eoi(MOUSE_IRQ);
            return;
        }
        // check button press conditions
        if (packet & LEFT_BUTTON) {
            //printf("Left button pressed!\n");
            left_pressed = 1;
        } else if ((packet & LEFT_BUTTON) == 0) {
            //printf("Left button released!\n");
            left_pressed = 0;
        }

        if (packet & RIGHT_BUTTON) {
            //printf("Right button pressed!\n");
            right_pressed = 1;
        } else if ((packet & RIGHT_BUTTON) == 0) {
            right_pressed = 0;
        }

        if (packet & MID_BUTTON) {
            //printf("Middle button pressed!\n");
            middle_pressed = 1;
        } else if ((packet & MID_BUTTON) == 0) {
            middle_pressed = 0;
        }
        // updata movements
        // get mouse movement data
        mouse_x = (int32_t)read_from_mouse(); //physical move of x
        mouse_y = (int32_t)read_from_mouse(); //physical move of y
        // update position
        if (packet & X_SIGN){
            mouse_x = mouse_x | LOW_BYTE_MASK;
        }
        if (packet & Y_SIGN){
            mouse_y = mouse_y | LOW_BYTE_MASK;
        }
        mouse_x /= 15; //make it smaller
        mouse_y /= 15; //make it smaller
        mouse_y = -mouse_y;
        // check boundary condition
        // x coordinate
        // printf("%d x_movement: ",mouse_x);
        // printf("; %d y_movement: ",mouse_y);
        if (cursor_x + mouse_x < 0) {
            cursor_x  = 0; // left boundary
        } else if (cursor_x + mouse_x > VGA_WIDTH - CURSOR_WIDTH - 1) {
            cursor_x = VGA_WIDTH - CURSOR_WIDTH - 1; // right boundary
        } else{
            cursor_x = cursor_x + mouse_x;
        }
        // y coordinate
        if (cursor_y + mouse_y < 0) {
            cursor_y  = 0; // upper boundary
        } else if (cursor_y + mouse_y > VGA_HEIGHT - CURSOR_HEIGHT - 1) {
            cursor_y = VGA_HEIGHT - CURSOR_HEIGHT - 1; // lower boundary
        } else{
            cursor_y = cursor_y + mouse_y;
        }
        // set new cursor place
        // printf("%d x_position: ",cursor_x);
        // printf("; %d y_position: ",cursor_y);
        // printf("\n");
        set_cursor(cursor_x, cursor_y);
        send_eoi(MOUSE_IRQ);
        // store package
        //prev_packet = packet;
        vbe_mouse_restore(prev_cursor_x, prev_cursor_y);
        vbe_mouse_update(cursor_x, cursor_y);
        vbe_mouse_click();
    } 
}



void wait_output_to_mouse()
{
    int timeout = 1000;
    /* All output to port 0x60 or 0x64 must 
    be preceded by waiting for bit 1 (value=2) 
    of port 0x64 to become clear. */
    while(timeout-- && (inb(CHECK_PORT) & 2) != 0) {}
}

void wait_input_to_mouse()
{
    int timeout = 1000;
    /* All output to port 0x60 or 0x64 must 
    be preceded by waiting for bit 1 (value=2) 
    of port 0x64 to become clear. */
    while(timeout-- && (inb(CHECK_PORT) & 1) != 0) {}
}

