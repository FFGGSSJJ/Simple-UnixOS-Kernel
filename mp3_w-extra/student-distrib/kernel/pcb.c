/**
 * @file: pcb.c
 * 
 * @ahthor: Jiahao Wei, Guanshujie Fu, Zitai Kong
 * @modifier: 
 * @description: functions for pcb driver
 * @creat_date: 2022.3.28
 */

#include "pcb.h"

// free pcbs map
int8_t pcbs_map = 0x00;

/**
 * @brief Create a pcb object
 * @return -1 -- cannot create more process
 *         0~7 -- process id
 * side effect: might change the active_pcb pointer
 */
int32_t create_pcb(void)
{
    int i;
    pcb_t *pcb_addr;
    for (i = 0; i < process_num_max; i++)
    {
        if (0 != (pcbs_map & (0x1 << i)))
            continue;

        /* set up pcb struct */
        pcb_addr = (pcb_t *)(bottom - block_size * (i + 1)); // static pcb addr
        pcb_addr->pid = i; // assign process id
        pcb_addr->execute_esp = bottom - block_size * i;
        pcb_addr->sched_esp = 0x0;
        pcb_addr->signal = 0;
        memset(pcb_addr->args, '\0', args_size);

        if (NULL == scheduled_process[running_process_index]) // no parent process in current terminal
            pcb_addr->terminalid = running_process_index;
        else
            pcb_addr->terminalid = current_active_termid;

        if (NULL == scheduled_process[running_process_index]) // no parent process in current terminal
            pcb_addr->parent_pcb = NULL;
        else // exist parent process
            //pcb_addr->parent_pcb = scheduled_process[current_active_termid];
            pcb_addr->parent_pcb = get_active_pcb();

        /* initialize the whole file_array */
        int j;
        for (j = 0; j < file_array_len; j++) {
            pcb_addr->file_array[j].fops_ptr = NULL;
            pcb_addr->file_array[j].type = -1;
            pcb_addr->file_array[j].inode_num = -1;
            pcb_addr->file_array[j].flags = 0;
            pcb_addr->file_array[j].position = 0;
        }
        // update pcbs_map
        pcbs_map = pcbs_map | (0x1 << i);
        
        return i;
    }
    return -1;
}


/**
 * @brief remove a pcb object
 * @return 0
 * side effect: might change the active_pcb pointer
 */
int32_t remove_pcb(void)
{
    /* update pcbs map */
    pcbs_map = pcbs_map & ~(0x1 << get_active_pcb()->pid);
    return 0;
}
