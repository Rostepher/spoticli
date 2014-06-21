#include "queue.h"

/**
 * Helper function to create queue elements, which are nodes.
 *
 * @param data void pointer to some data
 * @param next pointer to next queue_elem_t
 *
 * @return pointer to a new queue_elem_t
 */
static queue_elem_t *queue_elem_create(void *data, queue_elem_t *next)
{
    queue_elem_t *queue_elem = (queue_elem_t *) malloc(sizeof(queue_elem_t));
    queue_elem->data = data;
    queue_elem->next = next;

    return queue_elem;
}

/**
 * Destroys a queue_elem_t.
 *
 * @param queue_elem pointer to queue_elem_t
 */
static void queue_elem_destroy(queue_elem_t *queue_elem)
{
    free(queue_elem);
}

/**
 * Returns the address to a new queue_t.
 *
 * @return pointer to new queue_t
 */
queue_t *queue_create()
{
    queue_t *queue = (queue_t *) malloc(sizeof(queue_t));
    queue->size = 0;
    queue->head = NULL;

#ifdef SPOTICLI_QUEUE_THREAD_SAFE
    pthread_mutex_init(&(queue->mutex), NULL);
#endif

    return queue;
}

/**
 * Destroys a queue.
 *
 * @param queue pointer to queue_t
 */
void queue_destroy(queue_t *queue)
{
    queue_clean(queue);

#ifdef SPOTICLI_QUEUE_THREAD_SAFE
    pthread_mutex_destroy(&(queue->mutex));
#endif

    free(queue);
}

/**
 * Cleans a given queue, meaning it clears and destroys all queue elements and
 * resets the size to zero.
 *
 * @param queue pointer to queue_t
 */
void queue_clean(queue_t *queue)
{
#ifdef SPOTICLI_QUEUE_THREAD_SAFE
    pthread_mutex_lock(&(queue->mutex));
#endif

    queue_elem_t *temp;
    queue_elem_t *curr = queue->head;
    while (curr != NULL) {
        temp = curr;
        curr = temp->next;

        queue_elem_destroy(temp);
    }

    queue->size = 0;

#ifdef SPOTICLI_QUEUE_THREAD_SAFE
    pthread_mutex_unlock(&(queue->mutex));
#endif
}

/**
 * Enqueues the given data pointer onto a queue.
 *
 * @param queue pointer to queue_t
 * @param data void pointer to data
 */
void queue_enqueue(queue_t *queue, void *data)
{
#ifdef SPOTICLI_QUEUE_THREAD_SAFE
    pthread_mutex_lock(&(queue->mutex));
#endif

    if (queue->head == NULL) {
        queue->head = queue_elem_create(data, NULL);
    } else {
        queue_elem_t *curr = queue->head;
        while (curr->next != NULL)
            curr = curr->next;

        curr->next = queue_elem_create(data, NULL);
    }

    queue->size++;

#ifdef SPOTICLI_QUEUE_THREAD_SAFE
    pthread_mutex_unlock(&(queue->mutex));
#endif
}

/**
 * Dequeues and returns the first element in a queue.
 *
 * @param queue pointer to queue_t
 *
 * @return pointer to first queue element
 */
queue_elem_t *queue_dequeue(queue_t *queue)
{
#ifdef SPOTICLI_QUEUE_THREAD_SAFE
    pthread_mutex_lock(&(queue->mutex));
#endif

    // sanity check
    if (!queue || queue_is_empty(queue))
        return NULL;

    queue_elem_t *temp = queue->head;
    queue->head = temp->next;
    queue->size--;

#ifdef SPOTICLI_QUEUE_THREAD_SAFE
    pthread_mutex_unlock(&(queue->mutex));
#endif

    return temp;
}

/**
 * Returns a pointer to the first element in a queue.
 *
 * @param queue pointer to queue_t
 *
 * @return pointer to first queue element
 */
queue_elem_t *queue_peek(queue_t *queue)
{
#ifdef SPOTICLI_QUEUE_THREAD_SAFE
    pthread_mutex_lock(&(queue->mutex));
#endif

    return queue->head;

#ifdef SPOTICLI_QUEUE_THREAD_SAFE
    pthread_mutex_unlock(&(queue->mutex));
#endif
}

/**
 * Returns the size of a queue.
 *
 * @param queue pointer to queue_t
 *
 * @return size of queue
 */
int queue_size(queue_t *queue)
{
    return queue->size;
}

/**
 * Returns if a queue is empty.
 *
 * @param queue pointer to queue_t
 *
 * @return if queue is empty
 */
bool queue_is_empty(queue_t *queue)
{
    return queue->size <= 0;
}
