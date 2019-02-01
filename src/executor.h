#ifndef _EXECUTOR_H_
#define _EXECUTOR_H_

#include "internal.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_THREADS 64
#define MAX_QUEUE 65536

typedef enum {
  threadpool_terminate_all = 1,
  threadpool_wait = 2
} threadpool_destroy_flags_t;

typedef struct tptask_t {
  void (*function)(void *);
  void *args;
} tptask_t;

typedef struct threadpool_t {
  thqueue_t *queue;
  thread_t *threads;
  volatile size_t maxworkers;
} threadpool_t;

threadpool_t *threadpool_create(size_t);
int threadpool_destroy(threadpool_t *, int flags);
int threadpool_add_task(threadpool_t *, void (*function)(void *), void *);

#ifdef __cplusplus
}

#endif

#endif
