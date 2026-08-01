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
#include "qom/object.h"
#include "hw/display/ramfb.h"
#include "ui/surface.h"














#define NUM_IRQS 256
#define HELIX_GIC_SPI_BASE 32

#define TYPE_HELIX_MACHINE MACHINE_TYPE_NAME("helix")
OBJECT_DECLARE_SIMPLE_TYPE(HelixMachineState, HELIX_MACHINE)

#define TYPE_HELIX_RAMFB    "helix-ramfb"
OBJECT_DECLARE_SIMPLE_TYPE(HelixFramebufferState, HELIX_RAMFB)




#define HELIX_ROM_BASE      0x00000000
#define HELIX_ROM_SIZE      0x10000     // 64KB

#define HELIX_SRAM_BASE     0x00010000
#define HELIX_SRAM_SIZE     0x80000     // 512KB

#define HELIX_MMIO_BASE     0x01000000

#define HELIX_UART_BASE     (HELIX_MMIO_BASE + 0x000000)
#define HELIX_UART_SIZE     0x1000      // 4KB

#define HELIX_GICD_BASE     (HELIX_MMIO_BASE + 0x010000)
#define HELIX_GICD_SIZE     0x10000     // 64KB

#define HELIX_GICR_BASE     (HELIX_MMIO_BASE + 0x020000)
#define HELIX_GICR_SIZE     0x20000     // 128KB

#define HELIX_SDHCI_BASE    (HELIX_MMIO_BASE + 0x040000)
#define HELIX_SDHCI_SIZE    0x10000     // 64KB

#define HELIX_FW_CFG_BASE   (HELIX_MMIO_BASE + 0x050000)
#define HELIX_FW_CFG_SIZE   0x20        // 32 bytes

#define HELIX_USB_BASE      (HELIX_MMIO_BASE + 0x060000)
#define HELIX_USB_SIZE      0x2000      // 64KB

#define HELIX_DRAM_BASE     0x40000000
#define HELIX_DRAM_SIZE     0x3FFF0000 // 1GB (minus 16MB for framebuffer)

#define HELIX_FRAMEBUFFER_BASE  0x7FFF0000
#define HELIX_FRAMEBUFFER_SIZE  (16 * MiB)




enum {
    HELIX_ROM       = 0,
    HELIX_SRAM      = 1,
    HELIX_UART0     = 2,
    HELIX_GICD      = 3,
    HELIX_GICR      = 4,
    HELIX_EMMC      = 5,
    HELIX_FW_CFG    = 6,
    HELIX_DRAM      = 7,
    HELIX_FB        = 8
};

// IRQ's 16-32 for Private Peripheral IRQ's (like Generic Timer, etc), 32 - 1019 is for Shared Peripheral IRQ's (like UART, etc), 0 - 15 is for SGI's
enum {
    HELIX_UART_IRQ              = 32,
    HELIX_SECURE_TIMER_PHYS_IRQ = 29
};

struct RAMFBCfg {
    uint64_t addr;
    uint32_t fourcc;
    uint32_t flags;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
};

typedef struct HelixFramebufferState {
    QemuConsole     *con;
    struct RAMFBCfg ramfb_cfg;
} HelixFramebufferState;

typedef struct HelixMachineState {
    MachineState        parent;
    ARMCPU              *cpu;
    CPUState            *cs;
    PFlashCFI01         *bootrom;

    DeviceState         *gic;
    SysBusDevice        *gic_bus;

    DeviceState         *pl011;
    SysBusDevice        *pl011_bus;

    CadenceSDHCIState   sdhost;
    DeviceState         *plic;

    FWCfgState          *fw_cfg;

    QemuConsole         *con;
    MemoryRegion        *fb_mem;
    RAMFBState          *ramfb;

    MemoryRegion        *sysmem;        // Non-secure system memory
    MemoryRegion        *sec_sysmem;    // Secure system memory (currently unused)
} HelixMachineState;

#endif