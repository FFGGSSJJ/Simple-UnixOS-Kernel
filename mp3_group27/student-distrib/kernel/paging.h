/**
 * @file paging.h
 * @brief Defines paging-related data structure and functions
 * @version 0.1
 * @date 2022-03-17
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "../types.h"
#include "../lib.h"
#include "../drivers/terminal.h"

#define kernel_mem          0x00400000
#define program_size        0x00400000
#define program_mem         0x08000000
#define PROGRAM_IMG_START   0x08000000
#define PROGRAM_IMG_BEGIN   0x08048000
#define PRPGRAM_IMG_END     0x08400000
#define VIRTUAL_VMEM_BEGIN  0x10000000      // make sure it does not overlap with program img
#define PHYSICAL_VMEM_BEGIN 0x000B8000      // the start of vid 
#define TERMINAL_VID_BEGIN  0x00300000      // 3MB as the start of the terminal video memory
#define VIDMEM_SIZE         0x00001000      // 4KB size for video memory
#define TERM_NUM            3
#define PAGE_SIZE           1024

typedef union page_directory_entry {
    uint32_t val;
    struct KByte {
        uint32_t present            : 1;    // When the flag is set, the page is in physical memory.
        uint32_t read_or_write      : 1;    // When the flag is set, the page can be read and written into.
        uint32_t user_or_supervisor : 1;    // When this flag is clear, the page is assigned the supervisor privilege level.
        uint32_t write_through      : 1;    // 1 indicates write-through caching is enabled, otherwise write-back caching is enabled
        uint32_t cache_disabled     : 1;    // 1 indicates the caching of individual pages or page tables is disabled
        uint32_t accessed           : 1;
        uint32_t reserved           : 1;    // set to 0
        uint32_t page_size          : 1;    // 0 indicates 4 KBytes
        uint32_t global_page        : 1;    // ignored
        uint32_t avail              : 3;    // available for system programmer’s use
        uint32_t base_address       : 20;   // page table base address
    } KByte __attribute__ ((packed));
    struct MByte {
        uint32_t present            : 1;    // When the flag is set, the page is in physical memory.
        uint32_t read_or_write      : 1;    // When the flag is set, the page can be read and written into.
        uint32_t user_or_supervisor : 1;    // When this flag is clear, the page is assigned the supervisor privilege level.
        uint32_t write_through      : 1;    // 1 indicates write-through caching is enabled, otherwise write-back caching is enabled
        uint32_t cache_disabled     : 1;    // 1 indicates the caching of individual pages or page tables is disabled
        uint32_t accessed           : 1;
        uint32_t dirty              : 1;
        uint32_t page_size          : 1;    // 1 indicates 4 MBytes
        uint32_t global_page        : 1;    // 1 indicates a global page
        uint32_t avail              : 3;    // available for system programmer’s use
        uint32_t pat                : 1;    // Page Table Attribute Index, for processors that do not support the PAT, this bit is reserved and should be set to 0.
        uint32_t reserved           : 9;    // must be set to 0
        uint32_t base_address       : 10;   // page base address
    } MByte __attribute__ ((packed));
} page_directory_entry_t;

typedef union page_table_entry {
    uint32_t val;
    struct {
        uint32_t present            : 1;    // When the flag is set, the page is in physical memory.
        uint32_t read_or_write      : 1;    // When the flag is set, the page can be read and written into.
        uint32_t user_or_supervisor : 1;    // When this flag is clear, the page is assigned the supervisor privilege level.
        uint32_t write_through      : 1;    // 1 indicates write-through caching is enabled, otherwise write-back caching is enabled
        uint32_t cache_disabled     : 1;    // 1 indicates the caching of individual pages or page tables is disabled
        uint32_t accessed           : 1;
        uint32_t dirty              : 1;
        uint32_t pat                : 1;    // Page Table Attribute Index
        uint32_t global_page        : 1;    // 1 indicates a global page
        uint32_t avail              : 3;    // available for system programmer’s use
        uint32_t base_address       : 20;   // page base address
    } KByte __attribute__ ((packed));
} page_table_entry_t;

// create kernel page directory and page table
page_directory_entry_t kernel_page_dir[PAGE_SIZE] __attribute__((aligned(4 * PAGE_SIZE)));
page_table_entry_t kernel_page_table_0_4M[PAGE_SIZE] __attribute__((aligned(4 * PAGE_SIZE)));

/* this page is set for user to access the video memory in differrnt process */
page_table_entry_t user_page_4K[PAGE_SIZE] __attribute__((aligned(4 * PAGE_SIZE)));

/** initialize page directory and page tabel in memory */
void paging_init_kernel(void);

/* set up 4M page entry in page_dir, the page maps vir_addr to phy_addr */
int32_t map_vir_to_phy_4M(uint32_t vir_addr, uint32_t phy_addr);

/* unmap 4M page entry in page_dir */
int32_t unmap_vir_to_phy_4M(uint32_t vir_addr);

/* initialize the 4KB page set up for user vid */
int32_t set_usr_vidmem(uint8_t* vir_vmem, uint32_t phy_vmem);

/* update the video memory map when switch terminal */
int32_t update_usr_vidmem(int32_t terminal_id);

/* undo the mapping of set_usr_vidmem */
int32_t unmap_usr_vidmem(uint32_t vir_addr);

/* set up user page dir */
// int32_t setup_user_paging(page_directory_entry_t *page_dir, uint32_t vir_addr, uint32_t phy_addr);

/** print page dir content in screen */
void show_page_dir(page_directory_entry_t *page_dir);

/** print page table content in screen */
void show_page_table(page_table_entry_t *page_table);

void test_program_mapping(void);

#endif /* _PAGING_H */
