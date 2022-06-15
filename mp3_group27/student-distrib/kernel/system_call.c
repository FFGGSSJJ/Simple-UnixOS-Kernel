/**
 * @file system_call.c
 * @brief 
 * @version 0.1
 * @date 2022-04-06
 */

#include "system_call.h"

#define magic_len 4
#define entry_info_location 24
#define entry_info_len 4
#define prog_offset 0x00048000
#define block_size 0x2000 // 8KB
#define bottom 0x800000   // 8MB
#define user_prog_esp program_mem + kernel_mem
#define buf_size 256

// self defined RETURN val
#define PCBFULL -2
#define HALTFAIL -3

extern inline int32_t transit_to_user(uint32_t user_esp, uint32_t user_eip);
extern inline void jump_to_execute_ret(uint32_t kernel_esp, uint8_t status);

int8_t magic_num[magic_len] = {0x7f, 0x45, 0x4c, 0x46};

func_ptr terminal_operations[4] = {terminal_open, terminal_close, terminal_read, terminal_write};
func_ptr rtc_operations[4]      = {rtc_open, rtc_close, rtc_read, rtc_write};
func_ptr file_operations[4]     = {file_open, file_close, file_read, file_write};
func_ptr dir_operations[4]      = {dir_open, dir_close, dir_read, dir_write};

/**
 * @brief: attempts to load and execute a new program,
 *        handing off the processor to the new program until it terminates
 * @param: command
 * @return: -1    -- the command cannot be executed,
 *                  the program does not exist,
 *                  or the filename specified is not an executable
 *         256   -- the program dies by an exception
 *         0~255 -- the program executes a halt system call,
 *                  in which case the value returned is that given by the program’s call to halt
 */
int32_t execute(const uint8_t* command)
{
    /* Parse command */
    uint8_t buf[buf_size];
    uint8_t *str_ptrs[buf_size] = {NULL};
    uint8_t* scan = buf;
    int32_t str_num = 0; // number of strings
    strncpy((int8_t*)buf, (int8_t*)command, buf_size);
    // modify buf, make str_ptrs[i] point to different strings
    while (1)
    {
        while (' ' == *scan) scan++;
        str_ptrs[str_num++] = scan;
        while (' ' != *scan && '\0' != *scan) scan++;
        if (' ' == *scan)
            *scan++ = '\0';
        else
            break;
    }
    if (str_num == 1) str_ptrs[1] = scan;  // make str_ptrs[1] point to "\0"

    /* Check for executable */
    dentry_t temp_dentry;
    // fname is not valid or file does not exist
    if (-1 == read_dentry_by_name(str_ptrs[0], &temp_dentry))
        return -1;
    // file is not a regular file
    if (REG_TYPE != temp_dentry.filetype)
        return -1;
    // file is not executable
    int8_t first_4B[magic_len];
    read_data(temp_dentry.inode_num, 0, (uint8_t*)first_4B, magic_len);
    if (0 != strncmp(first_4B, magic_num, magic_len))
        return -1;

    /* Create PCB */
    int8_t pid;
    if (-1 == (pid = create_pcb())) // cannot create more process
        return -1;
    pcb_t *child_pcb = get_pcb(pid);
    cli();
    // modify scheduled_process
    scheduled_process[running_process_index] = child_pcb;

    // store args for getargs
    strcpy((int8_t*)child_pcb->args, (int8_t*)(str_ptrs[1]));
    // setup stdin and stdout
    child_pcb->file_array[0].fops_ptr = terminal_operations;
    child_pcb->file_array[0].flags = 1;
    child_pcb->file_array[1].fops_ptr = terminal_operations;
    child_pcb->file_array[1].flags = 1;
    // open terminal
    (*(child_pcb->file_array[0].fops_ptr[OPEN]))();

    /* Set up user program memory (paging) */
    map_vir_to_phy_4M(program_mem, bottom + program_size * (child_pcb->pid));

    /* Load file into memory (must do this after setting up user paging) */
    read_data(temp_dentry.inode_num, 0, (uint8_t *)(program_mem + prog_offset), GET_FILE_SIZE((&temp_dentry)));

    /* Prepare for Context Switch (modify TSS) */
    tss.esp0 = bottom - block_size * (child_pcb->pid);

    // find program entry point (the virtual address of the first instruction)
    uint32_t prog_entry;
    read_data(temp_dentry.inode_num, entry_info_location, (uint8_t *)(&prog_entry), entry_info_len);

    /* transfer to user program, when user program finishes, get exit status */
    int32_t status;
    /* store current process kernel esp into current (active) pcb, push IRET context to current process' kernel stack, and use IRET to switch to user */
    // sti will be called in transit_to_user
    status = transit_to_user(user_prog_esp, prog_entry);
    return status;
}

/**
 * @brief: store execute_esp in function execute into pcb (of process call execute), helper function of transit_to_user
 * @param kernel_esp kernel esp in function execute
 */
