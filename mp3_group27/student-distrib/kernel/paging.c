/**
 * @file paging.c
 * @brief Definitions of paging-related functions
 * @version 0.1
 * @date 2022-03-17
 */

#include "paging.h"

#define VIDEO               0xB8000
#define ENTRY_NUM           1024
#define dir_field_len       10
#define dir_field           0xffc00000
#define table_field_len     10
#define table_field         0x003ff000
#define offset_field_len    12
#define offset_field        0x00000fff

/** set control registers (CR0, CR4) to enable paging */
extern inline void enable_paging(void);

/** load CR3 (PDBR) */
extern inline void load_CR3(uint32_t base);

/**
 * brief: initialize kernel page directory and page tabel in memory
 * input: none
 * output: set entries in page directory and page table
 * return: none
 * side effects: none
 */
void paging_init_kernel(void) {
    int i;
    // defines page directory entry for video memory
    // it shoud be of index 0 in page directory in order to map 0xB8000 virtual to 0xB8000 physical
    {
        page_directory_entry_t pde_4K_0_4M;
        pde_4K_0_4M.KByte.present               = 0x1;
        pde_4K_0_4M.KByte.read_or_write         = 0x1;
        pde_4K_0_4M.KByte.user_or_supervisor    = 0x0;
        pde_4K_0_4M.KByte.write_through         = 0x0;
        pde_4K_0_4M.KByte.cache_disabled        = 0x0;
        pde_4K_0_4M.KByte.accessed              = 0x0;
        pde_4K_0_4M.KByte.reserved              = 0x0;
        pde_4K_0_4M.KByte.page_size             = 0x0;
        pde_4K_0_4M.KByte.global_page           = 0x0;
        pde_4K_0_4M.KByte.avail                 = 0x0;
        pde_4K_0_4M.KByte.base_address          = ((uint32_t)kernel_page_table_0_4M) >> 12;  // base address of kernel_page_table_0_4M, use right shift to get the highest 20 bits

        kernel_page_dir[0] = pde_4K_0_4M;
    }

    // defines page directory entry for kernel
    // it shoud be of index 1 in page directory in order to map 4MB virtual to 4MB physical
    {
        page_directory_entry_t global_kernel_page;
        global_kernel_page.MByte.present                 = 0x1;
        global_kernel_page.MByte.read_or_write           = 0x1;
        global_kernel_page.MByte.user_or_supervisor      = 0x0;
        global_kernel_page.MByte.write_through           = 0x0;
        global_kernel_page.MByte.cache_disabled          = 0x0;
        global_kernel_page.MByte.accessed                = 0x0;
        global_kernel_page.MByte.dirty                   = 0x0;
        global_kernel_page.MByte.page_size               = 0x1;
        global_kernel_page.MByte.global_page             = 0x1;
        global_kernel_page.MByte.avail                   = 0x0;
        global_kernel_page.MByte.pat                     = 0x0;
        global_kernel_page.MByte.reserved                = 0x0;
        global_kernel_page.MByte.base_address            = kernel_mem >> (table_field_len + offset_field_len);  // map 4MB virtual to 4MB physical

        kernel_page_dir[1] = global_kernel_page;
    }

    // set the remaing entry as not present
    {
        page_directory_entry_t not_present_pde;
        not_present_pde.val = 0x0;
        for (i = 2; i < ENTRY_NUM; i++) {
            kernel_page_dir[i] = not_present_pde;
        }
    }

    // initialize the kernel_page_table_0_4M
    {
        page_table_entry_t not_present_pte;
        not_present_pte.val = 0x0;
        page_table_entry_t video_memory_pte;
        video_memory_pte.KByte.present                = 0x1;
        video_memory_pte.KByte.read_or_write          = 0x1;
        video_memory_pte.KByte.user_or_supervisor     = 0x0;
        video_memory_pte.KByte.write_through          = 0x0;
        video_memory_pte.KByte.cache_disabled         = 0x1;
        video_memory_pte.KByte.accessed               = 0x0;
        video_memory_pte.KByte.dirty                  = 0x0;
        video_memory_pte.KByte.pat                    = 0x0;
        video_memory_pte.KByte.global_page            = 0x0;
        video_memory_pte.KByte.avail                  = 0x0;
        video_memory_pte.KByte.base_address           = VIDEO >> offset_field_len; // use right shift to get the highest 20 bits
        for (i = 0; i < ENTRY_NUM; i++) {
            kernel_page_table_0_4M[i] = not_present_pte;
        }

        /* initialize 4 page table entries for the video memory and terminal video memory buffers */
        kernel_page_table_0_4M[(VIDEO & table_field) >> offset_field_len] = video_memory_pte;
        for (i = 0; i < TERM_NUM; i++) {
            video_memory_pte.KByte.base_address = ((uint32_t)(TERMINAL_VID_BEGIN + VIDMEM_SIZE*i)) >> offset_field_len;
            kernel_page_table_0_4M[( (TERMINAL_VID_BEGIN + VIDMEM_SIZE*i) & table_field) >> offset_field_len] = video_memory_pte;
        }
    }
    load_CR3((uint32_t)kernel_page_dir);
    enable_paging();
}

