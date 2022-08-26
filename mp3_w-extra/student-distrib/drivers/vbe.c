/**
 * @file vbe.c
 * @author guanshujie fu
 * @brief see vbe.h
 * @version 0.1
 * @date 2022-04-27
 * 
 * @copyright Copyright (c) 2022
 * @ref see vbe.h
 */

#include "vbe.h"

// Address of linear buffer, set by PCI scanner on startup
// INIT it as 0, which means do not start VGA. 
uint32_t qemu_vga_addr      = 0;
uint16_t qemu_vga_xres      = 0;
uint16_t qemu_vga_yres      = 0;
uint16_t qemu_vga_bpp       = 0;      
uint32_t qemu_vga_enabled   = 0;
uint32_t qemu_vga_cursor_x  = 0;
uint32_t qemu_vga_cursor_y  = 0;
/* show desktop picture or not */
int32_t show_picture        = 0;
int32_t booting             = 0;
/* uint16_t vbe_read(uint16_t index)
 * input: index - index of the register in QEMU VGA
 * output: ret val - data stored in that register
 * description: read from a QEMU VGA register.
 */
uint16_t vbe_read(uint16_t index) {
    outw(index, VBE_DISPI_IOPORT_INDEX);
    return inw(VBE_DISPI_IOPORT_DATA);
}


/* void vbe_write(uint16_t index, uint16_t data)
 * input: index - index of register in QEMU VGA
 *         data - data to be written into
 * output: data written into the register
 * description: write to a QEMU VGA register.
 */
void vbe_write(uint16_t index, uint16_t data) {
    outw(index, VBE_DISPI_IOPORT_INDEX);
    outw(data, VBE_DISPI_IOPORT_DATA);
}


/* void vbe_init(uint16_t res_x, uint16_t res_y, uint16_t bpp)
 * input:   res_x - 
 *          res_y - 
 *          bpp - 
 * output: data written into the register
 * description: write to a QEMU VGA register.
 */
void vbe_init(uint16_t res_x, uint16_t res_y, uint16_t bpp)
{
    /* check availability */
    if (qemu_vga_addr == 0)  return;
    if (res_x > VBE_DISPI_MAX_XRES || res_y > VBE_DISPI_MAX_YRES) return;
    if (bpp != VBE_DISPI_BPP_32 && bpp != VBE_DISPI_BPP_16) return;

    qemu_vga_enabled = VBE_DISPI_DISABLED;
    /* remember to disbale vbe before modifying vbe status */
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    vbe_write(VBE_DISPI_INDEX_XRES, res_x);
    vbe_write(VBE_DISPI_INDEX_YRES, res_y);
    vbe_write(VBE_DISPI_INDEX_BPP, bpp);
    vbe_write(VBE_DISPI_INDEX_X_OFFSET, 0);
    vbe_write(VBE_DISPI_INDEX_Y_OFFSET, 0);
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED);

    /* set global varibale */
    qemu_vga_xres = res_x;
    qemu_vga_yres = res_y;
    qemu_vga_bpp  = bpp;
    qemu_vga_enabled = 1;
    booting = 0;
}

/*
 * vbe linear frame buffer structure in our design
 * +------------+
 * | Terminal 0 |
 * +------------+
 * | Terminal 1 |
 * +------------+
 * | Terminal 2 |
 * +------------+
 * | Desktop  3 |
 * +------------+
 * | Desktop  4 |
 * +------------+
 * | Desktop  X |
 * +------------+
 * 
 */

/**
 * @brief find the start address in lfb for the desktop picture
 * 
 * @return uint32_t as described
 */
uint32_t current_picture_addr()
{
    if (qemu_vga_addr == 0) return 0;
    if (show_picture)   return qemu_vga_addr + TERMINAL_NUM * (qemu_vga_xres * qemu_vga_yres * qemu_vga_bpp/BITS_PER_BYTE);
    else                return qemu_vga_addr + current_active_termid * (qemu_vga_xres * qemu_vga_yres * qemu_vga_bpp/BITS_PER_BYTE);
}


uint32_t current_running_addr()
{
    if (qemu_vga_addr == 0) return 0;
    return qemu_vga_addr + get_active_pcb()->terminalid * (qemu_vga_xres * qemu_vga_yres * qemu_vga_bpp/BITS_PER_BYTE);
}


/**
 * @brief set the displaying screen to selected window
 * 
 * @param id - 0~2 for terminal and 3 for desktop
 * @return int32_t fail or VBESUCCESS
 */
int32_t vbe_displaying_set(int id)
{
    if (!qemu_vga_enabled)  return VBEFAIL;
    if (id > TERMINAL_NUM)  return VBEFAIL;

    if (id < 3) show_picture = 0;
    vbe_write(VBE_DISPI_INDEX_Y_OFFSET, id * qemu_vga_yres);
    return VBESUCCESS;
}




