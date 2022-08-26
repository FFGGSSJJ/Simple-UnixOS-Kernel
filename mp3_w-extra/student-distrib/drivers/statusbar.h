/**
 * @file statusbar.h
 * @author guanshujie fu
 * @brief 
 * @version 0.1
 * @date 2022-04-30
 * 
 * @copyright Copyright (c) 2022
 * @ref see vbe.h
 * 
 */
#ifndef STATUSBAR_H
#define STATUSBAR_H

#include "vbe.h"

/* status bar structure */
/* 0     10                        50              80
 * +-----+-------------------------+---------------+ 0
 * | Icon|  Message (Terminal #)   | Time HH:MM:SS |
 * +-------------------------------+---------------+ 1
 */
/* following is based on 25*80 */
#define STATUSBAR_ICON_START        0
#define STATUSBAR_ICON_END          10
#define STATUSBAR_INFO_START        10
#define STATUSBAR_INFO_END          70
#define STATUSBAR_TIME_START        70
#define STATUSBAR_TIME_END          80

/* following is based on 400*720 */
#define STATUSBAR_ICON_X            2
#define STATUSBAR_ICON_Y            2

void statusbar_init();
void transparent_sb();
void swtich_terminalinfo();
void message_update(char* message, uint32_t len, uint8_t param);
void icon_update(int32_t id);
void clock_update(char* time);




#endif
/*status bar*/
