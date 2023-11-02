#include <signal.h>

#include "input_dev.h"
#include "output_dev.h"

ev_queue_t imu_ev;
ev_queue_t gamepad_ev;

output_dev_t imu_dev = {
  .fd = -1,
  .crtl_flags = 0x00000000U,
  .queue = &imu_ev,
};

output_dev_t gamepadd_dev = {
  .fd = -1,
  .crtl_flags = 0x00000000U,
  .queue = &gamepad_ev,
};

input_dev_t in_asus_kb_1_dev = {
  .crtl_flags = 0x00000000U,
};

input_dev_t in_asus_kb_2_dev = {
  .crtl_flags = 0x00000000U,
};

input_dev_t in_asus_kb_3_dev = {
  .crtl_flags = 0x00000000U,
};

input_dev_t in_asus_kb_4_dev = {
  .crtl_flags = 0x00000000U,
};

input_dev_t in_xbox_dev = {
  .crtl_flags = 0x00000000U,
};

void request_termination() {
  imu_dev.crtl_flags |= OUTPUT_DEV_CTRL_FLAG_EXIT;
  gamepadd_dev.crtl_flags |= OUTPUT_DEV_CTRL_FLAG_EXIT;
}

void sig_handler(int signo)
{
  if (signo == SIGINT) {
    request_termination();
    printf("received SIGINT\n");
  }
}

int main(int argc, char ** argv) {
  
  imu_dev.fd = create_output_dev("/dev/uinput", "Virtual IMU - ROGueENEMY", output_dev_imu);
  if (imu_dev.fd < 0) {
    // TODO: free(imu_dev.events_list);
    // TODO: free(gamepadd_dev.events_list);
    fprintf(stderr, "Unable to create IMU virtual device\n");
    return EXIT_FAILURE;
  }

  gamepadd_dev.fd = create_output_dev("/dev/uinput", "Virtual Controller - ROGueENEMY", output_dev_gamepad);
  if (gamepadd_dev.fd < 0) {
    close(imu_dev.fd);
    fprintf(stderr, "Unable to create gamepad virtual device\n");
    return EXIT_FAILURE;
  }

  __sighandler_t sigint_hndl = signal(SIGINT, sig_handler); 
  if (sigint_hndl == SIG_ERR) {
    fprintf(stderr, "Error registering SIGINT handler\n");
    return EXIT_FAILURE;
  }

  int ret = 0;

  pthread_t imu_thread, gamepad_thread;

  int imu_thread_creation = pthread_create(&imu_thread, NULL, output_dev_thread_func, (void*)(&imu_dev));
  if (imu_thread_creation != 0) {
    fprintf(stderr, "Error creating IMU output thread: %d\n", imu_thread_creation);
    ret = -1;
    request_termination();
    goto imu_thread_err;
  }
  
  int gamepad_thread_creation = pthread_create(&gamepad_thread, NULL, output_dev_thread_func, (void*)(&gamepadd_dev));
  if (gamepad_thread_creation != 0) {
    fprintf(stderr, "Error creating gamepad output thread: %d\n", gamepad_thread_creation);
    ret = -1;
    request_termination();
    goto gamepad_thread_err;
  }

  pthread_join(gamepad_thread, NULL);

gamepad_thread_err:
  pthread_join(imu_thread, NULL); 

imu_thread_err:
  if (!(gamepadd_dev.fd < 0))
    close(gamepadd_dev.fd);
  
  if (!(imu_dev.fd < 0))
    close(imu_dev.fd);
  
  // TODO: free(imu_dev.events_list);
  // TODO: free(gamepadd_dev.events_list);

  return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
