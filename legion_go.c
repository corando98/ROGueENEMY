#include "legion_go.h"
#include "input_dev.h"
#include "dev_hidraw.h"
#include "message.h"
#include "xbox360.h" // for xbox360_ev_map
#include "input_dev.h"

// Custom mapping for the xbox360 controller for the legion go
void xbox360_ev_map_custom(const evdev_collected_t *const coll, int in_messages_pipe_fd, void* user_data) {
	const xbox360_settings_t *const settings = (xbox360_settings_t*)user_data;

    for (uint32_t i = 0; i < coll->ev_count; ++i) {
		if (coll->ev[i].type == EV_KEY) {
			in_message_t current_message = {
				.type = GAMEPAD_SET_ELEMENT
			};

			if (coll->ev[i].code == BTN_EAST) {
				current_message.data.gamepad_set.element = (settings->nintendo_layout) ? GAMEPAD_BTN_CROSS : GAMEPAD_BTN_CIRCLE;
				current_message.data.gamepad_set.status.btn = coll->ev[i].value;
			} else if (coll->ev[i].code == BTN_NORTH) {
				current_message.data.gamepad_set.element = (settings->nintendo_layout) ? GAMEPAD_BTN_TRIANGLE : GAMEPAD_BTN_SQUARE;
				current_message.data.gamepad_set.status.btn = coll->ev[i].value;
			} else if (coll->ev[i].code == BTN_SOUTH) {
				current_message.data.gamepad_set.element = (settings->nintendo_layout) ? GAMEPAD_BTN_CIRCLE : GAMEPAD_BTN_CROSS;
				current_message.data.gamepad_set.status.btn = coll->ev[i].value;
			} else if (coll->ev[i].code == BTN_WEST) {
				current_message.data.gamepad_set.element = (settings->nintendo_layout) ? GAMEPAD_BTN_SQUARE : GAMEPAD_BTN_TRIANGLE;
				current_message.data.gamepad_set.status.btn = coll->ev[i].value;
			// } else if (coll->ev[i].code == BTN_SELECT) {
			// 	current_message.data.gamepad_set.element = GAMEPAD_BTN_OPTION;
			// 	current_message.data.gamepad_set.status.btn = coll->ev[i].value;
			// } else if (coll->ev[i].code == BTN_START) {
			// 	current_message.data.gamepad_set.element = GAMEPAD_BTN_SHARE;
			// 	current_message.data.gamepad_set.status.btn = coll->ev[i].value;
			} else if (coll->ev[i].code == BTN_TR) {
				current_message.data.gamepad_set.element = GAMEPAD_BTN_R1;
				current_message.data.gamepad_set.status.btn = coll->ev[i].value;
			} else if (coll->ev[i].code == BTN_TL) {
				current_message.data.gamepad_set.element = GAMEPAD_BTN_L1;
				current_message.data.gamepad_set.status.btn = coll->ev[i].value;
			} else if (coll->ev[i].code == BTN_THUMBR) {
				current_message.data.gamepad_set.element = GAMEPAD_BTN_R3;
				current_message.data.gamepad_set.status.btn = coll->ev[i].value;
			} else if (coll->ev[i].code == BTN_THUMBL) {
				current_message.data.gamepad_set.element = GAMEPAD_BTN_L3;
				current_message.data.gamepad_set.status.btn = coll->ev[i].value;
			} else if (coll->ev[i].code == BTN_MODE) {
				current_message.type = GAMEPAD_ACTION;
				current_message.data.action = GAMEPAD_ACTION_PRESS_AND_RELEASE_CENTER;
			}

			// send the button event over the pipe
			if (write(in_messages_pipe_fd, (void*)&current_message, sizeof(in_message_t)) != sizeof(in_message_t)) {
				fprintf(stderr, "Unable to write gamepad data to the in_message pipe\n");
			}
		} else if (coll->ev[i].type == EV_ABS) {
			in_message_t current_message = {
				.type = GAMEPAD_SET_ELEMENT,
			};
			
			if (coll->ev[i].code == ABS_X) {
				current_message.data.gamepad_set.element = GAMEPAD_LEFT_JOYSTICK_X;
				current_message.data.gamepad_set.status.joystick_pos = (int32_t)coll->ev[i].value;
			} else if (coll->ev[i].code == ABS_Y) {
				current_message.data.gamepad_set.element = GAMEPAD_LEFT_JOYSTICK_Y;
				current_message.data.gamepad_set.status.joystick_pos = (int32_t)coll->ev[i].value;
			} else if (coll->ev[i].code == ABS_RX) {
				current_message.data.gamepad_set.element = GAMEPAD_RIGHT_JOYSTICK_X;
				current_message.data.gamepad_set.status.joystick_pos = (int32_t)coll->ev[i].value;
			} else if (coll->ev[i].code == ABS_RY) {
				current_message.data.gamepad_set.element = GAMEPAD_RIGHT_JOYSTICK_Y;
				current_message.data.gamepad_set.status.joystick_pos = (int32_t)coll->ev[i].value;
			} else if (coll->ev[i].code == ABS_Z) {
				current_message.data.gamepad_set.element = GAMEPAD_BTN_L2_TRIGGER;
				current_message.data.gamepad_set.status.btn = coll->ev[i].value;
			} else if (coll->ev[i].code == ABS_RZ) {
				current_message.data.gamepad_set.element = GAMEPAD_BTN_R2_TRIGGER;
				current_message.data.gamepad_set.status.btn = coll->ev[i].value;
			} else if (coll->ev[i].code == ABS_HAT0X) {
				current_message.data.gamepad_set.element = GAMEPAD_DPAD_X;
				current_message.data.gamepad_set.status.btn = coll->ev[i].value;
			} else if (coll->ev[i].code == ABS_HAT0Y) {
				current_message.data.gamepad_set.element = GAMEPAD_DPAD_Y;
				current_message.data.gamepad_set.status.btn = coll->ev[i].value;
			}

			// send the button event over the pipe
			if (write(in_messages_pipe_fd, (void*)&current_message, sizeof(in_message_t)) != sizeof(in_message_t)) {
				fprintf(stderr, "Unable to write gamepad data to the in_message pipe\n");
			}
		}
	}
}


