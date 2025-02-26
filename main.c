#include <signal.h>
#include <stdlib.h>
#include <stdlib.h>

#include "input_dev.h"
#include "output_dev.h"
#include "logic.h"

logic_t global_logic;

static output_dev_t out_gamepadd_dev = {
  .gamepad_fd = -1,
  .imu_fd = -1,
  .logic = &global_logic,
};

static iio_filters_t in_iio_filters = {
  // .name = "accel_3d",
  .name = "gyro_3d",
};

static input_dev_t in_iio_dev = {
  .dev_type = input_dev_type_iio,
  .iio_filters = &in_iio_filters,
  .logic = &global_logic,
  //.input_filter_fn = input_filter_imu_identity,
};
static input_dev_t in_hidraw_dev = {
  .dev_type = input_dev_type_hidraw,
  .logic = &global_logic
};

static uinput_filters_t in_asus_kb_1_filters = {
  .name = "Asus Keyboard",
};

static input_dev_t in_asus_kb_1_dev = {
  .dev_type = input_dev_type_uinput,
  .ev_filters = &in_asus_kb_1_filters,
  .logic = &global_logic,
  .ev_input_filter_fn = input_filter_asus_kb,
};

static uinput_filters_t in_asus_kb_2_filters = {
  .name = "Asus Keyboard",
};

static input_dev_t in_asus_kb_2_dev = {
  .dev_type = input_dev_type_uinput,
  .ev_filters = &in_asus_kb_2_filters,
  .logic = &global_logic,
  .ev_input_filter_fn = input_filter_asus_kb,
};

static uinput_filters_t in_asus_kb_3_filters = {
  .name = "Asus Keyboard",
};

static input_dev_t in_asus_kb_3_dev = {
  .dev_type = input_dev_type_uinput,
  .ev_filters = &in_asus_kb_3_filters,
  .logic = &global_logic,
  .ev_input_filter_fn = input_filter_asus_kb,
};

static uinput_filters_t in_xbox_filters = {
  .name = "Generic X-Box pad",
};

static input_dev_t in_xbox_dev = {
  .dev_type = input_dev_type_uinput,
  .ev_filters = &in_xbox_filters,
  .logic = &global_logic,
  .ev_input_filter_fn = input_filter_identity,
};

// void sig_handler(int signo)
// {
//   if (signo == SIGINT) {
//     logic_request_termination(&global_logic);
//     printf("received SIGINT\n");
//   }
// }

