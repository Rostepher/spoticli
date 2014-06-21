#ifndef SPOTICLI_AUDIO_H
#define SPOTICLI_AUDIO_H

#include <stdint.h>
#include <pthread.h>

#include "queue.h"

typedef struct audio_fifo_data_s {
    int channels;
    int rate;
    int nsamples;
    int16_t samples[];
} audio_fifo_data_t;

typedef struct audio_fifo_s {
    queue_t *queue;         // generic queue to hold audio_fifo_data_t
    int total_samples;      // total samples in the queue
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} audio_fifo_t;

void audio_init(audio_fifo_t *af);
void audio_fifo_flush(audio_fifo_t *af);
audio_fifo_data_t *audio_get(audio_fifo_t *af);

#endif
