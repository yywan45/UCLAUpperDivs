#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <pthread.h>

static char test_name[10] = "add-none";
static long long num_threads = 1;
static long long num_iterations = 1;
static long long counter = 0;

static int opt_yield = 0;
void (*lock_type)(long long*, long long);

static pthread_mutex_t mutexlock = PTHREAD_MUTEX_INITIALIZER;
static volatile int spinlock;

/* Use as base function pointer to handle different function calls */

void add(long long *pointer, long long value)
{
  long long sum = *pointer + value;
  if (opt_yield)
    if (pthread_yield()) {
      fprintf(stderr, "There was an error in yielding the thread.\n");
      exit(1);
    }
  *pointer = sum;
}

void mutex_add(long long *pointer, long long value)
{
  if (pthread_mutex_lock(&mutexlock) != 0)
  {
    fprintf(stderr, "There was an error in locking the mutex.\n");
    exit(1);
  }
  add(pointer, value);
  if (pthread_mutex_unlock(&mutexlock) != 0)
  {
    fprintf(stderr, "There was an error in unlocking the mutex.\n");
    exit(1);
  }
}

void spin_add(long long *pointer, long long value)
{
  while (__sync_lock_test_and_set(&spinlock, 1));
  add(pointer, value);
  __sync_lock_release(&spinlock);
}

void compare_and_swap_add(long long *pointer, long long value)
{
  long long prev;
  long long sum;
  do
  {
    prev = *pointer;
    if (opt_yield)
      if (pthread_yield()) {
        fprintf(stderr, "There was an error in yielding the thread.\n");
        exit(1);
      }
    sum = prev + value;
  }
  while (__sync_val_compare_and_swap(pointer, prev, sum) != prev);
}

void thread_add(void *arg)
{
  int i;

  // add 1
  for (i = 0; i < num_iterations; i++)
      (*lock_type)(&counter, 1);

  // subtract 1
  for (i = 0; i < num_iterations; i++)
      (*lock_type)(&counter, -1);
}

void atomic(void)
{

  // start time
  struct timespec time_start;
  clock_gettime(CLOCK_MONOTONIC, &time_start); 

  // create threads
  pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));

  int i;

  for (i = 0; i < num_threads; i++)
    if (pthread_create(&threads[i], NULL, (void*)thread_add, NULL)) {
    fprintf(stderr, "Error in creating threads.\n");
    exit(1);
    }

  for (i = 0; i < num_threads; i++)
    if (pthread_join(threads[i], NULL)) {
      fprintf(stderr, "Error in joining threads.\n");
      exit(1);
    }

  // end time
  struct timespec time_end;
  clock_gettime(CLOCK_MONOTONIC, &time_end);     
  
  // output
  long long num_operations = num_threads * num_iterations * 2;
  long long start_time = (long long)(time_start.tv_sec * 1000000000 + time_start.tv_nsec);
  long long end_time = (long long)(time_end.tv_sec * 1000000000 + time_end.tv_nsec);
  long long run_time = end_time - start_time;
  long long time_per_op = run_time / num_operations;

  printf("%s,%lld,%lld,%lld,%lld,%lld,%lld\n", test_name, num_threads, num_iterations, num_operations, run_time, time_per_op, counter);
  free(threads);

}

int main(int argc, char** argv)
{
  int opt = 0;
  int option_index = 0;
  lock_type = &add;

  static struct option long_options[] = {
    { "threads", required_argument, 0, 't' },
    { "iterations", required_argument, 0, 'i' },
    { "yield", no_argument, 0, 'y' },
    { "sync", required_argument, 0, 's' }
  };

  while ((opt = getopt_long(argc, argv, "t:i:ys:", long_options, &option_index)) != -1)
  {
    switch (opt)
    {
      case 't':
        num_threads = atoi(optarg);
        break;
      case 'i':
        num_iterations = atoi(optarg);
        break;
      case 'y':
        opt_yield = 1;
        strcpy(test_name,"add-yield-none");
        break;
      case 's':
        if (*optarg == 'm') {
          strcpy(test_name,"add-yield-m");
          lock_type = &mutex_add;
        }
        else if (*optarg == 's') {
          strcpy(test_name,"add-yield-s");
          lock_type = &spin_add;
        }
        else if (*optarg == 'c') {
          strcpy(test_name,"add-yield-c");
          lock_type = &compare_and_swap_add;
        }
        else {
          fprintf(stderr, "ERROR: %c is invalid argument for sync option.\n", *optarg);
          exit(1);
        }

        break;
      default:
        abort();
    }
  }

  atomic();

  return 0;
}