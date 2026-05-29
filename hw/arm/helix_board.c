#include "qemu/osdep.h"
#include "hw/arm/helix.h"
#include "qemu/qemu-print.h"
#include "qemu/typedefs.h"
#include "qemu/units.h"
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














static const MemMapEntry helix_memmap[] = {
    [HELIX_ROM]     = {HELIX_ROM_BASE, HELIX_ROM_SIZE},
    [HELIX_SRAM]    = {HELIX_SRAM_BASE, HELIX_SRAM_SIZE},
    [HELIX_UART0]   = {HELIX_UART_BASE, HELIX_UART_SIZE},
    [HELIX_EMMC]    = {HELIX_EMMC_BASE, HELIX_EMMC_SIZE},
    [HELIX_DRAM]    = {HELIX_DRAM_BASE, HELIX_DRAM_SIZE}
};



static char *helix_get_bootrom_path(Object *obj, Error **err) {
    HelixMachineState *ms = HELIX_MACHINE(obj);

    if (!ms->bootrom_path) {
        qemu_printf("E: BootROM path not specified!\n");
        exit(-1);
    }

    return g_strdup(ms->bootrom_path);
}

static void helix_set_bootrom_path(Object *obj, const char *val, Error **err) {
    HelixMachineState *ms = HELIX_MACHINE(obj);

    if (ms->bootrom_path) {
        g_free(ms->bootrom_path);
    }

    ms->bootrom_path = g_strdup(val);
}

static char *helix_get_emmc_path(Object *obj, Error **err) {
    HelixMachineState *ms = HELIX_MACHINE(obj);

    if (!ms->emmc_path) {
        qemu_printf("E: EMMC path not specified!\n");
        exit(-1);
    }

    return g_strdup(ms->emmc_path);
}

static void helix_set_emmc_path(Object *obj, const char *val, Error **err) {
    HelixMachineState *ms = HELIX_MACHINE(obj);

    if (ms->emmc_path) {
        g_free(ms->emmc_path);
    }

    ms->emmc_path = g_strdup(val);
}

static void helix_board_init(MachineState *ms) {
    HelixMachineState *m = HELIX_MACHINE(ms);
    Object *cpu_obj;

    cpu_obj = object_new(ARM_CPU_TYPE_NAME("cortex-a53"));
    m->cpu = ARM_CPU(cpu_obj);

    // CPU starts in 64-bit mode by default
    object_property_set_bool(cpu_obj, "aarch64", true, &error_fatal);

    // Set reset vector base address to point to ROM code
    object_property_set_uint(cpu_obj, "rvbar", helix_memmap[HELIX_ROM].base, &error_fatal);

    // For now, the CPU will only have EL1 (TODO: Support all exception levels)
    object_property_set_bool(cpu_obj, "has_el3", false, &error_fatal);
    object_property_set_bool(cpu_obj, "has_el2", false, &error_fatal);

    // Realize the CPU
    qdev_realize(DEVICE(cpu_obj), NULL, &error_fatal);

    m->sysmem = get_system_memory();

    MemoryRegion *bootrom = g_new(MemoryRegion, 1);
    memory_region_init_rom(bootrom, NULL, "helix.bootrom", helix_memmap[HELIX_ROM].size, &error_fatal);
    memory_region_add_subregion(m->sysmem, helix_memmap[HELIX_ROM].base, bootrom);

    load_image_targphys(m->bootrom_path, HELIX_ROM_BASE, HELIX_ROM_SIZE, &error_fatal);
}

static void helix_board_class_init(ObjectClass *oc, const void *data) {
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "Custom ARM64 Helix virtual board";
    mc->init = helix_board_init;

    mc->default_cpu_type = ARM_CPU_TYPE_NAME("cortex-a53");

    // Only one A53 CPU supported
    mc->max_cpus = 1;
    mc->default_cpu_type = ARM_CPU_TYPE_NAME("cortex-a53");
    
    // 512MB default memory
    mc->default_ram_size = HELIX_DEFAULT_DRAM_SIZE;
}

static void helix_board_instance_init(Object *obj) {
    HelixMachineState *ms = HELIX_MACHINE(obj);

    object_property_add_str(obj, "bootrom", helix_get_bootrom_path, helix_set_bootrom_path);
    object_property_set_description(obj, "bootrom", "Path to Helix BootROM");

    object_property_add_str(obj, "emmc", helix_get_emmc_path, helix_set_emmc_path);
    object_property_set_description(obj, "emmc", "Path to Helix EMMC image");
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