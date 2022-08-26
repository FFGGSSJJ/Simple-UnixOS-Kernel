#include "../lib.h"
#include "../types.h"
#include "pit.h"
#include "i8259.h"
// Add more if necessary


int pit_counter = 0;
int animation_rate = 0;
int animation_check = -1;
int second_counter = 0;
int minnute_couter = 0;
int second = 0;
int minnute = 0;

/* 
 * pic_init
 *  DESCRIPTION: Initialize pit timer chip
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: enable pic IRQ and set interrupt rate to 10ms
 */
void pit_init(void)
{
    /* set the flag
     * since cli_and_save needs long type, use 32 bits */
    // uint32_t flag;

    /* create the critical section */
	// cli_and_save(flag);     // disable interrupts

    // Set Mode/Command Register
    outb(PITREAD_BACK_STATUS,MODE_REG);

    // Set interrupt period to 10ms
    uint16_t count = PITDEFAULT_RATE/TEN_MS;
    outb((count&HIGH_MASK),PIT_DATA_PORT);    // low bytes
    outb(((count&LOW_MASK)>>HIGH_SHIFT),PIT_DATA_PORT);   // high bytes

    // Connect to PIC
    enable_irq(PIT_IRQ);
    /* end the critical section */
	// restore_flags(flag);
	// sti();
}


/* 
 * pit_handler
 *  DESCRIPTION: handles interrupt and execute test_interrupt handler
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: execute schedule handler
 */
void pit_handler(void)
{
    // Send EOI
    send_eoi(PIT_IRQ);
    // Guarantee interrupts can happens
    // cli();
    // Change the next line to schedule handler later...
    //printf("PIT interrupt received!\n");
    char time[] = "00:00:00";
    if (++second_counter == SECOND_RATE) {
        if (++second == MINUTE) {
            second = 0;
            minnute++;
        }
        second_counter = 0;
    }
    time[7] = '0' + second%10;
    time[6] = '0' + second/10;
    time[4] = '0' + minnute%10;
    time[3] = '0' + minnute/10;
    clock_update(time);
    if (show_picture) {
        vbe_displaying_set(3);
        icon_update(3);
        pit_counter = 0;
    }
    if (show_picture == 0 && ++pit_counter == FRESH_COUNTER) {
        vga_color_t font, back;
        font.val = TERMINAL_FONT_COLOR;
        back.val = TERMINAL_BACKGROUND_COLOR;
        terminal_t* current_terminal = get_active_terminal();
        vbe_transfer((uint8_t*)current_terminal->screen_buffer, current_picture_addr(), &font, &back);
        icon_update(current_active_termid);
        //vbe_mouse_update(cursor_x, cursor_y);
        pit_counter = 0;
    }
    if (booting && ++animation_rate == ANIMA_RATE) {
        animation_check++;
        animation_rate = 0;
    }


    scheduler();
    // sti();
}


