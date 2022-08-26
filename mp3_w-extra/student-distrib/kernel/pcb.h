/**
 * @file: pcb.c
 * 
 * @ahthor: Jiahao Wei, Guanshujie Fu
 * @modifier: 
 * @description: struct and functions for pcb
 * @creat_date: 2022.3.27
 */

#ifndef _PCB_H_
#define _PCB_H_

#include "../types.h"
#include "../lib.h"
#include "../drivers/filesystem.h"
#include "../drivers/terminal.h"
#include "../drivers/rtc.h"

/* constants */
#define process_num_max 8
#define file_array_len 8
#define args_size 128
#define block_size 0x2000 // 8KB
#define bottom 0x800000 // 8MB

// kzt's definitions
#define MIN_FD 0
#define MAX_FD 8 
#define READ_INDEX  0
#define WRITE_INDEX 1
#define OPEN_INDEX  2
#define CLOSE_INDEX 3
#define STDIN_INDEX 0
#define STDOUT_INDEX 1

// Index in file descriptor operations
#define OPEN  0
#define CLOSE 1
#define READ  2
#define WRITE 3

// for scheduler
#define ACTIVE_SIZE 3

typedef int32_t(*func_ptr)();

typedef struct file_array_entry
{
    func_ptr*   fops_ptr;
    int32_t     type;
    int32_t     inode_num;
    int32_t     position;
    int32_t     flags;
} file_array_entry_t;


typedef struct pcb pcb_t;
struct pcb
{
    int32_t             pid; // since the mapping is static, pid is used to decide the mapping
    int32_t             terminalid;
    pcb_t               *parent_pcb;
    uint32_t            execute_esp; // used in execute
    uint32_t            sched_esp; // used in scheduler
    int32_t             signal;
    uint8_t             args[args_size];
    file_array_entry_t  file_array[file_array_len];
};

// The list contains the current active processes' pcb ptr
pcb_t *scheduled_process[ACTIVE_SIZE];
// index of current running process in scheduled_process array
uint8_t running_process_index;

#define get_pcb(pid) (pcb_t *)(bottom - block_size * (pid + 1));

extern inline pcb_t* get_active_pcb(void);

int32_t create_pcb(void);

int32_t remove_pcb(void);

#endif