static input_dev_t in_xbox_dev = {
    .dev_type = input_dev_type_uinput,
    .filters = {
        .ev = {
          .name = "Generic X-Box pad"
        }
    },
    .map = {
        .ev_input_map_fn = xbox360_ev_map_custom,
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

void send_gamepad_message(int in_messages_pipe_fd, int element, int status) {
    const in_message_t current_message = {
        .type = GAMEPAD_SET_ELEMENT,
        .data = {
            .gamepad_set = {
                .element = element,
                .status = {
                    .btn = status,
                }
            }
        }
    };
    
    const ssize_t in_message_pipe_write_res = write(in_messages_pipe_fd, (void*)&current_message, sizeof(in_message_t));
    if (in_message_pipe_write_res != sizeof(in_message_t)) {
        fprintf(stderr, "Unable to write data for element %d to the in_message pipe: %zu\n", element, in_message_pipe_write_res);
    }
}

static int llg_hidraw_map(int hidraw_fd, int in_messages_pipe_fd, void* user_data) {
    struct llg_hidraw_data *const llg_data = (struct llg_hidraw_data*)user_data;

    int read_res = read(hidraw_fd, llg_data->last_packet, sizeof(llg_data->last_packet));
    printf("read_res: %i\n", read_res);
    if (read_res != 64) {
        fprintf(stderr, "Error reading from hidraw device\n");
        return -EINVAL;
    }

    printf("HID: ");
    for (int i = 0; i < read_res; i++) {
        printf("%02x ", (unsigned char)llg_data->last_packet[i]);
    }
    printf("\n\n");
    printf("%02x \n", (unsigned char)llg_data->last_packet[20]);
    printf("%02x \n", (unsigned char)llg_data->last_packet[18]);
    // here we have llg_d  ata->last_packet filled with 64 bytes from the input device

    int l4_status = (llg_data->last_packet[20] & 0x80) ? 1 : 0;
    printf("L4 status: %s\n", l4_status ? "pressed" : "released");
    send_gamepad_message(in_messages_pipe_fd, GAMEPAD_BTN_L4, l4_status);

    int l5_status = (llg_data->last_packet[20] & 0x40) ? 1 : 0;
    printf("L5 status: %s\n", l5_status ? "pressed" : "released");
    send_gamepad_message(in_messages_pipe_fd, GAMEPAD_BTN_L5, l5_status);

    int r4_status = (llg_data->last_packet[20] & 0x20) ? 1 : 0;
    printf("R4 status: %s\n", r4_status ? "pressed" : "released");
    send_gamepad_message(in_messages_pipe_fd, GAMEPAD_BTN_R4, r4_status);

    int r5_status = (llg_data->last_packet[20] & 0x04) ? 1 : 0;
    printf("R5 status: %s\n", r5_status ? "pressed" : "released");
    send_gamepad_message(in_messages_pipe_fd, GAMEPAD_BTN_R5, r5_status);

    int select_status = (llg_data->last_packet[18] & 0x80) ? 1 : 0;
    printf("Select status: %s\n", select_status ? "pressed" : "released");
    send_gamepad_message(in_messages_pipe_fd, GAMEPAD_BTN_SHARE, select_status);

    int start_status = (llg_data->last_packet[18] & 0x40) ? 1 : 0;
    printf("Start status: %s\n", start_status ? "pressed" : "released");
    send_gamepad_message(in_messages_pipe_fd, GAMEPAD_BTN_OPTION, start_status);

    int ps_status = (llg_data->last_packet[20] & 0x01) ? 1 : 0;
    printf("PS status: %s\n", ps_status ? "pressed" : "released");
    send_gamepad_message(in_messages_pipe_fd, GAMEPAD_BTN_CENTER, ps_status);

    int qam_status = (llg_data->last_packet[20] & 0x02) ? 1 : 0;
    printf("QAM status: %s\n", qam_status ? "pressed" : "released");
    send_gamepad_message(in_messages_pipe_fd, GAMEPAD_BTN_QAM, qam_status);

    int r6_status = (llg_data->last_packet[20] & 0x08) ? 1 : 0;
    printf("R6 status: %s\n", r6_status ? "pressed" : "released");

    
    //send_gamepad_message(in_messages_pipe_fd, GAMEPAD_BTN_R3, r6_status);
    //Not sure what this button should be, but it's not R3


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