/**
 * brief: set up 4M page entry in kernel_page_dir, the page maps vir_addr to phy_addr
 * input: vir_addr
 *        phy_addr
 * return -1 -- fail, invalid input arguments
 *         0 -- success
 */
int32_t map_vir_to_phy_4M(uint32_t vir_addr, uint32_t phy_addr)
{
    /* check the alignment of input addr */
    if ((vir_addr & offset_field) || (phy_addr & offset_field)) return -1;

    /* set up page dir entry */
    page_directory_entry_t pde;
    pde.MByte.present                 = 0x1;
    pde.MByte.read_or_write           = 0x1;
    pde.MByte.user_or_supervisor      = 0x1; // user level
    pde.MByte.write_through           = 0x0;
    pde.MByte.cache_disabled          = 0x0;
    pde.MByte.accessed                = 0x0;
    pde.MByte.dirty                   = 0x0;
    pde.MByte.page_size               = 0x1;
    pde.MByte.global_page             = 0x0;
    pde.MByte.avail                   = 0x0;
    pde.MByte.pat                     = 0x0;
    pde.MByte.reserved                = 0x0;
    pde.MByte.base_address            = phy_addr >> (table_field_len + offset_field_len);

    kernel_page_dir[vir_addr >> (table_field_len + offset_field_len)] = pde;
    // flush the TLB
    load_CR3((uint32_t)kernel_page_dir);
    return 0;
}

/**
 * brief: unmap 4M page entry in page_dir
 * input: vir_addr to be unmapped
 * return -1 -- fail, invalid input arguments
 *         0 -- success
 */
int32_t unmap_vir_to_phy_4M(uint32_t vir_addr)
{
    /* check the alignment of input addr */
    if (vir_addr & offset_field) return -1;

    /* cannot unmap kernel memory */
    if (vir_addr == kernel_mem) return -1;

    kernel_page_dir[vir_addr >> (table_field_len + offset_field_len)].val = 0x0;
    // flush the TLB
    load_CR3((uint32_t)kernel_page_dir);
    return 0;
}

/**
 * @brief Set the usr vidmem object
 * 
 * @param vir_vmem - i don't know why it is 8-bit long
 * @param phy_vmem 
 * @return int32_t 
 */
