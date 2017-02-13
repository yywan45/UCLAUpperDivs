#include "headers.h"

void my_first_registered_exit() {
  printf("here is the first function registered with atexit().\n");
}

void my_second_registered_exit() {
  printf("here is the second function registered with atexit().\n");
}

// this is second thread because the real first thread is main

void *first_thread(void *arg) {
  printf("first thread starts running, going to sleep 1 second.\n");
  sleep(1);
  return (void *)111;
}

void *second_thread(void *arg) {
  printf("second thread starts running, going to sleep %d seconds.\n", (int)arg);
  sleep((int)arg);
  pthread_exit((void *)112);
}

int main(int argc, char *argv[]) {
  printf("registering first function with atexit().\n");
  atexit(my_first_registered_exit);
  printf("registering second function with atexit().\n");
  atexit(my_second_registered_exit);

  pthread_t thread_1_id, thread_2_id;

  printf("main thread starts to run two threads.\n");
  pthread_create(&thread_1_id, NULL, first_thread, NULL);
  pthread_create(&thread_2_id, NULL, second_thread, (void *)2);

  void *thread_1_retval, *thread_2_retval;
  pthread_join(thread_1_id, &thread_1_retval);
  pthread_join(thread_2_id, &thread_2_retval);

  printf("first thread's exit status: %d\n", (int)thread_1_retval);
  printf("second thread's exit status: %d\n", (int)thread_2_retval);

  return 0;

}
