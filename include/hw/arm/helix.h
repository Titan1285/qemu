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

#define HELIX_NOR_BASE      0x00100000
#define HELIX_NOR_SIZE      0x200000    // 2MB

#define HELIX_MMIO_BASE     0x02000000

#define HELIX_UART_BASE     HELIX_MMIO_BASE
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

#define HELIX_WDT_BASE      (HELIX_MMIO_BASE + 0x080000)
#define HELIX_WDT_SIZE      0x2000      // 64KB

#define HELIX_DRAM_BASE     0x40000000
#define HELIX_DRAM_SIZE     0x3F000000  // 1GB (minus 16MB for framebuffer)

#define HELIX_FRAMEBUFFER_BASE  0x7F000000
#define HELIX_FRAMEBUFFER_SIZE  0x1000000  // 16MB




enum {
    HELIX_ROM       = 0,
    HELIX_SRAM      = 1,
    HELIX_NOR       = 2,
    HELIX_UART0     = 3,
    HELIX_GICD      = 4,
    HELIX_GICR      = 5,
    HELIX_EMMC      = 6,
    HELIX_FW_CFG    = 7,
    HELIX_USB       = 8,
    HELIX_WDT       = 9,
    HELIX_DRAM      = 10,
    HELIX_FB        = 11
};

// IRQ's 16-32 for Private Peripheral IRQ's (like Generic Timer, etc), 32 - 1019 is for Shared Peripheral IRQ's (like UART, etc), 0 - 15 is for SGI's

enum {
    // Private Peripheral IRQ's
    HELIX_IRQ_WDT               = 27,
    HELIX_SECURE_TIMER_PHYS_IRQ = 29,
    HELIX_IRQ_UART              = 32,

    // Shared Peripheral IRQ's
    HELIX_IRQ_SDHCI             = 61,
    HELIX_IRQ_XHCI              = 62
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
    PFlashCFI01         *nor;

    DeviceState         *gic;
    SysBusDevice        *gic_bus;

    DeviceState         *usb;

    DeviceState         *pl011;
    SysBusDevice        *pl011_bus;

    CadenceSDHCIState   sdhost;         // SDHCI EMMC host
    DeviceState         *plic;

    DeviceState         *wdt;           // Watchdog timer

    FWCfgState          *fw_cfg;        // Firware config

    MemoryRegion        *fb_mem;        // Framebuffer memory region
    RAMFBState          *ramfb;         // Ram framebuffer state

    MemoryRegion        *sysmem;        // Non-secure system memory
    MemoryRegion        *sec_sysmem;    // Secure system memory (currently unused)
} HelixMachineState;

#endif