int32_t set_usr_vidmem(uint8_t* vir_vmem, uint32_t phy_vmem)
{
    /* get the index for the page dir entry and page table entry */
    uint32_t page_dir_id = ((uint32_t) vir_vmem) >> (table_field_len + offset_field_len);
    uint32_t page_tbl_id = ((uint32_t) vir_vmem & table_field) >> offset_field_len;

    /* initialize the page dir entry first */
    kernel_page_dir[page_dir_id].val = 0;
    kernel_page_dir[page_dir_id].KByte.present              = 0x1;
    kernel_page_dir[page_dir_id].KByte.read_or_write        = 0x1;
    kernel_page_dir[page_dir_id].KByte.user_or_supervisor   = 0x1;  // user level
    kernel_page_dir[page_dir_id].KByte.write_through        = 0x0;
    kernel_page_dir[page_dir_id].KByte.cache_disabled       = 0x0;
    kernel_page_dir[page_dir_id].KByte.accessed             = 0x0;
    kernel_page_dir[page_dir_id].KByte.page_size            = 0x0;
    kernel_page_dir[page_dir_id].KByte.global_page          = 0x0;
    kernel_page_dir[page_dir_id].KByte.avail                = 0x0;
    kernel_page_dir[page_dir_id].KByte.reserved             = 0x0;
    kernel_page_dir[page_dir_id].KByte.base_address         = ((uint32_t)user_page_4K) >> offset_field_len;

    /* initialize the page table entry for usr vid */
    user_page_4K[page_tbl_id].val                           = 0x0;
    user_page_4K[page_tbl_id].KByte.present                 = 0x1;
    user_page_4K[page_tbl_id].KByte.read_or_write           = 0x1;
    user_page_4K[page_tbl_id].KByte.user_or_supervisor      = 0x1;
    user_page_4K[page_tbl_id].KByte.write_through           = 0x0;
    user_page_4K[page_tbl_id].KByte.cache_disabled          = 0x1;
    user_page_4K[page_tbl_id].KByte.accessed                = 0x0;
    user_page_4K[page_tbl_id].KByte.dirty                   = 0x0;
    user_page_4K[page_tbl_id].KByte.pat                     = 0x0;
    user_page_4K[page_tbl_id].KByte.global_page             = 0x0;
    user_page_4K[page_tbl_id].KByte.avail                   = 0x0;
    user_page_4K[page_tbl_id].KByte.base_address            = phy_vmem >> offset_field_len;

    /* initialize the page table of usr page for vid */
    uint32_t i;
    page_table_entry_t not_present_pte;
    not_present_pte.val = 0x0;
    for (i = 0; i < ENTRY_NUM; i++) {
        if (i == page_tbl_id)   continue;
        user_page_4K[i] = not_present_pte;
    }

    /* flush the TLB */
    load_CR3((uint32_t)kernel_page_dir);
    /**/
    return 0;
}


/**
 * @brief used for updating video map when terminal switch or process switch
 * 
 * @param terminal_id 
 * @return int32_t 
 */
int32_t update_usr_vidmem(int32_t terminal_id)
{
    uint8_t* vir_vmem = (uint8_t*) VIRTUAL_VMEM_BEGIN;
    uint32_t page_tbl_id = ((uint32_t) vir_vmem & table_field) >> offset_field_len;
    /* set the vidmap for incoming displaying terminal */
    /* i.e. 0x10000000 -> 0x000B8000, 0x000B8000 -> 0x000B8000 */
    if (terminal_id == current_active_termid) {
        kernel_page_table_0_4M[(VIDEO & table_field) >> offset_field_len].KByte.base_address = VIDEO >> offset_field_len;
        user_page_4K[page_tbl_id].KByte.base_address = VIDEO >> offset_field_len;
    } else {
    /* set the vidmap for background terminal */
    /* i.e. 0x10000000 -> buffer, 0x000B8000 -> buffer */
        // kernel_page_table_0_4M[(VIDEO & table_field) >> offset_field_len].KByte.base_address = 
        //     (uint32_t) (multi_terminals[terminal_id].screen_buffer) >> offset_field_len;

        user_page_4K[page_tbl_id].KByte.base_address = 
            (uint32_t) (multi_terminals[terminal_id].screen_buffer) >> offset_field_len;
    }
    /* flush the TLB */
    load_CR3((uint32_t)kernel_page_dir);
    return 0;
}

/**
 * @brief undo the mapping of set_usr_vidmem
 * @param vir_addr of page to be unmapped
 * @return 0 for success, -1 for failure
 */
