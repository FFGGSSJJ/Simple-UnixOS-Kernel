/**
 * @file filesystem.c
 * @brief
 * @version 0.1
 * @date 2022-03-26
 */

#include "filesystem.h"


/**
 * brief: show the content of the given dentry
 * input: dentry -- dentry_t ptr
 * output: print dentry's content in the screen
 * return: none
 * side effect: none
 */
void show_dentry(dentry_t *dentry)
{
    int32_t i;
    int8_t *character;
    printf("filename: ");
    if (dentry == NULL) return;
    character = dentry->filename;
    for (i = 0; i < filename_len_max && '\0' != *character; i++, character++) // handle the case without terminal EOS
    {
        putc(*character);
    }
    printf("    type: %d, inode: %d, size: %d\n",
           dentry->filetype,
           dentry->inode_num,
           GET_FILE_SIZE(dentry));
}

/**
 * brief: show the content of filesystem's boot block
 * input: none
 * output: print boot_blk's content in the screen
 * return: none
 * side effect: none
 */
void show_boot_blk(void)
{
    int32_t i;
    printf("Number of dir entries: %d\n", boot_blk_ptr->dir_count);
    printf("Number of inodes: %d\n", boot_blk_ptr->inode_count);
    printf("Number of data blocks: %d\n", boot_blk_ptr->data_blk_count);
    for (i = 0; i < boot_blk_ptr->dir_count; i++)
    {
        show_dentry(boot_blk_ptr->dentries + i);
    }
}

/**
 * brief: fill in the dentry_t struct basing on the given index
 * input: fname -- file name
 *        dentry -- dentry_t ptr
 * output: if success, copy the matched dentry's content into the given dentry struct
 * return: -1 -- fname is not valid or file does not exist
 *          0 -- success
 * side effect: none
 */
int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry)
{
    // check if fname is valid
    if (fname == NULL)
        return -1;
    // check if fname is within filename_len_max
    if (strlen((int8_t *)fname) > filename_len_max)
        return -1;

    // serach through all the directory entries
    int i;
    for (i = 0; i < dentry_count_max; i++)
    {
        if (strncmp((int8_t *)fname, boot_blk_ptr->dentries[i].filename, filename_len_max) == 0)
        {
            // copy dentry info
            memcpy(dentry, boot_blk_ptr->dentries + i, dentry_size);
            return 0;
        }
    }
    // file not exist
    return -1;
}

/**
 * brief: fill in the dentry_t struct basing on the given index
 * input: index -- index into the boot block for directory entry
 *        dentry -- dentry_t ptr
 * output: if success, copy the matched dentry's content into the given dentry struct
 * return: -1 -- index is not valid
 *          0 -- success
 * side effect: none
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry)
{
    if (index >= boot_blk_ptr->dir_count)   return -1;

    // copy dentry info
    memcpy(dentry, boot_blk_ptr->dentries + index, dentry_size);

    return 0;
}



/**
 * brief: fill in the buf with data read from inode, offset with length
 * input: inode -- inode id
 *        offset -- offset for file data
 *        length -- read data length
 * output: if success, copy the matched content into the given buf
 * return: len -- read length
 *          -1 -- fail
 * side effect: none
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length)
{
    inode_t *inode_ptr;
    int32_t copy_end; // the offest (index) of the last byte to be copied +1
    int32_t blk_index;
    uint8_t *blk_ptr;
    int32_t blk_offset; // offest inside a block
    int32_t blk_cp_length; // length to copy inside a block
    int32_t len_copied = 0;
    
    // bad input arguments
    if (inode >= boot_blk_ptr->inode_count || buf == NULL) return -1;
    // initialize the loop
    inode_ptr = (inode_t *)(boot_blk_ptr + 1 + inode); // +1 to jump over the boot block
    copy_end = (offset + length) > inode_ptr->length ? inode_ptr->length : offset + length;

    while (offset < copy_end)
    {
        // update block index, block pointer
        blk_index = (inode_ptr->data_blk_index)[offset / blk_size];
        if (blk_index >= boot_blk_ptr->data_blk_count) return -1; // bad block index
        blk_ptr = (uint8_t *)(boot_blk_ptr + 1 + boot_blk_ptr->inode_count + blk_index);

        // compute offest inside a block and blk_cp_length (number of bytes to be copied inside a block)
        blk_offset = offset % blk_size;
        blk_cp_length = (copy_end - offset) > (blk_size - blk_offset) ? blk_size - blk_offset : copy_end - offset;

        // copy the content to buf and update length copied
        memcpy(buf, blk_ptr + blk_offset, blk_cp_length);
        len_copied += blk_cp_length;

        // update buf pointer, and offset
        buf += blk_cp_length;
        offset += blk_cp_length;
    }
    return len_copied;
}

/*#####################################################################
 * Here goes the file operations defined for regular file and directory
######################################################################*/

