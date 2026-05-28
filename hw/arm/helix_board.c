#include "qemu/osdep.h"
#include "hw/arm/helix.h"
#include "qom/object.h"
#include "hw/core/boards.h"
#include "hw/arm/boot.h"
#include "hw/arm/machines-qom.h"










static void helix_board_init(MachineState *ms) {
}

static void helix_board_class_init(ObjectClass *oc, const void *data) {
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "Custom ARM64 Helix virtual board";
    mc->init = helix_board_init;
    mc->default_cpu_type = ARM_CPU_TYPE_NAME("cortex-a53");
}

static const TypeInfo helix_board_type_info = {
    .name       = MACHINE_TYPE_NAME("helix"),
    .parent     = TYPE_MACHINE,
    .class_init = helix_board_class_init,
    .interfaces = arm_aarch64_machine_interfaces
};

static void helix_register_types(void) {
    type_register_static(&helix_board_type_info);
}

type_init(helix_register_types)