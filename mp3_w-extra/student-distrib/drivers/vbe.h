/**
 * @file vbe.h
 * @author guanshujie fu
 * @modifier
 * @brief   The VBE standard defines a set of functions related to the operation of the video card, 
 *          and specifies how they can be invoked. These functions were implemented by the video card manufacturers, 
 *          usually in a ROM in the video card. Originally, the interface to these functions used only INT 0x10, 
 *          i.e. the interrupt number used by the BIOS' video services, and had to be executed with the CPU in real-mode. 
 *          Starting with version 2.0, VBE specifies also direct call of some time-critical functions from protected mode. 
 *          Version 3.0 specified a new interface for protected mode for most functions, but their implementation is optional: 
 *          i.e. an implementation can claim conformance with VBE 3.0, even though it does not support the protected mode interface.
 * @version 0.1
 * @date 2022-04-27
 * 
 * @copyright Copyright (c) 2022
 * 
 * @ref https://wiki.osdev.org/Bochs_VBE_Extensions
 *      https://github.com/qemu/vgabios
 *      https://web.fe.up.pt/~pfs/aulas/lcom2020/labs/lab5/lab5_3.html
 *      https://github.com/xddxdd/uiuc-ece391-mp3
 *      offcial doc: http://www.phatcode.net/res/221/files/vbe20.pdf
 * 
 * 
 */
#ifndef VBE_H
#define VBE_H

#include "../lib.h"
#include "../types.h"
#include "../data/vbe_text.h"
#include "../kernel/pcb.h"
#include "terminal.h"
#include "mouse.h"
#include "pit.h"
#include "../data/color.h"
#include "../data/desktop.h"
#include "../data/icons.h"
#include "../data/boot_imgs.h"

/*
 * VBE    Vesa Bios Extension
 * DISPI  (Bochs) Display Interface
 * BPP    Bits Per Pixel
 * LFB    Linear Frame Buffer
*/
#define VBE_DISPI_BANK_ADDRESS          0xA0000
#define VBE_DISPI_BANK_SIZE_KB          64

#define VBE_DISPI_MAX_XRES              1024
#define VBE_DISPI_MAX_YRES              768
#define VBE_DISPI_BPP_16                16
#define VBE_DISPI_BPP_32                32
/* i don't know the exact size */
#define QEMU_VGA_DEFAULT_WIDTH          720
#define QEMU_VGA_DEFAULT_HEIGHT         400

/* 
 * To read a register, 
 * first write the index value to VBE_DISPI_IOPORT_INDEX (0x01CE), 
 * then read the 16-bit value from VBE_DISPI_IOPORT_DATA (0x01CF) 
 */
#define VBE_DISPI_IOPORT_INDEX          0x01CE
#define VBE_DISPI_IOPORT_DATA           0x01CF

/* normal index
 *  VBE_DISPI_INDEX_ID:
 *      This parameter can be used to detect the current display API (both bochs & vbebios).
 *      i.e. VBE_DISPI_ID0 - VBE_DISPI_ID4 (see below)
 *
 *  VBE_DISPI_INDEX_VIRT_WIDTH:
 *      This parameter can be used to read/write the current virtual width.
 *  VBE_DISPI_INDEX_VIRT_HEIGHT:    
 *      This parameter can be read in order to obtain the current virtual height.
 *      This setting will be adjusted after setting a virtual width in order to stay within limit of video memory.
 */
#define VBE_DISPI_INDEX_ID              0x0         
#define VBE_DISPI_INDEX_XRES            0x1
#define VBE_DISPI_INDEX_YRES            0x2
#define VBE_DISPI_INDEX_BPP             0x3
#define VBE_DISPI_INDEX_ENABLE          0x4
#define VBE_DISPI_INDEX_BANK            0x5
#define VBE_DISPI_INDEX_VIRT_WIDTH      0x6
#define VBE_DISPI_INDEX_VIRT_HEIGHT     0x7


/* offset index 
 *  VBE_DISPI_INDEX_X_OFFSET:
 *      The current X offset (in pixels!) of the visible screen part.
 *  VBE_DISPI_INDEX_Y_OFFSET:
 *      The current Y offset (in pixels!) of the visible screen part.
 *  Writing a new offset will also result in a complete screen refresh.
 *  These are the start point of the displaying screen
 * */
#define VBE_DISPI_INDEX_X_OFFSET        0x8
#define VBE_DISPI_INDEX_Y_OFFSET        0x9

/* vbe interface 
 * see document in ref_2/vbe_display_api.txt/API History for detail
 */
