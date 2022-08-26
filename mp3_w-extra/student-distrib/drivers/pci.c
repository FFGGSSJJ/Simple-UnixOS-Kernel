/**
 * @file pci.c
 * @author your name (you@domain.com)
 * @brief PCI is initialized only for registering VGA and RTL8139
 * @version 0.1
 * @date 2022-04-27
 * 
 * @copyright Copyright (c) 2022
 * @ref see pci.h
 * 
 */
#include "pci.h"



#if 1

/**
 * @brief check all devices on the bus
 * 
 * @param bus 
 * @return int32_t 
 */
int32_t checkDevice(uint8_t bus)
{
    int32_t device;
    for (device = 0; device < PCI_COUNT_DEVICE; device++) {
        pci_device_t target_dev;
        if (SUCCESS == checkFunc(bus, device, &target_dev)) 
            return SUCCESS;
        continue;
    } return FAIL;
}


/**
 * @brief check and register the QEMU VGA device
 * 
 * @param bus 
 * @param device 
 * @param target_dev 
 * @return int32_t 
 */
int32_t checkFunc(uint8_t bus, uint8_t device, pci_device_t* target_dev)
{
    pci_addr_t addr;
    int32_t func;
    addr.val = 0;
    addr.bus_num = bus;
    addr.device_num = device;
    addr.enable = 1;

    for (func = 0; func < PCI_COUNT_FUNC; func++) {
        addr.func_num = func;
        /* Register upper bound, 0x10 for standard devices, 0x12 for CardBus */
        /* 0x10 = 16. Every time we read 4 bytes and 16*4 = 64 bytes */
        uint8_t reg_bound = 0x10;
        for (addr.reg_num = 0; addr.reg_num < reg_bound; addr.reg_num++) {
            outl(addr.val, PCI_REG_ADDR);
            target_dev->val[addr.reg_num] = inl(PCI_REG_DATA);
        }

        /* target info read finishes */
        if (target_dev->vendor_id == PCI_INVALID_ID || target_dev->device_id == PCI_INVALID_ID) 
            continue;
        printf("PCI device %x:%x.%x: %x:%x\n", bus, device, func, target_dev->vendor_id, target_dev->device_id);
        if (target_dev->vendor_id == PCI_QEMUVGA_VENDOR_ID && target_dev->device_id == PCI_QEMUVGA_DEVICE_ID) {
            qemu_vga_addr = target_dev->device.bar[0] & PCI_MASK_BAR_MEMSPACE;
            printf("\n### QEMU VGA Adapter detected, vram=%x ###\n", qemu_vga_addr);
            return SUCCESS;
        }
    }
    return FAIL;
}
#endif



/**
 * @brief void pci_init()
 * output: PCI devices get scanned and initialized
 * description: scans every slot of the PCI bus, detects and initializes everything.
 */
void pci_init() {
    uint8_t bus;

    // Smart scan of the whole PCI bus.
    // Start with bus 0, and if a PCI bridge is detected, add that bridge into list.
    for(bus = 0; bus < PCI_COUNT_BUS; bus++) {
        printf("PCI scan on bus %d\n", bus);
        if (SUCCESS == checkDevice(bus)) {
            printf("PCI scan complete\n");
            return;
        }
    }
}




