#pragma once

#include "logic.h"

#undef VIRT_EVDEV_DEBUG


// Emulates a "Generic" controller:
#define OUTPUT_DEV_NAME             "ROGueENEMY"
#define OUTPUT_DEV_VENDOR_ID        0x108c
#define OUTPUT_DEV_PRODUCT_ID       0x0323
#define OUTPUT_DEV_VERSION          0x0111

/*
// Emulates a steam controller
#define OUTPUT_DEV_NAME             "Steam Controller"
#define OUTPUT_DEV_VENDOR_ID        0x28de
#define OUTPUT_DEV_PRODUCT_ID       0x1102
#define OUTPUT_DEV_VERSION          0x0111
#define OUTPUT_DEV_BUS_TYPE         BUS_USB
*/

/*
//Emulates an Xbox one wireless controller:
#define OUTPUT_DEV_NAME             "Xbox Wireless Controller"
#define OUTPUT_DEV_VENDOR_ID        0x045e
#define OUTPUT_DEV_PRODUCT_ID       0x028e
#define OUTPUT_DEV_BUS_TYPE         BUS_BLUETOOTH
*/

/*
// Emulates a DualShock controller
#define OUTPUT_DEV_NAME             "Sony Interactive Entertainment DualSense Wireless Controller"
#define OUTPUT_DEV_VENDOR_ID        0x054c
#define OUTPUT_DEV_PRODUCT_ID       0x0ce6
#define OUTPUT_DEV_VERSION          0x8111
#define OUTPUT_DEV_BUS_TYPE         BUS_USB
*/

#define PHYS_STR "00:11:22:33:44:55"

#define ACCEL_RANGE 512
#define GYRO_RANGE  2000 // max range is +/- 35 radian/s

#define GYRO_DEADZONE 1 // degrees/s to count as zero movement

void *virt_evdev_thread_func(void *ptr);