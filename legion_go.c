#include "legion_go.h"
#include "input_dev.h"
#include "dev_hidraw.h"
#include "xbox360.h"

static input_dev_t in_xbox_dev = {
    .dev_type = input_dev_type_uinput,
    .filters = {
        .ev = {
          .name = "Generic X-Box pad"
        }
    },
    .map = {
        .ev_input_map_fn = xbox360_ev_map,
    }
};

static xbox360_settings_t x360_cfg;

static input_dev_t in_iio_dev = {
    .dev_type = input_dev_type_iio,
    .filters = {
        .iio = {
            .name = "gyro_3d",
        }
    },
};

static hidraw_filters_t hidraw_filters = {
    .pid = 0x6182,
    .vid = 0x17ef,
    .rdesc_size = 44,
};
static struct llg_hidraw_data {
    uint8_t last_packet[64];
} llg_hidraw_user_data;

static int llg_hidraw_map(int hidraw_fd, int in_messages_pipe_fd, void* user_data) {
    struct llg_hidraw_data *const llg_data = (struct llg_hidraw_data*)user_data;

    int read_res = read(hidraw_fd, llg_data->last_packet, sizeof(llg_data->last_packet));
    if (read_res != 64) {
        fprintf(stderr, "Error reading from hidraw device\n");
        return -EINVAL;
    }
    printf("HID: %s\n\n", llg_data->last_packet);
    // here we have llg_data->last_packet filled with 64 bytes from the input device

    if(llg_data->last_packet[24] & 0x08) {
        const in_message_t current_message = {
        .type = GAMEPAD_SET_ELEMENT,
        .data = {
            .gamepad_set = {
            .element = GAMEPAD_BTN_L4,
            .status = {
                .btn = 1,
            }
            }
        }
        };
        const ssize_t in_message_pipe_write_res = write(in_messages_pipe_fd, (void*)&current_message, sizeof(in_message_t));
        if (in_message_pipe_write_res != sizeof(in_message_t)) {
        fprintf(stderr, "Unable to write data for L4 to the in_message pipe: %zu\n", in_message_pipe_write_res);
        }
    }
    
    // this does send messages to the output device

    // successful return
    return 0;
}

static input_dev_t in_hidraw_dev = {
    .dev_type = input_dev_type_hidraw,
    .filters = {
        .hidraw = {
            .pid = 0x6182,
            .vid = 0x17ef,
            .rdesc_size = 44,
        }
    },
    .user_data = (void*)&llg_hidraw_user_data,
    .map = {
        .hidraw_input_map_fn = llg_hidraw_map,
    },
};
typedef enum legion_go_platform_mode {
    llg_platform_mode_hidraw,
} legion_go_platform_mode;

typedef struct legion_go_platform {
    int _pad;
    legion_go_platform_mode platform_mode;

    union {
        dev_hidraw_t *hidraw;
    } platform;

    unsigned long mode;
    unsigned int modes_count;
} legion_go_platform_t;

static int legion_platform_init(void** platform_data) {
    int res = -EINVAL;

    legion_go_platform_t *const llg_platform = malloc(sizeof(legion_go_platform_t));
    if (llg_platform == NULL) {
        fprintf(stderr, "Unable to allocate memory for the platform data\n");
        res = -ENOMEM;
        goto legion_platform_init_err;
    }

    *platform_data = (void*)llg_platform;
    res = dev_hidraw_open(&hidraw_filters, &llg_platform->platform.hidraw);
    if (res !=0){
        fprintf(stderr, "Unable to open the llg hidraw main device...\n");
        free(*platform_data);
        *platform_data = NULL;
        goto legion_platform_init_err;
    }
    uint8_t rdesc[256];
    size_t rdesc_sz;
    res = dev_hidraw_get_rdesc(llg_platform->platform.hidraw, rdesc, sizeof(rdesc), &rdesc_sz);
    if (res != 0) {
        fprintf(stderr, "Unable to get llg rdesc\n");
        goto legion_platform_init_err;
    }
    llg_platform->platform_mode = llg_platform_mode_hidraw;

    // const int fd = dev_hidraw_get_fd(llg_platform->platform.hidraw);
legion_platform_init_err:
    return res;
}

static void legion_platform_deinit(void** platform_data) {
      legion_go_platform_t *const platform = (legion_go_platform_t *)(*platform_data);

    if ((platform->platform_mode == llg_platform_mode_hidraw) && (platform->platform.hidraw != NULL)) {
        dev_hidraw_close(platform->platform.hidraw);
        platform->platform.hidraw = NULL;
    }
    free(*platform_data);
    *platform_data = NULL;
}

int legion_platform_leds(uint8_t r, uint8_t g, uint8_t b, void* platform_data) {
    legion_go_platform_t * const platform = (legion_go_platform_t *)platform_data;
    if(platform == NULL){
        return -EINVAL;
    }
    // :param controller: byte - The controller byte (0x03 for left, 0x04 for right)
    // :param mode: byte - The mode of the LED (e.g., 0x01 for solid, 0x02 for blinking)
    // :param color: bytes - The RGB color value (e.g., b'\xFF\x00\x00' for red)
    // :param brightness: byte - The brightness value (0x00 to 0x64)
    // :param speed: byte - The speed setting for dynamic modes (0x00 to 0x64, higher is slower)
    // :param profile: byte - The profile number
    // :param on: bool - True to turn on, False to turn off the RGB LEDs
    // :return: bytes - The command byte array
    uint8_t mode = 0x01;
    uint8_t brightness = 0x64;
    const uint8_t brightness_buf[] = { 

    };
    uint8_t colors_buf[64] = {
        0x5A, 0xB3, 0x00, mode, r, g, b, brightness, 
    };
    memset(&colors_buf[8], 0xCD, 56);  // Assuming the first 8 bytes are set as above
    
    if((platform->platform_mode == llg_platform_mode_hidraw) && (platform->platform.hidraw != NULL))
    {
        int fd = dev_hidraw_get_fd(platform->platform.hidraw);
        // if (write(fd, brightness_buf, sizeof(brightness_buf)) != 64) {
        // fprintf(stderr, "Unable to send LEDs brightness (1) command change\n");
        // goto llg_platform_leds_err;
        // }
        printf("Colors: %i", colors_buf);
        if (write(fd, colors_buf, sizeof(colors_buf)) != 64) {
        fprintf(stderr, "Unable to send LEDs color command change (1)\n");
        goto llg_platform_leds_err;
        }
        printf("changing leds\n");
    }
    return 0;
    llg_platform_leds_err:
    return -EINVAL;
}

input_dev_composite_t legion_composite = {
    .dev = {
        &in_hidraw_dev,
        &in_xbox_dev,
        // &in_iio_dev,
    },
    .dev_count = 2,
    .init_fn = legion_platform_init,
    .leds_fn = legion_platform_leds,
    .deinit_fn = legion_platform_deinit,
};


input_dev_composite_t* legion_go_device_def(const controller_settings_t *const settings) {
    x360_cfg.nintendo_layout = settings->nintendo_layout;

    in_xbox_dev.user_data = (void*)&x360_cfg;
    return &legion_composite;
}



// add properties on on devices_status.h
// and reuse the message with buttons
// then do filtering and wizardry xScale into the message read
// so that report generation is the fastest possible
