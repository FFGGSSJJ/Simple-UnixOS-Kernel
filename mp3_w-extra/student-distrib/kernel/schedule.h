/**
 * @file schedule.h
 * @brief 
 * @version 0.1
 * @date 2022-04-22
 */

#ifndef _SCHEDULE_H
#define _SCHEDULE_H

#include "../types.h"
#include "../lib.h"
#include "pcb.h"
#include "paging.h"
#include "../x86_desc.h"
#include "system_call.h"
#include "../drivers/terminal.h"

/* Externally-visible functions */
extern void scheduler(void);
/* Helper function: initialize active pcb list*/
void schedule_init(void);

#endif