void store_execute_esp(uint32_t kernel_esp)
{
    // assumption: in the first call to execute in entry(), the esp in close to the bottom 0x800000
    pcb_t *active_pcb_ptr = get_active_pcb();
    active_pcb_ptr->execute_esp = kernel_esp;
}

/**
 * @brief terminates current process, restore any parent data, restore parent paging,
 * return the exit status to its parent process
 * @param status the exit status of current process
 * @return 0 should never be returned
 * @warning This call should never return to the caller.
 */
int32_t halt(uint16_t status)
{
    pcb_t *active_pcb_ptr = get_active_pcb();
    if (NULL == active_pcb_ptr)
        return -1;

    /* close all files */
    int i;
    for (i = 0; i < file_array_len; i++)
    {
        if (0 != active_pcb_ptr->file_array[i].flags)
            (*(active_pcb_ptr->file_array[i].fops_ptr[CLOSE]))();
    }

    /* remove pcb */
    pcb_t *parent_pcb = active_pcb_ptr->parent_pcb;
    // modify scheduled_process
    cli();
    scheduled_process[running_process_index] = parent_pcb; // does the order matter?
    remove_pcb();
    if (NULL == parent_pcb) // no process remains
    {
        execute((uint8_t*)"shell"); // restart shell
    }

    /* restore parent data */
    /* Prepare for Context Switch (modify TSS) */
    // if there is no problem, kernel esp should be at the bottom of the block after return to user
    tss.esp0 = bottom - block_size * (parent_pcb->pid);

    /* always unmap the user video memory, might cause problems */
    //unmap_usr_vidmem(VIRTUAL_VMEM_BEGIN);
    /* restore parent paging */
    map_vir_to_phy_4M(program_mem, bottom + program_size * (parent_pcb->pid));

    /* jump to execute_ret in transit_to_user */
    jump_to_execute_ret(parent_pcb->execute_esp, status);

    /* This call should never reach this line. */
    return 0;
}


/* 
 *  int32_t open (const uint8_t* filename)
 *  DESCRIPTION: open the file, store into file descriptor
 *  INPUTS:     filename -- the name of file
 *  OUTPUTS:    none
 *  RETURN VALUE: index of file descriptor for success, -1 for failure
 */
int32_t open (const uint8_t* filename){

    //check input validity
    if (filename == NULL || *filename == '\0')    return -1;
    pcb_t *active_pcb_ptr = get_active_pcb();
    //check whether available file descriptor space
    int32_t i;      // i will be the fd of the current file
    for (i = 2; i < file_array_len; i++)
        if (0 == active_pcb_ptr->file_array[i].flags)
            break;
    if (file_array_len == i)    return -1;
    
    //check whether the named file exist
    uint32_t file_type;
    dentry_t temp_dentry;
    if(-1 == read_dentry_by_name(filename,&temp_dentry))
        return -1;
    
    //initialize the file descriptor
    file_type = temp_dentry.filetype;
    active_pcb_ptr->file_array[i].flags = 1;
    active_pcb_ptr->file_array[i].inode_num = temp_dentry.inode_num;
    active_pcb_ptr->file_array[i].position = 0;

    switch (file_type)
    {
    case RTC_TYPE:
        active_pcb_ptr->file_array[i].fops_ptr = rtc_operations;
        active_pcb_ptr->file_array[i].type = RTC_TYPE;
        break;
    case DIR_TYPE:
        active_pcb_ptr->file_array[i].fops_ptr = dir_operations;
        active_pcb_ptr->file_array[i].type = DIR_TYPE;
        break;
    case REG_TYPE:
        active_pcb_ptr->file_array[i].fops_ptr = file_operations;
        active_pcb_ptr->file_array[i].type = REG_TYPE;
        break;
    default:
        active_pcb_ptr->file_array[i].flags = 0;
        active_pcb_ptr->file_array[i].type = -1;
        return -1;
    }

    //check whether open success
    if( -1 == (*(active_pcb_ptr->file_array[i].fops_ptr[OPEN]))() )
        return -1;

    return i;
}



/* 
 *  int32_t close (int32_t fd)
 *  DESCRIPTION: close the file, clear corresponding file descriptor
 *  INPUTS:     fd -- the index of file descriptor
 *  OUTPUTS:    none
 *  RETURN VALUE: 0 for success, -1 for failure
 */
int32_t close (int32_t fd){
    //check input validity, 0,1 for default descriptor which can't be closed
    if(fd < 2 || fd >= file_array_len){
        return -1;
    }
    pcb_t *active_pcb_ptr = get_active_pcb();
    //the file is closed before
    if(0 == active_pcb_ptr->file_array[fd].flags){
        return -1;
    }

    active_pcb_ptr->file_array[fd].position = 0;
    active_pcb_ptr->file_array[fd].inode_num = -1;
    active_pcb_ptr->file_array[fd].flags = 0;
    return (*(active_pcb_ptr->file_array[fd].fops_ptr[CLOSE]))();
}

