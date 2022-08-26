/**
 * @file pci.h
 * @author guanshujie fu
 * @brief 
 * @version 0.1
 * @date 2022-04-27
 * 
 * @copyright Copyright (c) 2022
 * @ref https://wiki.osdev.org/PCI
 *      https://github.com/xddxdd/uiuc-ece391-mp3
 *      https://devicehunt.com/all-pci-vendors (vendor and device ids in pci)
 */

#ifndef PCI_H
#define PCI_H

#include "../lib.h"
#include "../types.h"
#include "vbe.h"

/* PCI_REG_ADDR specifies the configuration address that is required to be accesses, 
 * while accesses to PCI_REG_DATA will actually generate the configuration access and 
 * will transfer the data to or from the CONFIG_DATA register.*/
#define PCI_REG_ADDR 0xCF8
#define PCI_REG_DATA 0xCFC

/* Mask to be used in Base Address Register */
#define PCI_MASK_BAR_MEMSPACE   0xfffffff0
#define PCI_MASK_BAR_IOSPACE    0xfffffffc

/**/
#define PCI_COUNT_BUS               255
#define PCI_COUNT_DEVICE            32
#define PCI_COUNT_FUNC              8
#define PCI_COUNT_REG               64
#define PCI_QEMUVGA_VENDOR_ID       0x1234
#define PCI_QEMUVGA_DEVICE_ID       0x1111
#define PCI_INVALID_ID              0xFFFF

#define PCI_TYPE_STANDARD           0
#define PCI_TYPE_PCI_PCI            1       // PCI-PCI Bridge
#define PCI_TYPE_CARDBUS            2       // PCI-CardBus bridge

#define SUCCESS                     1
#define FAIL                        -1


/* Configuration Space Access Mechanism #1 is utilized for PCI initialization */
/* struct for register PCI_REG_ADDR - 32-bit */
/*  | Bit 31	   | Bits 30-24	    | Bits 23-16	| Bits 15-11	  | Bits 10-8	    | Bits 7-0 (last 2 bits are always 0)
    | Enable Bit   | Reserved	    | Bus Number	| Device Number	  | Function Number	| Register Offset1*/
typedef union {
    uint32_t val;
    struct __attribute__((packed)) {
        uint8_t reserved0   : 2;
        uint8_t reg_num     : 6;
        uint8_t func_num    : 3;
        uint8_t device_num  : 5;
        uint8_t bus_num     : 8;
        uint8_t reserved1   : 7;
        uint8_t enable      : 1;
    };
} pci_addr_t;


/* struct for PCI Device Structure (256-byte Configuration space) */
/* Three Device Types: Normal, PCI-PCI Bridge, PCI-CardBus bridge
 * Header Filed:       0x00    0x01            0x02
 * 
 * 
 * Note:    val[18] - 72 bytes: device requires 64 bytes, bridges require 72 bytes
 * Note:    BAR stands for Base Address Register. 
 *          Base Address Registers (or BARs) can be used to hold memory addresses used by the device, 
 *          or offsets for port addresses. Typically, memory address BARs need to be located in physical ram 
 *          while I/O space BARs can reside at any memory address (even beyond physical memory).
 * Note:    Normal devices like VGA have 6 bars. 
 *          * Memory Space Bar layout:
 *          | Bits 31-4	                    | Bit 3	        | Bits 2-1	| Bit 0
 *          | 16-Byte Aligned Base Address	| Prefetchable	| Type	    | Always 0
 * 
 *          * I/O Space Bar Layout:
 *          | Bits 31-2	                    | Bit 1	    | Bit 0
            | 4-Byte Aligned Base Address	| Reserved	| Always 1
 */
typedef union {
    uint32_t val[18];
    struct {
        // Basic information, shared among all 3 types.
        uint16_t vendor_id;
        uint16_t device_id;
        uint16_t command;
        uint16_t status;
        uint8_t revision_id;
        uint8_t prog_if;
        uint8_t subclass;
        uint8_t class_code;
        uint8_t cache_line_size;
        uint8_t latency_timer;
        uint8_t header_type;
        uint8_t bist;
        union {
            // For ordinary devices
            struct {
                uint32_t bar[6];
                uint32_t cardbus_cis_ptr;
                uint16_t subsystem_vendor_id;
                uint16_t subsystem_id;
                uint32_t expansion_rom_base_addr;
                uint8_t capabilities_pointer;
                uint8_t reserved[7];
                uint8_t interrupt_line;
                uint8_t interrupt_pin;
                uint8_t min_grant;
                uint8_t max_latency;
            } device;
            // For PCI-PCI bridge
            struct {
                uint32_t bar[2];
                uint8_t primary_bus_num;
                uint8_t secondary_bus_num;
                uint8_t subordinate_bus_num;
                uint8_t secondary_latency_timer;
                uint8_t io_base;
                uint8_t io_limit;
                uint16_t secondary_status;
                uint16_t memory_base;
                uint16_t memory_limit;
                uint16_t prefetchable_memory_base;
                uint16_t prefetchable_memory_limit;
                uint32_t prefetchable_base_upper;
                uint32_t prefetchable_limit_upper;
                uint16_t io_base_upper;
                uint16_t io_limit_upper;
                uint8_t capability_pointer;
                uint8_t reserved[3];
                uint32_t expansion_rom_base_addr;
                uint8_t interrupt_line;
                uint8_t interrupt_pin;
                uint16_t bridge_control;
            } pci_bridge;
            // For CardBus bridge
            struct {
                uint32_t cardbus_socket_base_addr;
                uint8_t offset_capabilities_list;
                uint8_t reserved;
                uint16_t secondary_status;
                uint8_t pci_bus_num;
                uint8_t cardbus_bus_num;
                uint8_t subordinate_bus_num;
                uint8_t cardbus_latency_timer;
                uint32_t mem_base_addr0;
                uint32_t mem_limit0;
                uint32_t mem_base_addr1;
                uint32_t mem_limit1;
                uint32_t io_base_addr0;
                uint32_t io_limit0;
                uint32_t io_base_addr1;
                uint32_t io_limit1;
                uint8_t interrupt_line;
                uint8_t interrupt_pin;
                uint16_t bridge_control;
                uint16_t subsystem_device_id;
                uint16_t subsystem_vendor_id;
                uint32_t legacy_mode_base_addr;
            } cardbus_bridge;
        };
    };
} pci_device_t;



/* functions to initialize the VGA */
void pci_init();
int32_t checkDevice(uint8_t bus);
int32_t checkFunc(uint8_t bus, uint8_t device, pci_device_t* target_dev);


#endif
