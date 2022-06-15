/**
 * @file schedule.c
 * @brief 
 * @version 0.1
 * @date 2022-04-22
 */

#include "schedule.h"
 
#define block_size 0x2000 // 8KB
#define bottom 0x800000   // 8MB

 /* 
 *  DESCRIPTION: initialize the active processes' pcb list
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: change scheduled_process 
 */
void schedule_init(void)
{
	int i;
	//clear the contant
	for(i = 0; i < ACTIVE_SIZE; i++) {
        scheduled_process[ACTIVE_SIZE] = NULL;
    }
    running_process_index = 0;
	return;
}

 /* 
 *  DESCRIPTION: initialize the active processes' pcb list
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: change scheduled_process 
 */
void schedule_handler(void)
{
    /* find PCB of the next process */
    running_process_index = (running_process_index + 1) % ACTIVE_SIZE;
    pcb_t* next_pcb = scheduled_process[running_process_index];

    /* if next terminal has no shell, create one */
    if (NULL == next_pcb)
    {
        execute((uint8_t*)"shell");
        // this call to execute will never return, the following code will not reached
    }

    /* change program memory mapping */
    map_vir_to_phy_4M(program_mem, bottom + program_size * (next_pcb->pid));
    update_usr_vidmem(next_pcb->terminalid);

    /* change rtc rate */
    // set_rate((process_terminal)->rtc_rate);

    /* modify TSS */
    tss.esp0 = bottom - block_size * (next_pcb->pid);

    return;
}

/**
 * @brief 
 * 
 * @param esp
 * @return -1 for failure
 *         0  for success 
 */
int32_t store_current_shched_esp(uint32_t esp)
{
    pcb_t* current_pcb = scheduled_process[running_process_index];
    if (NULL == current_pcb) return -1;
    current_pcb->sched_esp = esp;
    return 0;
}

uint32_t get_next_shched_esp(void)
{
    /* since running_process_index has changed in schedule_handler */
    /* from the perspective of scheduler, running_process_index is the next scheduled process index */
    pcb_t* current_pcb = scheduled_process[running_process_index];
    return current_pcb->sched_esp;
}
