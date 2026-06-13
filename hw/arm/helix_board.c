#include "qemu/osdep.h"
#include "hw/arm/helix.h"
#include "qemu/qemu-print.h"
#include "qemu/typedefs.h"
#include "qemu/units.h"
#include "qobject/qlist.h"
#include "qom/object.h"
#include "hw/core/boards.h"
#include "hw/arm/boot.h"
#include "hw/arm/machines-qom.h"
#include "cpu-qom.h"
#include "qemu/error-report.h"
#include "system/address-spaces.h"
#include "qapi/error.h"
#include "hw/core/loader.h"
#include "system/reset.h"
#include "hw/block/flash.h"
#include "hw/core/qdev-properties.h"
#include "system/system.h"
#include "hw/intc/arm_gicv3_common.h"
#include "exec/cpu-common.h"
#include "system/blockdev.h"
#include "system/system.h"
#include "target/arm/gtimer.h"
#include "hw/arm/bsa.h"
#include "hw/intc/arm_gic_common.h"
#include "hw/sd/sdhci.h"
#include "hw/sd/cadence_sdhci.h"
























static const MemMapEntry helix_memmap[] = {
    [HELIX_ROM]     = {HELIX_ROM_BASE, HELIX_ROM_SIZE},
    [HELIX_SRAM]    = {HELIX_SRAM_BASE, HELIX_SRAM_SIZE},
    [HELIX_UART0]   = {HELIX_UART_BASE, HELIX_UART_SIZE},
    [HELIX_GICD]    = {HELIX_GICD_BASE, HELIX_GICD_SIZE},
    [HELIX_GICR]    = {HELIX_GICR_BASE, HELIX_GICR_SIZE},
    [HELIX_EMMC]    = {HELIX_SDHCI_BASE, HELIX_SDHCI_SIZE},
    [HELIX_DRAM]    = {HELIX_DRAM_BASE, HELIX_DRAM_SIZE}
};




static void helix_gic_init(MachineState *ms) {
    HelixMachineState *m = HELIX_MACHINE(ms);
    const char *gic_type;
    QList *redist_region_count;
    uint32_t redist_capacity, redist_count;
    uint32_t num_smp = ms->smp.cpus;




    gic_type = gicv3_class_name();
    m->gic = qdev_new(gic_type);

    qdev_prop_set_uint32(m->gic, "revision", 3);
    qdev_prop_set_uint32(m->gic, "num-cpu", num_smp ? num_smp : 1);

    qdev_prop_set_uint32(m->gic, "num-irq", NUM_IRQS + 32);
    qdev_prop_set_bit(m->gic, "has-security-extensions", false);

    redist_capacity = helix_memmap[HELIX_GICR].size / GICV3_REDIST_SIZE;
    redist_count = MIN(num_smp ? num_smp : 1, redist_capacity);

    redist_region_count = qlist_new();
    qlist_append_int(redist_region_count, redist_count);
    qdev_prop_set_array(m->gic, "redist-region-count", redist_region_count);

    object_property_set_link(OBJECT(m->gic), "sysmem", OBJECT(m->sysmem), &error_fatal);
    qdev_prop_set_bit(m->gic, "has-lpi", true);

    m->gic_bus = SYS_BUS_DEVICE(m->gic);
    sysbus_realize_and_unref(m->gic_bus, &error_fatal);

    sysbus_mmio_map(m->gic_bus, 0, helix_memmap[HELIX_GICD].base);
    sysbus_mmio_map(m->gic_bus, 1, helix_memmap[HELIX_GICR].base);

    for (uint32_t i = 0; i < num_smp; i++) {
        DeviceState *cpu = DEVICE(qemu_get_cpu(i));
        int intid_base = NUM_IRQS + i * GIC_INTERNAL;
        int irq;

        const int timer_irq[] = {
            [GTIMER_PHYS]       = ARCH_TIMER_NS_EL1_IRQ,
            [GTIMER_VIRT]       = ARCH_TIMER_VIRT_IRQ,
            [GTIMER_HYP]        = ARCH_TIMER_NS_EL2_IRQ,
            [GTIMER_SEC]        = ARCH_TIMER_S_EL1_IRQ,
            [GTIMER_HYPVIRT]    = ARCH_TIMER_NS_EL2_VIRT_IRQ,
            [GTIMER_S_EL2_PHYS] = ARCH_TIMER_S_EL2_IRQ,
            [GTIMER_S_EL2_VIRT] = ARCH_TIMER_S_EL2_VIRT_IRQ 
        };

        for (irq = 0; irq < ARRAY_SIZE(timer_irq); irq++) {
            qdev_connect_gpio_out(cpu, irq, qdev_get_gpio_in(m->gic, intid_base + timer_irq[irq]));
        }

        qdev_connect_gpio_out_named(cpu, "gicv3-maintenance-interrupt", 0, qdev_get_gpio_in(m->gic, intid_base + ARCH_GIC_MAINT_IRQ));
        qdev_connect_gpio_out_named(cpu, "pmu-interrupt", 0, qdev_get_gpio_in(m->gic, intid_base + VIRTUAL_PMU_IRQ));

        sysbus_connect_irq(m->gic_bus, i, qdev_get_gpio_in(cpu, ARM_CPU_IRQ));
        sysbus_connect_irq(m->gic_bus, i + num_smp, qdev_get_gpio_in(cpu, ARM_CPU_FIQ));
        sysbus_connect_irq(m->gic_bus, i + 2 * num_smp, qdev_get_gpio_in(cpu, ARM_CPU_VIRQ));
        sysbus_connect_irq(m->gic_bus, i + 3 * num_smp, qdev_get_gpio_in(cpu, ARM_CPU_VFIQ));
    }
}