int32_t unmap_usr_vidmem(uint32_t vir_addr)
{
    /* get the index for the page dir entry and page table entry */
    uint32_t page_dir_id = (vir_addr) >> (table_field_len + offset_field_len);
    uint32_t page_tbl_id = (vir_addr & table_field) >> offset_field_len;

    /* unmap the paging */
    kernel_page_dir[page_dir_id].val = 0;
    user_page_4K[page_tbl_id].val = 0;

    // flush the TLB
    load_CR3((uint32_t)kernel_page_dir);
    return 0;
}

/**
 * brief: set up user page dir
 * input: page_dir
 *        vir_addr
 *        phy_addr
 * return： -1 -- fail
 *           0 -- success 
 */
// int32_t setup_user_paging(page_directory_entry_t *page_dir, uint32_t vir_addr, uint32_t phy_addr)
// {
//     /* check page_dir pointer */
//     if (-1 == map_vir_to_phy_4M(page_dir, vir_addr, phy_addr)) return -1;
//     page_dir[1] = global_kernel_page;
//     return 0;
// }

/**
 * brief: print page dir content into screen
 * input: ptr to page_dir
 * output: none
 * side effect: change video memory
 */
void show_page_dir(page_directory_entry_t *page_dir)
{
    uint32_t i, meet_not_present;
    printf("page dir at %#x\n", page_dir);
    printf("|index    P    base    G    PS    U/S    R/W\n");
    for (i = 0, meet_not_present = 0; i < ENTRY_NUM; i++)
    {
        if (0 == page_dir[i].KByte.present || 0 == page_dir[i].MByte.present)
        {
            if (0 == meet_not_present)
            {
                printf("|...    0\n");
                meet_not_present = 1;
            }
        }
        else
        {
            meet_not_present = 0;
            if (1 == page_dir[i].MByte.page_size)
            {
                printf("|%u    1    %#x    %u    4M    %u    %u\n", i, page_dir[i].MByte.base_address, page_dir[i].MByte.global_page, page_dir[i].MByte.user_or_supervisor, page_dir[i].MByte.read_or_write);
            }
            else
            {
                printf("|%u    1    %#x    %u    4K    %u    %u\n", i, page_dir[i].KByte.base_address, page_dir[i].KByte.global_page, page_dir[i].KByte.user_or_supervisor, page_dir[i].KByte.read_or_write);
            }
        }
    }
    printf("\n");
}

/**
 * brief: print page table content into screen
 * input: ptr to page_table
 * output: none
 * side effect: change video memory
 */
void show_page_table(page_table_entry_t *page_table)
{
    uint32_t i, meet_not_present;
    printf("page table at %#x\n", page_table);
    printf("|index    P    base    G    U/S    R/W\n");
    for (i = 0, meet_not_present = 0; i < ENTRY_NUM; i++)
    {
        if (0 == page_table[i].KByte.present)
        {
            if (0 == meet_not_present)
            {
                printf("|...    0\n");
                meet_not_present = 1;
            }
        }
        else
        {
            meet_not_present = 0;
                printf("|%u    1    %#x    %u    %u    %u\n", i, page_table[i].KByte.base_address, page_table[i].KByte.global_page, page_table[i].KByte.user_or_supervisor, page_table[i].KByte.read_or_write);
        }
    }
    printf("\n");
}

void test_program_mapping(void)
{
    printf("content in virtual kernel mem: %d\n", *(int32_t*)kernel_mem);
    printf("content in virtual program mem: %d\n", *(int32_t*)program_mem);
    *(int32_t*)kernel_mem = 1;
    printf("content in virtual kernel mem: %d\n", *(int32_t*)kernel_mem);
    printf("content in virtual program mem: %d\n", *(int32_t*)program_mem);
    *(int32_t*)program_mem = 2;
    printf("content in virtual kernel mem: %d\n", *(int32_t*)kernel_mem);
    printf("content in virtual program mem: %d\n", *(int32_t*)program_mem);
}
