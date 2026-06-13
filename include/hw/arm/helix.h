#ifndef HELIX_H
#define HELIX_H

#include "qemu/osdep.h"
#include "cpu-qom.h"
#include "qemu/typedefs.h"
#include "qemu/units.h"
#include "hw/block/flash.h"
#include "qom/object.h"
#include "hw/core/boards.h"
#include "hw/arm/boot.h"
#include "system/memory.h"
#include "system/system.h"
#include "qemu/typedefs.h"
#include "system/address-spaces.h"
#include "hw/char/pl011.h"
#include "hw/core/sysbus.h"
#include "hw/sd/cadence_sdhci.h"











#define NUM_IRQS 256
#define HELIX_GIC_SPI_BASE 32

#define TYPE_HELIX_MACHINE MACHINE_TYPE_NAME("helix")
OBJECT_DECLARE_SIMPLE_TYPE(HelixMachineState, HELIX_MACHINE)

#define HELIX_DEFAULT_DRAM_SIZE (512 * MiB)



#define HELIX_ROM_BASE      0x00000000
#define HELIX_ROM_SIZE      0x10000     // 64KB

#define HELIX_SRAM_BASE     0x00010000
#define HELIX_SRAM_SIZE     0x80000     // 512KB

#define HELIX_UART_BASE     0x01000000
#define HELIX_UART_SIZE     0x1000      // 4KB

#define HELIX_GICD_BASE     0x01001000
#define HELIX_GICD_SIZE     0x10000     // 64KB

#define HELIX_GICR_BASE     0x01011000
#define HELIX_GICR_SIZE     0x20000     // 128KB

#define HELIX_SDHCI_BASE    0x01031000
#define HELIX_SDHCI_SIZE    0x10000     // 64KB

#define HELIX_DRAM_BASE     0x40000000
#define HELIX_DRAM_SIZE     0x20000000  // 512MB



enum {
    HELIX_ROM   = 0,
    HELIX_SRAM  = 1,
    HELIX_UART0 = 2,
    HELIX_GICD  = 3,
    HELIX_GICR  = 4,
    HELIX_EMMC  = 5,
    HELIX_DRAM  = 6
};

// IRQ's 16-32 for Private Peripheral IRQ's (like Generic Timer, etc), 32 - 1019 is for Shared Peripheral IRQ's (like UART, etc), 0 - 15 is for SGI's
enum {
    HELIX_UART_IRQ              = 32,
    HELIX_SECURE_TIMER_PHYS_IRQ = 29
};



typedef struct HelixMachineState {
    MachineState    parent;
    ARMCPU          *cpu;
    CPUState        *cs;
    PFlashCFI01     *bootrom;

    DeviceState     *gic;
    SysBusDevice    *gic_bus;

    DeviceState     *pl011;
    SysBusDevice    *pl011_bus;

    CadenceSDHCIState   sdhost;
    DeviceState         *plic;
    
    MemoryRegion    *sysmem;        // Non-secure system memory
    MemoryRegion    *sec_sysmem;    // Secure system memory
} HelixMachineState;

#endif