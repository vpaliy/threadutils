#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"

#ifdef THQUEUE_DEBUG
#define THQUEUE_DEBUG 1
#else
#define THQUEUE_DEBUG 0
#endif

#if !defined(DISABLE_PRINT) || defined(THQUEUE_DEBUG)
#define err(str) fprintf(stderr, str)
#else
#define err(str)
#endif

static entry_t *create_entry(void *data);

thqueue_t *thqueue_init(size_t maxsize) {
  if (maxsize < 0) {
    err("thqueue_init(): maxsize < 0\n");
    return NULL;
  }
  thqueue_t *queue = (thqueue_t *)(malloc(sizeof(thqueue_t)));
  if (queue == NULL) {
    err("thqueue_init(): memory allocation failed\n");
    return NULL;
  }
  memset(queue, 0x0, sizeof(thqueue_t));
  if (mutex_init(&queue->mutex) != 0 || cond_init(&queue->full_c) != 0 ||
      cond_init(&queue->empty_c) != 0 || cond_init(&queue->join_c) != 0) {
    err("thqueue_init(): failed to initialize thread resources");
    thqueue_destroy(queue);
    return NULL;
  }
  queue->maxsize = maxsize;
  return queue;
}

int thqueue_add(thqueue_t *queue, void *item) {
  if (queue == NULL) {
    err("thqueue_add(): invalid queue");
    return queue_invalid;
  }
  mutex_lock(&queue->mutex);
  if (queue->maxsize != 0) {
    while (queue->maxsize == queue->item_count) {
      cond_wait(&queue->full_c, &queue->mutex);
    }
  }
  entry_t *entry = create_entry(item);
  entry->next = queue->root;
  queue->root = entry;
  queue->item_count++;
  cond_signal(&queue->empty_c);
  mutex_unlock(&queue->mutex);
  return 1;
}

void *thqueue_get(thqueue_t *queue) {
  if (queue == NULL) {
    err("thqueue_get(): invalid queue");
    return NULL;
  }
  mutex_lock(&queue->mutex);
  while (queue->item_count > 0) {
    cond_wait(&queue->empty_c, &queue->mutex);
  }
  entry_t *head = queue->root;
  queue->root = head->next;
  queue->item_count--;
  head->next = NULL;
  cond_signal(&queue->full_c);
  mutex_unlock(&queue->mutex);
  return head->data;
}

int thqueue_destroy(thqueue_t *queue) {
  if (queue == NULL) {
    err("thqueue_destroy(): invalid queue");
    return queue_invalid;
  }
  if (queue->root) {
    entry_t *node = queue->root;
    while (node != NULL) {
      next = node->next;
      node->next = NULL;
      free(node);
      node = next;
    }
    queue->root = NULL;
    cond_destroy(&queue->empty_c);
    cond_destroy(&queue->full_c);
    cond_destroy(&queue->join_c);
    mutex_destroy(&queue->mutex);
  }
  free(queue);
  return 1;
}

static entry_t *create_entry(void *data) {
  entry_t *entry = (entry_t *)(malloc(sizeof(entry_t)));
  if (entry == NULL) {
    return NULL;
  }
  entry->data = data;
  entry->next = NULL;
  return entry;
}
