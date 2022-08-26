/**
 * @file: asm_linkage.h
 * 
 * @ahthor: Guanshujie Fu
 * @description: headers for asm linkages used in idt
 * @creat_date: 2022.5.18 - add headers
 */
#ifndef ASM_LINKAGE_H
#define ASM_LINKAGE_H
#ifndef ASM


#include "../drivers/rtc.h"
#include "../drivers/keyboard.h"
#include "../drivers/pit.h"
#include "system_call.h"


// linkages for interrupt handlers
extern void rtc_handler_linkage();
extern void keyboard_handler_linkage();
extern void pit_handler_linkage();

// linkages for system call
extern void system_call_linkage();

#endif
#endif
