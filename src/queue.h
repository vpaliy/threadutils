#ifndef _THREAD_QUEUE_H_
#define _THREAD_QUEUE_H_

#include "internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  queue_invalid = -1,
  queue_lock_failure = -2,
  queue_full = -3,
  queue_thread_failure = -5,
  queue_memory_allocation = -6
} queue_error_t;

typedef struct entry_t {
  struct entry_t *next;
  void *data;
} entry_t;

typedef struct thqueue_t {
  cond_t full_c;
  cond_t empty_c;
  cond_t join_c;
  mutex_t mutex;
  volatile size_t maxsize;
  volatile size_t item_count;
  entry_t *root;
} thqueue_t;

thqueue_t *thqueue_init(size_t);
int thqueue_destroy(thqueue_t *);
int thqueue_add(thqueue_t *, void *);
void *thqueue_get(thqueue_t *);

#ifdef __cplusplus
}

#endif

#endif
