#ifndef _THREAD_QUEUE_H_
#define _THREAD_QUEUE_H_

#include "internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cond_bundle_t {
  mutex_t mutex;
  cond_t cond;
} cond_bundle_t;

typedef struct entry_t {
  struct entry_t *next;
  void *data;
} entry_t;

typedef struct thread_queue_t {
  cond_bundle_t _full_c;
  cond_bundle_t _empty_c;
  cond_bundle_t _join_c;
  mutex_t _mutex;
  size_t _maxsize;
  size_t _item_count;
  entry_t *root;
} thread_queue_t;

void destroy_thread_queue(thread_queue_t *);
void destroy_entry(entry_t *);
void thread_queue_put(thread_queue_t *, void *);
entry_t *thread_queue_get(thread_queue_t *);
thread_queue_t *create_thread_queue();

#ifdef __cplusplus
}

#endif

#endif
