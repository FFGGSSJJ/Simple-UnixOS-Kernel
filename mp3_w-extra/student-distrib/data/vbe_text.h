/**
 * @file vbe_char.h
 * @author guanshuejie fu
 * @brief 
 * @version 0.1
 * @date 2022-04-29
 * 
 * @copyright Copyright (c) 2022
 * @ref ece391/mp2/text.h(c)
 * 
 */
#ifndef VBE_TEXT_H
#define VBE_TEXT_H

#define FONT_DATA_WIDTH     8
#define FONT_ACTUAL_WIDTH   9
#define FONT_DATA_HEIGHT    16

/* 9*80 = 720; 16*25 = 400 */
#define SCREEN_COL          80
#define SCREEN_ROW          25

extern unsigned char font_data[256][16];

#endif
