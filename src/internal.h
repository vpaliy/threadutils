#ifndef _INTERNAL_THREAD_UTILS_H_
#define _INTERNAL_THREAD_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600

#define mutex_t SRWLOCK

#define mutex_init InitializeSRWLock
#define mutex_lock AcquireSRWLockExclusive
#define mutex_unlock ReleaseSRWLockExclusive
#define mutex_destroy(m)

#define cond_t CONDITION_VARIABLE

#define cond_init InitializeConditionVariable
#define cond_signal WakeConditionVariable
#define cond_broadcast WakeAllConditionVariable
#define cond_wait(c, m) SleepConditionVariableSRW((c), (m), INFINITE, 0)
#define cond_destroy(c)

#else

#define mutex_t CRITICAL_SECTION

#define mutex_init(m) InitializeCriticalSectionAndSpinCount((m), MUTEX_SPINS)
#define mutex_lock EnterCriticalSection
#define mutex_unlock LeaveCriticalSection
#define mutex_destroy DeleteCriticalSection

typedef struct cond_t {
  int waiters_count;
  CRITICAL_SECTION waiters_count_lock;
  int release_count;
  int wait_generation_count;
  HANDLE event;
} cond_t;

static void cond_init(cond_t* c) {
  c->waiters_count = 0;
  InitializeCriticalSection(&c->waiters_count_lock);
  c->release_count = 0;
  c->wait_generation_count = 0;
  c->event = CreateEvent(NULL, true, false, NULL);
}

static void cond_signal(cond_t* c) {
  EnterCriticalSection(&c->waiters_count_lock);
  if (c->waiters_count > c->release_count) {
    SetEvent(c->event);
    c->release_count++;
    c->wait_generation_count++;
  }
  LeaveCriticalSection(&c->waiters_count_lock);
}

static void cond_broadcast(cond_t* c) {
  EnterCriticalSection(&c->waiters_count_lock);
  if (c->waiters_count > 0) {
    SetEvent(c->event);
    c->release_count = c->waiters_count;
    c->wait_generation_count++;
  }
  LeaveCriticalSection(&c->waiters_count_lock);
}

static void cond_wait(cond_t* c, mutex_t* m) {
  EnterCriticalSection(&c->waiters_count_lock);

  c->waiters_count++;

  int my_generation = c->wait_generation_count;

  LeaveCriticalSection(&c->waiters_count_lock);
  mutex_unlock(m);

  bool wait_done;

  do {
    WaitForSingleObject(c->event, INFINITE);

    EnterCriticalSection(&c->waiters_count_lock);
    int release_count = c->release_count;
    int wait_generation_count = c->wait_generation_count;
    LeaveCriticalSection(&c->waiters_count_lock);

    wait_done = release_count > 0 && wait_generation_count != my_generation;
  } while (!wait_done);

  mutex_lock(m);
  EnterCriticalSection(&c->waiters_count_lock);
  c->waiters_count--;
  int release_count = --c->release_count;
  LeaveCriticalSection(&c->waiters_count_lock);

  if (release_count == 0) ResetEvent(c->event);
}

static void cond_destroy(cond_t* c) {
  DeleteCriticalSection(&c->waiters_count_lock);
  CloseHandle(c->event);
}

#endif

#else

#include <pthread.h>

#define thread_t pthread_t

#define mutex_t pthread_mutex_t
#define mutex_attr_t pthread_mutexattr_t
#define mutex_init(m) pthread_mutex_init((m), NULL)
#define mutex_lock pthread_mutex_lock
#define mutex_unlock pthread_mutex_unlock
#define mutex_destroy pthread_mutex_destroy

#define cond_t pthread_cond_t
#define cond_init(c) pthread_cond_init((c), NULL)
#define cond_wait(c, m) pthread_cond_wait((c), (m))
#define cond_attr_t pthread_condattr_t
#define cond_signal pthread_cond_signal
#define cond_broadcast pthread_cond_broadcast
#define cond_destroy pthread_cond_destroy

#endif

#ifdef __cplusplus
}
#endif

#endif
