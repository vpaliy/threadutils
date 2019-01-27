#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "queue.h"

#define CONSUMER_THREAD_COUNT 10

static void*    allocate(size_t);
static void     init_cond_bundle(cond_bundle_t *, mutex_t *);
static void     error_exit(char *);
static void     lock_cond(cond_bundle_t *);
static void     unlock_cond(cond_bundle_t *);
static entry_t* create_entry(void *data);
static void     dlog(char *);
static void     cond_wait(cond_bundle_t *);
static void     cond_signal(cond_bundle_t *);


thread_queue_t *
create_thread_queue(size_t maxsize) {
  if (maxsize < 0) {
    return NULL;
  }
  thread_queue_t * queue = (thread_queue_t *)
      (allocate(sizeof(thread_queue_t)));
  memset(queue, 0x0, sizeof(thread_queue_t));
  if (mutex_init(&queue->_mutex, NULL)) {
    error_exit("mutex: failed to create mutex");
  }
  queue->_maxsize = maxsize;
  init_cond_bundle(&queue->_full_c, &queue->_mutex);
  init_cond_bundle(&queue->_empty_c, &queue->_mutex);
  init_cond_bundle(&queue->_join_c, &queue->_mutex);
  dlog("queue: has been initialized");
  return queue;
}

void
thread_queue_put (thread_queue_t *queue, void* item) {
  lock_cond(&queue->_full_c);
  if (queue->_maxsize != 0) {
    while (queue->_maxsize == queue->_item_count) {
      cond_wait(&queue->_full_c);
    }
  }
  entry_t *entry = create_entry(item);
  entry->next = queue->root;
  queue->root = entry;
  queue->_item_count++;
  cond_signal(&queue->_empty_c);
  unlock_cond(&queue->_full_c);
}

entry_t*
thread_queue_get(thread_queue_t *queue) {
  lock_cond(&queue->_empty_c);
  while (queue->_item_count > 0) {
    cond_wait(&queue->_empty_c);
  }
  entry_t *head = queue->root;
  queue->root = head->next;
  queue->_item_count--;
  head->next = NULL;
  cond_signal(&queue->_full_c);
  unlock_cond(&queue->_empty_c);
  return head;
}

static void
cond_wait(cond_bundle_t *bundle) {
  if (pthread_cond_wait(&bundle->cond, &bundle->mutex)) {
    error_exit("cond: failed to wait");
  }
}

static void
cond_signal(cond_bundle_t *bundle) {
  if (pthread_cond_signal(&bundle->cond)) {
    error_exit("cond: failed to signal");
  }
}

static entry_t *
create_entry(void *data) {
  entry_t * entry = (entry_t *)
    (allocate(sizeof(entry_t)));
  entry->data = data;
  entry->next = NULL;
  return entry;
}

static void
dlog(char *message) {
  #ifdef DEBUG
    printf("%s\n", message);
  #endif
}

static void *
allocate(size_t size) {
  void *object = malloc(size);
  if (object == NULL) {
    error_exit("malloc: failed to allocate memory");
  }
  return object;
}

static void
init_cond_bundle(cond_bundle_t *bundle, mutex_t *mutex) {
  bundle->mutex = *mutex;
  if (cond_init(&bundle->cond, NULL)) {
    error_exit("cond: failed to init");
  }
  dlog("cond: has been initialized");
}

static void
error_exit(char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

static void
lock_cond(cond_bundle_t *cond_bundle) {
  if (mutex_lock(&cond_bundle->mutex)) {
    error_exit("mutex: lock failed");
  }
}

static void
unlock_cond(cond_bundle_t *cond_bundle) {
  if (mutex_unlock(&cond_bundle->mutex)) {
    error_exit("mutex: unlock failed");
  }
}

thread_queue_t *queue = create_thread_queue(10);

static typedef struct producer_args {
  int thread_id;
  size_t buffer_size;
} producer_args;

void *
producer(void *args) {
  producer_args *args = (producer_args *)(args);
  printf("producer %d: has started \n", args->thread_id);
  int data [args->buffer_size];
  for (int index = 0; index < args->buffer_size; index++) {
    data[index] = index;
    printf("producer %d: putting value = %d \n", args->thread_id, index);
    thread_queue_put(queue, data + index);
    printf("producer %d: has put value = %d \n", args->thread_id, index);
  }
  printf("producer %d: has finished\n", args->thread_id);
  pthread_exit(NULL);
}

void *
consumer(void *args) {
  int thread_id = *(int*)(args);
  printf("consumer %d: has started\n", thread_id);
  while (1) {
    printf("consumer %d: is waiting\n", thread_id);
    entry_t *entry = thread_queue_get(queue);
    if (entry == NULL) {
      printf("consumer %d: has finished\n", thread_id);
      break;
    }
    print("thread: %d has received %d\n", *(int*)(entry->data));
  }
  pthread_exit(NULL);
}