#define VBE_DISPI_ID0                   0xB0C0
#define VBE_DISPI_ID1                   0xB0C1
#define VBE_DISPI_ID2                   0xB0C2
#define VBE_DISPI_ID3                   0xB0C3
#define VBE_DISPI_ID4                   0xB0C4

#define VBE_DISPI_DISABLED              0x00
#define VBE_DISPI_ENABLED               0x01
#define VBE_DISPI_VBE_ENABLED           0x40
#define VBE_DISPI_NOCLEARMEM            0x80

#define RGB32_MASK                      0xFFFFFF
#define BITS_PER_BYTE                   8
#define VBESUCCESS                      1
#define VBEFAIL                         -1
#define DESKTOP1                        3
#define DESKTOP2                        4



/* LFB address - 4-byte addressable according to our design (bpp32)
 * The initialization is done in PCI
 * When using a linear framebuffer, the BGA exposes all of the video memory in a single linearly addressable section of memory. 
 * The address of the framebuffer is not fixed, and must be read from the first PCI base address register (BAR 0 of device 0x1234:0x1111).
 * To enable the linear framebuffer, use the VBE_DISPI_LFB_ENABLED flag (0x40) when enabling the BGA in conjunction with the VBE_DISPI_ENABLED flag.  
 */
extern uint32_t qemu_vga_addr;
/* after pci initialization, qemu_vga_addr should be 0xFD000000 */

extern uint32_t vbe_buffer_addr;
extern int32_t show_picture;
extern int32_t booting;


/* Status information of QEMU VGA 
 * bpp: bits per pixel, or bit depth
 * res: display resolution, the number of distinct pixels in each dimension that can be displayed
 *      unit in pixel: width * height
 * first three status are set when boot and will not be changed
 */
extern uint16_t qemu_vga_xres;
extern uint16_t qemu_vga_yres;
extern uint16_t qemu_vga_bpp;      
extern uint32_t qemu_vga_enabled;
extern uint32_t qemu_vga_cursor_x;
extern uint32_t qemu_vga_cursor_y;


/* color struct for pixel 
 * only 32 is supported in our design 
 * see ref0/Memory layout for detail
 */
typedef union {
    uint32_t val;
    struct __attribute__ ((packed)) {
        uint8_t r16         : 5;
        uint8_t g16         : 6;
        uint8_t b16         : 5;
        uint16_t dummy16    : 16;
    };
    struct __attribute__ ((packed)) {
        uint8_t r32;
        uint8_t g32;
        uint8_t b32;
        uint8_t dummy32;    // ignored
    };
} vga_color_t;

/* a buffer used to store the background of the mouse */
uint32_t background_buf[ICON_HEIGHT*ICON_WIDTH];

/* vbe functions */
uint16_t vbe_read(uint16_t index);
void vbe_write(uint16_t index, uint16_t data);
void vbe_init(uint16_t res_x, uint16_t res_y, uint16_t bpp);

/**/
int32_t vbe_onepixel_color_set(uint32_t x, uint32_t y, uint32_t display_vidmem, vga_color_t *color);
int32_t vbe_onerow_color_set(uint32_t y, uint32_t display_vidmem, vga_color_t *color);
void vbe_display_test();
int32_t vbe_displaying_set(int id);
int32_t vbe_bufferring_set(int id);
uint32_t current_picture_addr();
uint32_t current_running_addr();


/* functions for transferring from text mode to display mode */
uint32_t vbe_get_pixel(uint16_t scr_x, uint16_t scr_y);
void vbe_putc(uint16_t scr_x, uint16_t scr_y, uint8_t c, vga_color_t* fontcolor, vga_color_t* backbolor);
void vbe_putk(uint16_t scr_x, uint16_t scr_y, uint8_t c, vga_color_t* fontcolor, vga_color_t* backbolor);
void vbe_transfer(uint8_t* text_vidmem, uint32_t display_vidmem, vga_color_t* fontcolor, vga_color_t* backbolor);
void vbe_rollup(int display);

/* functions to draw mouse */
void vbe_mouse_init();
void vbe_store(uint32_t mouse_x, uint32_t  mouse_y);
void vbe_mouse_update(uint32_t mouse_x, uint32_t  mouse_y);
void vbe_mouse_restore(uint32_t mouse_x, uint32_t  mouse_y);
void vbe_mouse_click();
/* */
void boot_animation();
void show_desktop(uint32_t height, uint32_t width, uint8_t* data);

/* VBE_H */
#endif