/**
 * brief: open operation for regular file (type 2)
 * input: none
 * output: none
 * return: 0 -- sucess
 * side effect: none
 */
int32_t file_open(void)
{
    return 0;
}

/**
 * brief: read data from data block basing on specified inode, and copy them into buf
 * input: inode -- inode index (inode number)
 *        offset -- starting position of read operation
 *        buf -- destination where the read bytes will be stored
 *        length -- number of bytes need to read
 * output: put bytes read into buf
 * return: number of bytes read
 * side effect: none
 */
int32_t file_read(uint32_t inode_num, uint32_t position, void *buf, uint32_t length)
{
    if (buf == NULL)
        return -1;
    return read_data(inode_num, position, (uint8_t*)buf, length);
}

/**
 * brief: writes data into the file specified by the inode number
 * input: buf -- store the bytes that will be written into the file
 *        length -- number of bytes need to write
 * output: put bytes from buf into the file specified by the inode number
 * return: -1 -- fail
 * side effect: none
 */
int32_t file_write(uint32_t inode, void *buf, uint32_t length)
{
    inode_t *inode_ptr;
    int32_t write_end; // the offest (index) of the last byte to be copied +1
    int32_t blk_index;
    uint8_t *blk_ptr;
    int32_t blk_wt_length; // length to copy inside a block
    int32_t blk_num_total; // total number of data blocks available for an inode
    int32_t offset; // offset of data written realatively to the whole file
    
    // bad input arguments
    if (inode >= boot_blk_ptr->inode_count || buf == NULL) return -1;
    // initialize the loop
    inode_ptr = (inode_t *)(boot_blk_ptr + 1 + inode); // +1 to jump over the boot block
    blk_num_total = (inode_ptr->length)/blk_size + (int32_t)(((inode_ptr->length) % blk_size) > 0);
    write_end = length > blk_num_total*blk_size ? blk_num_total*blk_size : length;
    offset = 0;

    while (offset < write_end)
    {
        // update block index, block pointer
        blk_index = (inode_ptr->data_blk_index)[offset / blk_size];
        if (blk_index >= boot_blk_ptr->data_blk_count) return -1; // bad block index
        blk_ptr = (uint8_t *)(boot_blk_ptr + 1 + boot_blk_ptr->inode_count + blk_index);

        // compute offest inside a block and blk_wt_length (number of bytes to be written inside a block)
        blk_wt_length = (write_end - offset) > blk_size ? blk_size : write_end - offset;

        // copy the content to buf and update length copied
        memcpy(blk_ptr, (uint8_t *)buf + offset, blk_wt_length);

        // update buf pointer, and offset
        offset += blk_wt_length;
    }
    // update file length
    inode_ptr->length = write_end;
    return write_end;
}

/**
 * brief: close operation for regular file (type 2)
 * input: none
 * output: none
 * return: 0 -- sucess
 * side effect: none
 */
int32_t file_close(void)
{
    return 0;
}

/**
 * brief: open directory
 * input: none
 * output: 
 * return: 0
 * side effect: none
 * It is an useless function whose functionality can be replaced by open()
 */
int32_t dir_open(void)
{
    return 0;
}


/**
 * brief: read directory
 * input: position -- which dentry in a directory to read
 *        buf -- store the read data
 *        length -- self-explained
 * output: 
 * return: 0
 * side effect: none
 * It will be called when system call read() is called. 
 */
int32_t dir_read(uint32_t inode_num, int32_t position, void *buf, uint32_t length)
{
    if (buf == NULL)    return -1;
    if (position >= boot_blk_ptr->dir_count)   return 0;
    if (0 == length) return 0;

    // read dir
    dentry_t tmp_dentry;
    read_dentry_by_index(position, &tmp_dentry);

    // copy filename
    int8_t *scan = tmp_dentry.filename;
    int32_t i;
    for (i = 0; i < filename_len_max && '\0' != *scan; i++) // handle the case without terminal EOS
    {
        ((int8_t*)buf)[i] = *scan++;
    }

    return i;
}


/**
 * brief: write directory
 * input: position -- which dentry in a directory to write
 *        buf -- the data to write
 *        length -- self-explained
 * output: 
 * return: -1
 * side effect: none
 * It will be called when system call read() is called. 
 */
int32_t dir_write(int32_t position, void *buf, uint32_t length)
{
    return -1;
}


/**
 * brief: close directory
 * input: 
 * output: 
 * return: 0
 * side effect: none
 * It will be called when system call close() is called. 
 */
int32_t dir_close(void)
{
    return 0;
}
