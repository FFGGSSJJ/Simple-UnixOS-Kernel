/**
 * @file filesystem.h
 * @brief Defines filesystem structs and declares functions
 * @version 0.1
 * @date 2022-03-26
 */

#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "../types.h"
#include "../multiboot.h"
#include "../lib.h"

#define blk_size 4096
#define boot_blk_reserved_len 52
#define dentry_size 64
#define dentry_count_max 63
#define dentry_reserved_len 24
#define filename_len_max 32
#define data_blk_count_max 1023

#define RTC_TYPE 0
#define DIR_TYPE 1
#define REG_TYPE 2


typedef struct dentry
{
    int8_t      filename[filename_len_max];
    int32_t     filetype;
    int32_t     inode_num;
    int8_t      reserved[24];
} dentry_t;

typedef struct boot_blk
{
    int32_t     dir_count;
    int32_t     inode_count;
    int32_t     data_blk_count;
    int8_t      reserved[boot_blk_reserved_len];
    dentry_t    dentries[dentry_count_max];
} boot_blk_t;

typedef struct inode
{
    int32_t     length; // file length in Byte
    int32_t     data_blk_index[data_blk_count_max];
} inode_t;

/* global pointer for boot_blk */
boot_blk_t *boot_blk_ptr;

/* get filesystem module address */
#define init_filesystem(mbi) boot_blk_ptr = (boot_blk_t*)(((module_t*)mbi->mods_addr)->mod_start) // Note that filesystem module is the first module
#define GET_FILE_SIZE(dentry_ptr) ((inode_t *)(boot_blk_ptr + 1 + dentry_ptr->inode_num))->length // +1 to jump over the boot block

/* show the content of show the content of the given dentry */
void show_dentry(dentry_t *dentry);

/* show the content of filesystem boot block */
void show_boot_blk(void);

/* fill in the dentry_t struct basing on the given name */
int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry);

/* fill in the dentry_t struct basing on the given index */
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry);

/* read data from data block basing on specified inode, and copy them into buf */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length);

/* open operation for regular file (type 2) */
int32_t file_open(void);

/* read data from data block basing on specified inode, and copy them into buf */
int32_t file_read(uint32_t inode_num, uint32_t position, void *buf, uint32_t length);

/* writes data into the file specified by the inode number */
int32_t file_write(uint32_t inode_num, void *buf, uint32_t length);

/* close operation for regular file (type 2) */
int32_t file_close(void);

/* directory operations */
int32_t dir_open(void);

int32_t dir_read(uint32_t inode_num, int32_t position, void *buf, uint32_t length);

int32_t dir_write(int32_t position, void *buf, uint32_t length);

int32_t dir_close(void);

#endif /* _FILESYSTEM_H */