/**
 * @brief set color for a single pixel
 * 
 * @param x     - x coordinate of pixel
 * @param y     - y coordinate of pixel
 * @param color - color
 * @return int32_t VBESUCCESS/VBEFAIL
 */
int32_t vbe_onepixel_color_set(uint32_t x, uint32_t y, uint32_t display_vidmem, vga_color_t *color)
{
    /* check availability */
    if (!qemu_vga_enabled)  return VBEFAIL;
    if (x < 0 || x >= VBE_DISPI_MAX_XRES || y < 0 || y > VBE_DISPI_MAX_YRES)    return VBEFAIL;
    if (color == NULL)  return VBEFAIL;

    /* get pixel address in memory */
    uint32_t addr = display_vidmem + (y * qemu_vga_xres + x) * qemu_vga_bpp/BITS_PER_BYTE;
    /* set color into the memory */
    *((uint32_t*)addr) = color->val & RGB32_MASK;

    return VBESUCCESS;
}


/**
 * @brief set color for a row
 * 
 * @param y 
 * @param color 
 * @return int32_t VBESUCCESS/VBEFAIL
 */
int32_t vbe_onerow_color_set(uint32_t y, uint32_t display_vidmem, vga_color_t *color)
{
    /* check availability */
    if (!qemu_vga_enabled)  return VBEFAIL;
    if (y < 0 || y > VBE_DISPI_MAX_YRES)    return VBEFAIL;
    if (color == NULL)  return VBEFAIL;

    /* set color */
    uint32_t x;
    for (x = 0; x < qemu_vga_xres; x++)
        vbe_onepixel_color_set(x, y, display_vidmem, color);
    return VBESUCCESS;
}


/**
 * @brief get pixel color from current displaying vbe screen
 * 
 * @param scr_x 
 * @param scr_y 
 * @return uint32_t 
 */
uint32_t vbe_get_pixel(uint16_t scr_x, uint16_t scr_y)
{
    if(!qemu_vga_enabled) return VBEFAIL;
    if (scr_x >= qemu_vga_xres || scr_y >= qemu_vga_yres)   return VBEFAIL;

    uint32_t position = current_picture_addr() + (scr_y * qemu_vga_xres + scr_x) * qemu_vga_bpp/BITS_PER_BYTE;
    return *((uint32_t*)position);
}


/**
 * @brief put character into current active memory
 * 
 * @param scr_x 
 * @param scr_y 
 * @param c 
 * @param fontcolor 
 * @param backbolor 
 */
void vbe_putc(uint16_t scr_x, uint16_t scr_y, uint8_t c, vga_color_t* fontcolor, vga_color_t* backbolor)
{
    /* check availability */
    if (!qemu_vga_enabled)  return;
    int w, h;

    for (h = 0; h < FONT_DATA_HEIGHT; h++) {
        for (w = 0; w < FONT_DATA_WIDTH; w++) {
            uint8_t mask = 1<<(7 - w);
            if(font_data[c][h] & mask) 
                vbe_onepixel_color_set(scr_x*FONT_ACTUAL_WIDTH + w, scr_y*FONT_DATA_HEIGHT + h, current_running_addr(), fontcolor);
            else 
                vbe_onepixel_color_set(scr_x*FONT_ACTUAL_WIDTH + w, scr_y*FONT_DATA_HEIGHT + h, current_running_addr(), backbolor);
        }
        /* the actual width of a char is 9 as there is one pixel gap between characters */
        for (w = FONT_DATA_WIDTH; w < FONT_ACTUAL_WIDTH; w++) 
            vbe_onepixel_color_set(scr_x*FONT_ACTUAL_WIDTH + w, scr_y*FONT_DATA_HEIGHT + h, current_running_addr(), backbolor);
    }
    return;
}







/**
 * @brief put character into current displaying memory
 * 
 * @param scr_x - screen x (up-left corner of the character)
 * @param scr_y - screen y
 * @param c     - char
 * @param fontcolor 
 * @param backcolor
 */
