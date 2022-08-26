/**
 * @file system_call.h
 * @brief 
 * @version 0.1
 * @date 2022-04-06
 */

#include "../lib.h"
#include "pcb.h"
#include "../drivers/filesystem.h"
#include "paging.h"
#include "../x86_desc.h"

int32_t halt(uint16_t status);

int32_t execute(const uint8_t* command);

int32_t read(int32_t fd, void* buf, int32_t nbytes);

int32_t write(int32_t fd, const void* buf, int32_t nbytes);

int32_t open(const uint8_t* filename);

int32_t close(int32_t fd);

int32_t getargs(uint8_t* buf, int32_t nbytes);

int32_t vidmap(uint8_t** screen_start);

int32_t set_handler(int32_t signum, void* handler_address);

int32_t sigreturn(void);
