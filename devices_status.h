#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct gamepad_status {
    bool connected;

    int32_t joystick_positions[2][2]; // [0 left | 1 right][x axis | y axis]

    uint8_t dpad; // 0x00 x - | 0x01 x -> | 0x02 x <- | 0x00 y - | 0x10 y ^ | 0x10 y . | 

    uint8_t l2_trigger;
    uint8_t r2_trigger;

    uint8_t triangle;
    uint8_t circle;
    uint8_t cross;
    uint8_t square;

    uint8_t l1;
    uint8_t r1;

    uint8_t r3;
    uint8_t l3;

    uint8_t option;
    uint8_t share;
    uint8_t center;

    uint8_t l4;
    uint8_t r4;
    
    uint8_t l5;
    uint8_t r5;

    struct timeval last_gyro_motion_time;
    struct timeval last_accel_motion_time;

    double gyro[3]; // | x, y, z| right-hand-rules -- in rad/s
    double accel[3]; // | x, y, z| positive: right, up, towards player -- in m/s^2

    int16_t raw_gyro[3];
    int16_t raw_accel[3];

    uint64_t rumble_events_count;
    uint8_t motors_intensity[2]; // 0 = left, 1 = right

    volatile uint32_t flags;

} gamepad_status_t;

typedef struct keyboard_status {
    bool connected;
} keyboard_status_t;

typedef struct devices_status {
    // this mutex MUST be grabbed when reading and/or writing below properties
    pthread_mutex_t mutex;

    gamepad_status_t gamepad;

    keyboard_status_t kbd;

} devices_status_t;

void kbd_status_init(keyboard_status_t *const stats);

void gamepad_status_init(gamepad_status_t *const stats);

void devices_status_init(devices_status_t *const stats);