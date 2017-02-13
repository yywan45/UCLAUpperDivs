#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <pthread.h>
#include <stdint.h>
#include "SortedList.h"

static char *yield_opts = "none";
static char *sync_opts = "none";
static long long num_threads = 1;
static long long num_iterations = 1;
static long long num_elements = 0;

int opt_yield = 0;
void (*lock_type)(long long*, long long);

static pthread_mutex_t mutexlock = PTHREAD_MUTEX_INITIALIZER;
static volatile int spinlock;

static SortedList_t list;
static SortedListElement_t *elements;
// static pthread_t* threads;
// static char **keys;

/* Use as base function pointer to handle different function calls */

void lock(void)
{
  if (*sync_opts == 'm')
    pthread_mutex_lock(&mutexlock);
  else if (*sync_opts == 's')
    while (__sync_lock_test_and_set(&spinlock, 1));
}

void unlock(void)
{
  if (*sync_opts == 'm')
    pthread_mutex_unlock(&mutexlock);
  else if (*sync_opts == 's')
    __sync_lock_release(&spinlock);
}

void* thread_create(void *thread_index)
{
  int start_index = *((int*)thread_index) * num_iterations;
  SortedListElement_t *thread_start = &elements[start_index];
  int i;

  for (i = 0; i < num_iterations; i++) {
    lock();
    SortedList_insert(&list, &thread_start[i]);
    unlock();
  }

  lock();
  SortedList_length(&list);
  unlock();

  for (i = 0; i < num_iterations; i++) {
    lock();
    SortedList_delete(SortedList_lookup(&list, thread_start[i].key));
    unlock();
  }

  free(thread_index); // TODO?
  pthread_exit(NULL);
}

void timed(void)
{

  // start time
  struct timespec time_start;
  clock_gettime(CLOCK_MONOTONIC, &time_start); 

  // create threads
  pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));

  int i;

  for (i = 0; i < num_threads; i++) {
    int *temp = (int*)malloc(sizeof(int*));
    *temp = i;
    if (pthread_create(&threads[i], NULL, thread_create, (void*)temp)) {
      fprintf(stderr, "Error in creating threads.\n");
      exit(1);
    }
  }

  for (i = 0; i < num_threads; i++)
    if (pthread_join(threads[i], NULL)) {
      fprintf(stderr, "Error in joining threads.\n");
      exit(1);
    }

  // end time
  struct timespec time_end;
  clock_gettime(CLOCK_MONOTONIC, &time_end);     
  
  // check list length
  int list_length = SortedList_length(&list);
  if(list_length != 0) {
    fprintf(stderr, "ERROR: list length is %d\n", list_length);
    exit(1);
  }

  // output
  long long num_operations = num_threads * num_iterations * 3;
  long long start_time = (long long)(time_start.tv_sec * 1000000000 + time_start.tv_nsec);
  long long end_time = (long long)(time_end.tv_sec * 1000000000 + time_end.tv_nsec);
  long long run_time = end_time - start_time;
  long long time_per_op = run_time / num_operations;

  printf("list-%s-%s,%lld,%lld,1,%lld,%lld,%lld\n", yield_opts, sync_opts, num_threads, num_iterations, num_operations, run_time, time_per_op);

  for (i = 0; i < num_elements; i++)
    free((void*)elements[i].key);

  free(threads);
  free(elements);

}

static const char* random_char = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void initialize_list(void)
{
  num_elements = num_threads * num_iterations;
  // elements = malloc(num_elements * sizeof(struct SortedListElement));
  // keys = malloc(num_elements * sizeof(char*));
  // int key_length = 3;
  
  // for(int i = 0; i < num_elements; i++) {
  //   keys[i] = malloc(key_length * sizeof *keys[i]);
  //   for(int j = 0; j < key_length; j++)
  //     keys[i][j] = (char)((rand() % 93) + 33);
  //   elements[i] = malloc(sizeof(struct SortedListElement));
  //   elements[i]->next = NULL;
  //   elements[i]->prev = NULL;
  //   elements[i]->key = keys[i];
  // }

  elements = (SortedListElement_t*)malloc(sizeof(SortedListElement_t) * num_elements);

  list.prev = &list;
  list.next = &list;
  list.key  = NULL;

  srand(time(NULL));
  int key_length = 6;

  for (int i = 0; i < num_threads * num_iterations; i++)
  {
    char* temp = (char*)malloc(sizeof(char) * key_length);
    for (int j = 0; j < key_length - 1; j++)
      temp[j] = random_char[rand() % (sizeof(random_char) - 1)];
    temp[key_length - 1] = '\0';
    elements[i].key = temp;
  }
}

int main(int argc, char** argv)
{
  int opt = 0;
  int option_index = 0;

  static struct option long_options[] = {
    { "threads", required_argument, 0, 't' },
    { "iterations", required_argument, 0, 'i' },
    { "yield", required_argument, 0, 'y' },
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
        yield_opts = optarg;
        break;
      case 's':
        sync_opts = optarg;
        if (*sync_opts != 'm' && *sync_opts != 's') {
          fprintf(stderr, "ERROR: %c is invalid argument for sync option.\n", *optarg);
          exit(1);
        }
        break;
      default:
        abort();
    }
  }

  if (opt_yield) {
    if (strlen(yield_opts) > 3){
      fprintf(stderr, "ERROR: yield given too many arguments.\n");
      exit(1);
    }
    for (int i = 0; i < strlen(yield_opts); i++) {
      if (yield_opts[i] == 'i')
        opt_yield |= INSERT_YIELD; 
      else if (yield_opts[i] == 'd')
        opt_yield |= DELETE_YIELD;
      else if (yield_opts[i] == 'l')
        opt_yield |= LOOKUP_YIELD;
      else {
        fprintf(stderr, "ERROR: %c is invalid argument for yield option.\n", yield_opts[i]);
        exit(1);
      }
    }
  }

  initialize_list();
  timed();
  return 0;
}