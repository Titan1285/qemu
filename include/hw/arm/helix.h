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









#define TYPE_HELIX_MACHINE MACHINE_TYPE_NAME("helix")
OBJECT_DECLARE_SIMPLE_TYPE(HelixMachineState, HELIX_MACHINE)

#define HELIX_DEFAULT_DRAM_SIZE (512 * MiB)



#define HELIX_ROM_BASE  0x00000000
#define HELIX_ROM_SIZE  0x10000     // 64KB

#define HELIX_SRAM_BASE 0x00010000
#define HELIX_SRAM_SIZE 0x40000     // 256KB

#define HELIX_UART_BASE 0x00050000
#define HELIX_UART_SIZE 0x1000      // 4KB

#define HELIX_EMMC_BASE 0x00051000
#define HELIX_EMMC_SIZE 0x1000      // 4KB (TODO: Verify this...)

#define HELIX_DRAM_BASE 0x40000000
#define HELIX_DRAM_SIZE 0x20000000  // 512MB



enum {
    HELIX_ROM   = 0,
    HELIX_SRAM  = 1,
    HELIX_UART0 = 2,
    HELIX_EMMC  = 3,
    HELIX_DRAM  = 4
};



typedef struct HelixMachineState {
    MachineState    parent;
    ARMCPU          *cpu;
    CPUState        *cs;
    PFlashCFI01     *flash;

    MemoryRegion    *sysmem;        // Non-secure system memory
    MemoryRegion    *sec_sysmem;    // Secure system memory

    char    *bootrom_path;
    char    *emmc_path;
} HelixMachineState;

#endif