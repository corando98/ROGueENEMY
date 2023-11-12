#pragma once

#include "queue.h"
#include "message.h"

#undef INCLUDE_INPUT_DEBUG
#undef IGNORE_INPUT_SCAN

#define INPUT_DEV_CTRL_FLAG_EXIT 0x00000001U

typedef enum input_dev_type {
    input_dev_type_uinput,
    input_dev_type_iio,
} input_dev_type_t;

typedef struct uinput_filters {
    const char* name;
} uinput_filters_t;

typedef struct iio_filters {
    const char* name;
} iio_filters_t;

typedef struct input_dev {
    input_dev_type_t dev_type;

    const uinput_filters_t* ev_filters;
    const iio_filters_t* iio_filters;

    volatile uint32_t crtl_flags;

    queue_t *queue;

    input_filter_t input_filter_fn;
} input_dev_t;

void *input_dev_thread_func(void *ptr);

int open_and_hide_input();

int input_filter_identity(struct input_event* events, size_t* size, uint32_t* count);

int input_filter_asus_kb(struct input_event*, size_t*, uint32_t*);