static void helix_uart_init(MachineState *ms) {
    HelixMachineState *m = HELIX_MACHINE(ms);

    m->pl011 = qdev_new("pl011");
    m->pl011_bus = SYS_BUS_DEVICE(m->pl011);



    qdev_prop_set_chr(m->pl011, "chardev", serial_hd(0));
    sysbus_realize_and_unref(m->pl011_bus, &error_fatal);

    sysbus_mmio_map(m->pl011_bus, 0, helix_memmap[HELIX_UART0].base);

    sysbus_connect_irq(m->pl011_bus, 0, qdev_get_gpio_in(DEVICE(m->gic), 32));
}

static void helix_cpu_init(MachineState *ms) {
    HelixMachineState *m = HELIX_MACHINE(ms);
    Object *cpu_obj;

    cpu_obj = object_new(ARM_CPU_TYPE_NAME("cortex-a57"));
    m->cpu = ARM_CPU(cpu_obj);

    // CPU starts in 64-bit mode by default
    object_property_set_bool(cpu_obj, "aarch64", true, &error_fatal);

    // Set reset vector base address to point to ROM code
    object_property_set_uint(cpu_obj, "rvbar", helix_memmap[HELIX_ROM].base, &error_fatal);

    // Ensure all exception levels are enabled by default
    object_property_set_bool(cpu_obj, "has_el3", true, &error_fatal);
    object_property_set_bool(cpu_obj, "has_el2", true, &error_fatal);

    // Set timer freqency
    object_property_set_int(cpu_obj, "cntfrq", 62500000, &error_fatal);

    // Realize the CPU
    qdev_realize(DEVICE(cpu_obj), NULL, &error_fatal);

    // Ensure CPU can view HelixBoard's internal memory
    m->cs = CPU(cpu_obj);
    cpu_address_space_init(m->cs, 0, "cpu-memory", m->sysmem);
}

static void helix_flash_init(MachineState *ms) {
    HelixMachineState *m = HELIX_MACHINE(ms);
    DriveInfo *dinfo;
    DeviceState *flash;

    flash = qdev_new("cfi.pflash01");

    qdev_prop_set_uint64(flash, "sector-length", 64 * KiB);
    qdev_prop_set_uint64(flash, "num-blocks", 1);
    qdev_prop_set_uint32(flash, "width", 1);
    qdev_prop_set_string(flash, "name", "helix.bootrom");

    dinfo = drive_get(IF_PFLASH, 0, 0);
    qdev_prop_set_drive_err(flash, "drive", blk_by_legacy_dinfo(dinfo), &error_fatal);

    sysbus_realize_and_unref(SYS_BUS_DEVICE(flash), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(flash), 0, helix_memmap[HELIX_ROM].base);

    m->bootrom = PFLASH_CFI01(flash);
}

static void helix_sdhci_init(MachineState *ms) {
    HelixMachineState *m = HELIX_MACHINE(ms);
    DriveInfo *dinfo = drive_get(IF_SD, 0, 0);



    object_initialize_child(OBJECT(m), "sdhost", &m->sdhost, TYPE_CADENCE_SDHCI);

    sysbus_realize(SYS_BUS_DEVICE(&m->sdhost), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(&m->sdhost), 0, helix_memmap[HELIX_EMMC].base);

    sysbus_connect_irq(SYS_BUS_DEVICE(&m->sdhost), 0, qdev_get_gpio_in(DEVICE(m->gic), 61));

    CadenceSDHCIState *sdhci = &(m->sdhost);
    DeviceState *mmc = qdev_new(TYPE_SD_CARD);

    qdev_prop_set_drive_err(mmc, "drive", blk_by_legacy_dinfo(dinfo), &error_fatal);
    qdev_realize_and_unref(mmc, sdhci->bus, &error_fatal);
}

static void helix_board_init(MachineState *ms) {
    HelixMachineState *m = HELIX_MACHINE(ms);

    m->sysmem = get_system_memory();

    // Initialize SRAM
    MemoryRegion *sram = g_new(MemoryRegion, 1);
    memory_region_init_ram(sram, NULL, "helix.sram", helix_memmap[HELIX_SRAM].size, &error_fatal);
    memory_region_add_subregion(m->sysmem, helix_memmap[HELIX_SRAM].base, sram);

    MemoryRegion *dram = g_new(MemoryRegion, 1);
    memory_region_init_ram(dram, NULL, "helix.dram", helix_memmap[HELIX_DRAM].size, &error_fatal);
    memory_region_add_subregion(m->sysmem, helix_memmap[HELIX_DRAM].base, dram);

    // Initialize devices
    helix_cpu_init(ms);
    helix_gic_init(ms);

    helix_uart_init(ms);
    helix_flash_init(ms);
    helix_sdhci_init(ms);
}

static void helix_board_class_init(ObjectClass *oc, const void *data) {
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "Custom ARM64 Helix virtual board";
    mc->init = helix_board_init;

    // Only one A53 CPU supported
    mc->max_cpus = 1;
    mc->default_cpu_type = ARM_CPU_TYPE_NAME("cortex-a57");
    
    // 512MB default memory
    mc->default_ram_size = HELIX_DEFAULT_DRAM_SIZE;
}

static void helix_board_instance_init(Object *obj) {
}

static const TypeInfo helix_board_type_info = {
    .name           = TYPE_HELIX_MACHINE,
    .parent         = TYPE_MACHINE,
    .class_init     = helix_board_class_init,
    .instance_init  = helix_board_instance_init,
    .instance_size  = sizeof(HelixMachineState),
    .interfaces     = arm_aarch64_machine_interfaces
};

static void helix_register_types(void) {
    type_register_static(&helix_board_type_info);
}

type_init(helix_register_types)