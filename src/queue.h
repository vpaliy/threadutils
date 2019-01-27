#ifndef _THREAD_QUEUE_H_
#define _THREAD_QUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <stdbool.h>

#define thread_t        pthread_t
#define mutex_t         pthread_mutex_t
#define cond_t          pthread_cond_t
#define mutex_attr_t    pthread_mutexattr_t
#define cond_attr_t     pthread_condattr_t
#define mutex_init      pthread_mutex_init
#define cond_init       pthread_cond_init
#define mutex_lock      pthread_mutex_lock
#define mutex_unlock    pthread_mutex_unlock
#define mutex_destroy   pthread_mutex_destroy
#define cond_destroy    pthread_cond_destroy

typedef struct cond_bundle_t {
  mutex_t mutex;
  cond_t  cond;
} cond_bundle_t;

typedef struct entry_t {
  struct entry_t *next;
  void    *data;
} entry_t;

typedef struct thread_queue_t {
  cond_bundle_t _full_c;
  cond_bundle_t _empty_c;
  cond_bundle_t _join_c;
  mutex_t       _mutex;
  size_t        _maxsize;
  size_t        _item_count;
  entry_t       *root;
} thread_queue_t;

void destroy_thread_queue(thread_queue_t *);

void destroy_entry(entry_t *);

void thread_queue_put(thread_queue_t*, void*);

entry_t * thread_queue_get(thread_queue_t*);

thread_queue_t * create_thread_queue();


#ifdef __cplusplus
}

#endif

#endif