void vbe_putk(uint16_t scr_x, uint16_t scr_y, uint8_t c, vga_color_t* fontcolor, vga_color_t* backbolor)
{
    /* check availability */
    if (!qemu_vga_enabled)  return;
    int w, h;

    for (h = 0; h < FONT_DATA_HEIGHT; h++) {
        for (w = 0; w < FONT_DATA_WIDTH; w++) {
            uint8_t mask = 1<<(7 - w);
            if(font_data[c][h] & mask) 
                vbe_onepixel_color_set(scr_x*FONT_ACTUAL_WIDTH + w, scr_y*FONT_DATA_HEIGHT + h, current_picture_addr(), fontcolor);
            else 
                vbe_onepixel_color_set(scr_x*FONT_ACTUAL_WIDTH + w, scr_y*FONT_DATA_HEIGHT + h, current_picture_addr(), backbolor);
        }
        /* the actual width of a char is 9 as there is one pixel gap between characters */
        for (w = FONT_DATA_WIDTH; w < FONT_ACTUAL_WIDTH; w++) 
            vbe_onepixel_color_set(scr_x*FONT_ACTUAL_WIDTH + w, scr_y*FONT_DATA_HEIGHT + h, current_picture_addr(), backbolor);
    }
    return;
}


/**
 * @brief it transfers the text video memory into the display video memory
 * 
 * @param text_vidmem 
 * @param display_vidmem 
 * @param fontcolor 
 * @param backcolor 
 */
void vbe_transfer(uint8_t* text_vidmem, uint32_t display_vidmem, vga_color_t* fontcolor, vga_color_t* backcolor)
{
    /* check availability */
    if (!qemu_vga_enabled || show_picture)  return;

    int scr_x, scr_y;
    for (scr_y = 1; scr_y < SCREEN_ROW; scr_y++) {
        for (scr_x = 0; scr_x < SCREEN_COL; scr_x++) {
            uint8_t c = *(uint8_t *)(text_vidmem + ((NUM_COLS * scr_y + scr_x) << 1));
            int w, h;
            for (h = 0; h < FONT_DATA_HEIGHT; h++) {
                for (w = 0; w < FONT_DATA_WIDTH; w++) {
                    uint8_t mask = 1<<(7 - w);
                    if(font_data[c][h] & mask) 
                        vbe_onepixel_color_set(scr_x*FONT_ACTUAL_WIDTH + w, scr_y*FONT_DATA_HEIGHT + h, display_vidmem, fontcolor);
                    else 
                        vbe_onepixel_color_set(scr_x*FONT_ACTUAL_WIDTH + w, scr_y*FONT_DATA_HEIGHT + h, display_vidmem, backcolor);
                }
                /* the actual width of a char is 9 as there is one pixel gap between characters */
                for (w = FONT_DATA_WIDTH; w < FONT_ACTUAL_WIDTH; w++) 
                    vbe_onepixel_color_set(scr_x*FONT_ACTUAL_WIDTH + w, scr_y*FONT_DATA_HEIGHT + h, display_vidmem, backcolor);
            }
        }
    } return;
}

/**
 * @brief roll up the vbe screen
 * Note: The first row is used as a status bar and hence will not be moved.
 */
void vbe_rollup(int display)
{
    cli();
    /* check availability */
    if (!qemu_vga_enabled)  return;

    uint32_t offset = FONT_DATA_HEIGHT * qemu_vga_xres * qemu_vga_bpp / BITS_PER_BYTE;
    uint32_t length = (SCREEN_ROW - 2) * offset;
    uint32_t addr = display == 1 ? current_picture_addr() : current_running_addr();

    memcpy((char*)(addr+offset), (char*)(addr+offset*2), length);
    memset((char*)addr+(SCREEN_ROW-1)*offset, 0, offset);
    sti();
}


/**
 * @brief self explained
 * 
 */
void vbe_mouse_init()
{
    vbe_mouse_update(0, 0);
}


/**
 * @brief restore the current background color
 * 
 * @param mouse_x 
 * @param mouse_y 
 */
void vbe_store(uint32_t mouse_x, uint32_t  mouse_y)
{
    /* check avail */
    if (!qemu_vga_enabled || !mouse_enabled)  return;

    uint32_t i, j;
    for (i = 0; i < CURSOR_HEIGHT; i++) {
        for (j = 0; j < CURSOR_WIDTH; j++) {
            background_buf[i*CURSOR_WIDTH + j] = vbe_get_pixel(mouse_x + j, mouse_y + i);
        }
    }
}

/**
 * @brief update the mouse in the screen
 * 
 * @param mouse_x 
 * @param mouse_y 
 */
void vbe_mouse_update(uint32_t mouse_x, uint32_t  mouse_y)
{
    /* check avail */
    if (!qemu_vga_enabled || !mouse_enabled)  return;
    uint32_t i, j;
    vbe_store(mouse_x, mouse_y);
    for (i = 0; i < CURSOR_HEIGHT; i++) {
        for (j = 0; j < CURSOR_WIDTH; j++) {
            vga_color_t mousecolor;
            mousecolor.val = mouse_data[i*CURSOR_WIDTH + j];
            if ((mousecolor.val != 0xFFFFFF) && ((mousecolor.val & 0xFF00FF) == 0xFF00FF))
                mousecolor.val = vbe_get_pixel(mouse_x + j, mouse_y + i);
            vbe_onepixel_color_set(mouse_x + j, mouse_y + i, current_picture_addr(), &mousecolor);
        }
    }
}


