/**
 * @file statusbar.c
 * @author guanshujie fu
 * @brief 
 * @version 0.1
 * @date 2022-04-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "statusbar.h"
uint32_t transparent_sb_color[FONT_DATA_HEIGHT*DESKTOP_IMAGE_WIDTH];
/**
 * @brief 
 * 
 */
void statusbar_init()
{
    /* check availability */
    if(!qemu_vga_enabled)   return;
    /* set message in status bar */
    int i;
    vga_color_t fontcolor;
    fontcolor.val = STATUS_BAR_FONTCOLOR;
    vga_color_t backcolor;
    backcolor.val = STATUS_BAR_BACKCOLOR;
    char init[] = "Our OS System !";
    uint32_t len = strlen(init);
    for (i = 0; i < STATUSBAR_TIME_END; i++)
        vbe_putk(i, 0, ' ', &backcolor, &backcolor);
    for (i = 0; i < len && i < STATUSBAR_INFO_END; i++) 
        vbe_putk(i+STATUSBAR_INFO_START, 0, init[i], &fontcolor, &backcolor);
}


/**
 * @brief 
 * 
 */
void transparent_sb()
{
    if (!qemu_vga_enabled)  return;
    int i,j;
    for (i = 0; i < FONT_DATA_HEIGHT; i++) {
        for (j = 0; j < DESKTOP_IMAGE_WIDTH; j++) {
            transparent_sb_color[i*DESKTOP_IMAGE_WIDTH + j] = (0xFFFFFF + DESKTOP_IMAGE_DATA[i*DESKTOP_IMAGE_WIDTH + j]);
        }
    } return;
}





/**
 * @brief 
 * 
 */
void swtich_terminalinfo()
{
    /* check availability */
    if(!qemu_vga_enabled)   return;
    if(current_active_termid > TERM_NUM)    return;

    char info[] = "Current Terminal : #0";
    uint32_t len = strlen(info);
    info[len-1] = '0' + current_active_termid;

    message_update(info, len, 0);
    return;
}


/**
 * @brief 
 * 
 * @param message 
 * @param len 
 * @param param 
 */
void message_update(char* message, uint32_t len, uint8_t param)
{
    /* check availability */
    if(!qemu_vga_enabled)   return;
    if(message == NULL) return;

    /* set message in status bar */
    int i;
    vga_color_t fontcolor;
    fontcolor.val = STATUS_BAR_FONTCOLOR;
    vga_color_t backcolor;
    backcolor.val = STATUS_BAR_BACKCOLOR;
    for (i = 0; i < len && i < STATUSBAR_INFO_END; i++) 
        vbe_putk(i+STATUSBAR_INFO_START, 0, message[i], &fontcolor, &backcolor);
    for (i = len; i < STATUSBAR_INFO_END; i++)
        vbe_putk(i+STATUSBAR_INFO_START, 0, ' ', &backcolor, &backcolor);
}


/**
 * @brief update the icon part of status bar
 *          for desktop (id = 3), it will be terminal icons
 *          for terminals (id 0 - 2), it will be a close icon
 * @param id 
 */
void icon_update(int32_t id)
{
    /* check availability */
    if(!qemu_vga_enabled)   return;
    if(id > TERM_NUM) return;

    /* set icon pictures for desktop*/
    uint32_t i, j;
    vga_color_t color;
    if (id == TERM_NUM) {
        /* set background */
        for (i = 0; i < STATUSBAR_ICON_END; i++) { 
            color.val = STATUS_BAR_BACKCOLOR;
            vbe_putk(i, 0, ' ', &color, &color);
        }
        /* set icon */
        for (i = 0; i < ICON_HEIGHT; i++) {
            for (j = 0; j < ICON_WIDTH; j++) {
                color.val = terminal_icon[i*ICON_WIDTH + j];
                vbe_onepixel_color_set(STATUSBAR_ICON_X + j, STATUSBAR_ICON_Y + i, current_picture_addr(), &color);
            }
        }
    } else {
        /* set background */
        for (i = 0; i < STATUSBAR_ICON_END; i++) { 
            color.val = STATUS_BAR_BACKCOLOR;
            vbe_putk(i, 0, ' ', &color, &color);
        }
        /* set icon */
        for (i = 0; i < ICON_HEIGHT; i++) {
            for (j = 0; j < ICON_WIDTH; j++) {
                color.val = close_icon[i*ICON_WIDTH + j];
                if (color.val == 0xFFFFFF)  color.val = STATUS_BAR_BACKCOLOR;
                vbe_onepixel_color_set(STATUSBAR_ICON_X + j, STATUSBAR_ICON_Y + i, current_picture_addr(), &color);
            }
        }
    }
}



/**
 * @brief 
 * 
 * @param time 
 */
void clock_update(char* time)
{
    /* check availability */
    if(!qemu_vga_enabled)   return;
    if(time == NULL) return;

    /* set message in status bar */
    int i;
    uint16_t len = strlen(time);
    vga_color_t fontcolor;
    fontcolor.val = STATUS_BAR_FONTCOLOR;
    vga_color_t backcolor;
    backcolor.val = STATUS_BAR_BACKCOLOR;
    for (i = 0; i < len && i < STATUSBAR_TIME_END; i++) 
        vbe_putk(i+STATUSBAR_TIME_START, 0, time[i], &fontcolor, &backcolor);
    // for (i = len; i < STATUSBAR_TIME_END; i++)
    //     vbe_putk(i+STATUSBAR_TIME_START, 0, ' ', &backcolor, &backcolor);
}