int main(int argc, char ** argv) {
  const int logic_creation_res = logic_create(&global_logic);
  if (logic_creation_res < 0) {
    fprintf(stderr, "Unable to create logic: %d", logic_creation_res);
    return EXIT_FAILURE;
  }

  int imu_fd = create_output_dev("/dev/uinput", output_dev_imu);
  if (imu_fd < 0) {
    fprintf(stderr, "Unable to create IMU virtual device\n");
    return EXIT_FAILURE;
  }

  int gamepad_fd = create_output_dev("/dev/uinput", output_dev_gamepad);
  if (gamepad_fd < 0) {
    close(imu_fd);
    fprintf(stderr, "Unable to create gamepad virtual device\n");
    return EXIT_FAILURE;
  }

  int mouse_fd = create_output_dev("/dev/uinput", output_dev_mouse);
  if (mouse_fd < 0) {
    close(gamepad_fd);
    close(imu_fd);
    fprintf(stderr, "Unable to create mouse virtual device\n");
    return EXIT_FAILURE;
  }

  out_gamepadd_dev.gamepad_fd = gamepad_fd;
  out_gamepadd_dev.imu_fd = imu_fd;
  out_gamepadd_dev.mouse_fd = mouse_fd;

/*
  __sighandler_t sigint_hndl = signal(SIGINT, sig_handler); 
  if (sigint_hndl == SIG_ERR) {
    fprintf(stderr, "Error registering SIGINT handler\n");
    return EXIT_FAILURE;
  }
*/
  signal(SIGTERM, sigterm_handler);

  int ret = 0;

  pthread_t gamepad_thread;
  pthread_t xbox_thread, asus_kb_1_thread, asus_kb_2_thread, asus_kb_3_thread, iio_thread, hidraw_thread;
  
  
  //Added 1ms intial "input delay" to match hardware controller delay. Updated to 1.5ms, seems good so far. 
  //Reduced cpu usage by ~12%
  const int gamepad_thread_creation = pthread_create(&gamepad_thread, NULL, output_dev_thread_func, (void*)(&out_gamepadd_dev));
  if (gamepad_thread_creation != 0) {
    fprintf(stderr, "Error creating gamepad output thread: %d\n", gamepad_thread_creation);
    ret = -1;
    logic_request_termination(&global_logic);
    goto gamepad_thread_err;
  }
  //Using 30% cpu usage for some reason
  //Updated sleep to 5K -> reduced cpu usage by ~20%
  const int xbox_thread_creation = pthread_create(&xbox_thread, NULL, input_dev_thread_func, (void*)(&in_xbox_dev));
  if (xbox_thread_creation != 0) {
    fprintf(stderr, "Error creating xbox input thread: %d\n", xbox_thread_creation);
    ret = -1;
    logic_request_termination(&global_logic);
    goto xbox_drv_thread_err;
  }
  /* Disabling had no effect on CPU usage avg
  // const int asus_kb_1_thread_creation = pthread_create(&asus_kb_1_thread, NULL, input_dev_thread_func, (void*)(&in_asus_kb_1_dev));
  // if (asus_kb_1_thread_creation != 0) {
  //   fprintf(stderr, "Error creating asus keyboard (1) input thread: %d\n", asus_kb_1_thread_creation);
  //   ret = -1;
  //   logic_request_termination(&global_logic);
  //   goto asus_kb_1_thread_err;
  // }

  // const int asus_kb_2_thread_creation = pthread_create(&asus_kb_2_thread, NULL, input_dev_thread_func, (void*)(&in_asus_kb_2_dev));
  // if (asus_kb_2_thread_creation != 0) {
  //   fprintf(stderr, "Error creating asus keyboard (2) input thread: %d\n", asus_kb_2_thread_creation);
  //   ret = -1;
  //   logic_request_termination(&global_logic);
  //   goto asus_kb_2_thread_err;
  // }

  // const int asus_kb_3_thread_creation = pthread_create(&asus_kb_3_thread, NULL, input_dev_thread_func, (void*)(&in_asus_kb_3_dev));
  // if (asus_kb_3_thread_creation != 0) {
  //   fprintf(stderr, "Error creating asus keyboard (3) input thread: %d\n", asus_kb_3_thread_creation);
  //   ret = -1;
  //   logic_request_termination(&global_logic);
  //   goto asus_kb_3_thread_err;
  // }
  */

  const int iio_thread_creation = pthread_create(&iio_thread, NULL, input_dev_thread_func, (void*)(&in_iio_dev));
  if (iio_thread_creation != 0) {
    fprintf(stderr, "Error creating iio input thread: %d\n", iio_thread_creation);
    ret = -1;
    logic_request_termination(&global_logic);
    goto iio_thread_err;
  }
  // Disabling had no signification effect avg CPU usage
  const int hidraw_thread_creation = pthread_create(&hidraw_thread, NULL, input_dev_thread_func, (void*)(&in_hidraw_dev));
  if (hidraw_thread_creation != 0) {
    fprintf(stderr, "Error creating iio input thread: %d\n", hidraw_thread_creation);
    ret = -1;
    logic_request_termination(&global_logic);
    goto hidraw_thread_err;
  }
 
  pthread_join(iio_thread, NULL);

hidraw_thread_err:
  pthread_join(hidraw_thread, NULL);

iio_thread_err:
  pthread_join(asus_kb_3_thread, NULL);

asus_kb_3_thread_err:
  pthread_join(asus_kb_2_thread, NULL);

asus_kb_2_thread_err:
  pthread_join(asus_kb_1_thread, NULL);

asus_kb_1_thread_err:
  pthread_join(xbox_thread, NULL);

xbox_drv_thread_err:
  pthread_join(gamepad_thread, NULL);

gamepad_thread_err:
  ioctl(gamepad_fd, UI_DEV_DESTROY);
  close(gamepad_fd);
  
  // TODO: free(imu_dev.events_list);
  // TODO: free(gamepadd_dev.events_list);

  return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