/**
 * @brief restore the previous background color
 * 
 * @param mouse_x 
 * @param mouse_y 
 */
void vbe_mouse_restore(uint32_t mouse_x, uint32_t  mouse_y)
{
    /* check avail */
    if (!qemu_vga_enabled || !mouse_enabled)  return;
    uint32_t i, j;
    for (i = 0; i < CURSOR_HEIGHT; i++) {
        for (j = 0; j < CURSOR_WIDTH; j++) {
            vga_color_t backcolor;
            backcolor.val = background_buf[i*CURSOR_WIDTH + j];
            vbe_onepixel_color_set(mouse_x + j, mouse_y + i, current_picture_addr(), &backcolor);
        }
    }

}



void vbe_mouse_click()
{
    /* check avail */
    if (!qemu_vga_enabled || !mouse_enabled)  return;

    if (left_pressed) {
        if (cursor_x < 0 || cursor_x > STATUSBAR_ICON_X + ICON_WIDTH + 3 || 
            cursor_y < 0 || cursor_y > STATUSBAR_ICON_Y + ICON_HEIGHT + 3)   
            return;
        else if (current_active_termid == TERM_NUM) {
            terminal_switch(0);
        } else {
            show_desktop(DESKTOP_IMAGE_HEIGHT, DESKTOP_IMAGE_WIDTH, (uint8_t*)DESKTOP_IMAGE_DATA);
        }
    } return;
}





/**
 * @brief show desktop picture in the vbe screen
 * 
 * @param height 
 * @param width 
 * @param data 
 */
void show_desktop(uint32_t height, uint32_t width, uint8_t* data)
{
    if (!qemu_vga_enabled)  return;
    if (height > qemu_vga_yres || width > qemu_vga_xres)    return;
    if (data == NULL)   return;

    show_picture = 1;
    /* copy img to the vbe memory row by row, byte by byte*/
    int row;
    int rowlen = qemu_vga_xres * qemu_vga_bpp / BITS_PER_BYTE;
    for (row = FONT_DATA_HEIGHT; row < height; row++) {
        memcpy((char*)(current_picture_addr() + row*rowlen), 
               (char*)(data + row*width*qemu_vga_bpp/BITS_PER_BYTE), 
               width*qemu_vga_bpp/BITS_PER_BYTE);
    }
    char word[] = "DESKTOP";
    uint32_t len = strlen(word);
    message_update(word, len, 1);
    icon_update(3);
    vbe_displaying_set(3);
    current_active_termid = 3;

}



/**
 * @brief self explained
 * 
 */
void boot_animation()
{
    if (!qemu_vga_enabled)  return;

    /* indicates that the system is booting */
    booting = 1;
    vbe_displaying_set(3);

    /* get the imgs of boot animation */
    uint32_t boot_img_addr = qemu_vga_addr + 3 * (qemu_vga_xres * qemu_vga_yres * qemu_vga_bpp/BITS_PER_BYTE);
    uint16_t i,j;
    vga_color_t bootcolor;

    /* set animation */
    while (animation_check == -1);
    while (animation_check <= 10) {
        if (animation_check%2 == 0) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot1[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = BLACKS;
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        }
    }

    /* 10 - 20 (5s - 10s)*/
    while (animation_check <= 20) {
        if (animation_check%10 == 2) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot2[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%10 == 3) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot3[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%10 == 4) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot4[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%10 == 5) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot5[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%10 == 6) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot6[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%10 == 7) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot7[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%10 == 8) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot8[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%10 == 9) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot9[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%10 == 0) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot10[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        }
    }

    /* 20 - 30 (10s - 15s)*/
    while (animation_check <= 30) {
        if (animation_check%20 == 1) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot11[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%20 == 2) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot12[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%20 == 3) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot13[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%20 == 4) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot14[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%20 == 5) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot15[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%20 == 6) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot16[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%20 == 7) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot17[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        } else if (animation_check%20 == 8) {
            for (i = 0; i < BOOT_HEIGHT; i++) {
                for (j = 0; j < BOOT_WIDTH; j++) {
                    bootcolor.val = boot18[i*BOOT_WIDTH + j];
                    vbe_onepixel_color_set(BOOT_X + j, BOOT_Y + i, boot_img_addr, &bootcolor);
                }
            }
        }
    }
    return;
}









