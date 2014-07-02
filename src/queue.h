#ifndef SPOTICLI_QUEUE_H
#define SPOTICLI_QUEUE_H

#ifdef SPOTICLI_QUEUE_THREAD_SAFE
#include <pthread.h>
#endif

#include <stdbool.h>
#include <stdlib.h>

typedef struct queue_elem_s {
    void *data;
    struct queue_elem_s *next;
} queue_elem_t;

typedef struct queue_s {
    queue_elem_t *head;
    int size;
#ifdef SPOTICLI_QUEUE_THREAD_SAFE
    pthread_mutex_t mutex;
#endif
} queue_t;

queue_t *queue_create();
void queue_destroy(queue_t *queue);
void queue_flush(queue_t *queue);
void queue_enqueue(queue_t *queue, void *data);
queue_elem_t *queue_dequeue(queue_t *queue);
queue_elem_t *queue_peek(queue_t *queue);
int queue_size(queue_t *queue);
bool queue_is_empty(queue_t *queue);

#endif