/* 
 *  syscall_read
 *  DESCRIPTION: read from the passed in fd (rtc,KB,filesys,terminal)
 *               call the corresponding read function and return number
 *               of bytes read
 *  INPUTS:     fd -- the index of file descriptor
 *              buf - the buffer to read to
 *              nbytes - number of bytes to read
 *  OUTPUTS:    none
 *  RETURN VALUE: number of bytes read for success, -1 for failure
 *  SIDE EFFECT: none
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes)
{
    pcb_t* pcb;
    int32_t retval;

    // get current pcb
    pcb = get_active_pcb();

    //check input validity
    // fd should one of 0-7 and not be stdout(1)
    if (fd < MIN_FD || fd >= MAX_FD || fd == STDOUT_INDEX) {
        return -1;
    }
    // buf should be valid and nbytes should be non-negative
    if (buf == NULL || nbytes < 0) {
        return -1;
    }
    // check if current fd is enabled
    if (pcb->file_array[fd].flags == 0) {
        return -1;
    }

    // Execute corresponding read operation
    int32_t position = pcb->file_array[fd].position;
    uint32_t inode_id = pcb->file_array[fd].inode_num;
    retval = (*(pcb->file_array[fd].fops_ptr[READ]))(inode_id, position, buf, nbytes);

    /* The position update need to be considered more */
    if (pcb->file_array[fd].type == DIR_TYPE)
        pcb->file_array[fd].position++;
    else
        pcb->file_array[fd].position += nbytes;
    // what if retval = -1?
    return retval;
}

/* 
 *  syscall_write
 *  DESCRIPTION: write data to the passed in fd (rtc,KB,filesys,terminal)
 *               call the corresponding write function and return number
 *               of bytes write
 *  INPUTS:     fd -- the index of file descriptor
 *              buf - the buffer to write to
 *              nbytes - number of bytes to write
 *  OUTPUTS:    none
 *  RETURN VALUE: number of bytes read for success, -1 for failure
 *  SIDE EFFECT: none
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes)
{
    pcb_t* pcb;
    int32_t retval;

    // get current pcb
    pcb = get_active_pcb();

    //check input validity
    // fd should one of 0-7 and not be stdin(0)
    if (fd < MIN_FD || fd >= MAX_FD) {
        return -1;
    }
    if (fd == STDIN_INDEX) {
        return -1;
    }
    // buf should be valid
    if (buf == NULL) {
        return -1;
    }
    // nbytes should be non-negative
    if (nbytes < 0) {
        return -1;
    }

    // check if current fd is enabled
    if (pcb->file_array[fd].flags == 0) {
        return -1;
    }

    // Execute corresponding read operation
    retval = (*(pcb->file_array[fd].fops_ptr[WRITE]))(fd,buf,nbytes);
    // what if retval = -1?
    return retval;
}


/* 
 *  getargs
 *  DESCRIPTION: reads the program’s command line arguments into a user-level buffer. 
 *  INPUTS:    
 *              buf - the buffer to write to
 *              nbytes - number of bytes to write
 *  OUTPUTS:    none
 *  RETURN VALUE: 0 for success, -1 for failure
 *  SIDE EFFECT: none
 */
int32_t getargs(uint8_t* buf, int32_t nbytes)
{
    // check input
    if (NULL == buf) return -1;

    pcb_t* active_pcb_ptr = get_active_pcb();
    // no arguments
    if ('\0' == *(active_pcb_ptr->args)) return -1;

    // check if the arguments and a terminal NULL (0-byte) fit in the user buffer
    int32_t len = 1 + strlen((int8_t*)(active_pcb_ptr->args));
    if (len > nbytes)
        return -1;

    // copy args from pcb's field into user buffer
    strcpy((int8_t*)buf, (int8_t*)(active_pcb_ptr->args));
    return 0;
}


/**
 * @brief the system call video memory function for user
 *        this function maps a virtual address to the video memory
 * 
 * @param screen_start 
 * @return int32_t: 0 for success and -1 for failed
 */
int32_t vidmap(uint8_t** screen_start)
{
    /* sanity check */
    if (screen_start == NULL || 
        (uint32_t) screen_start <= PROGRAM_IMG_BEGIN ||
        (uint32_t) screen_start >= PRPGRAM_IMG_END )
        return -1;
    
    /* set up the virtual and physical memory */
    uint32_t* phy_vmem = (uint32_t*) PHYSICAL_VMEM_BEGIN;
    /* i don't know why it is 8-bit long*/
    uint8_t* vir_vmem = (uint8_t*) VIRTUAL_VMEM_BEGIN;
    *screen_start = vir_vmem;

    /* set up user video memory mapping */
    set_usr_vidmem(vir_vmem, (uint32_t) phy_vmem);
    return 0;
    
}


/**
 * @brief Set the handler object
 * 
 * @param signum 
 * @param handler_address 
 * @return int32_t 
 */
int32_t set_handler(int32_t signum, void* handler_address)
{
    return 0;
}


/**
 * @brief 
 * 
 * @return int32_t 
 */
int32_t sigreturn(void)
{
    return 0;
}
