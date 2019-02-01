
#ifndef _EXECUTOR_H_
#define _EXECUTOR_H_

#include "internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  pending = 1,
  cancelled = 2,
  finished = 3,
  running = 4
} future_state_t;

typedef struct future_t {
  cond_t condition;
  volatile future_state_t state;
} future_t;

#ifdef __cplusplus
}

#endif

#endif
