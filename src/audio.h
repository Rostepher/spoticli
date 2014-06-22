#ifndef SPOTICLI_AUDIO_H
#define SPOTICLI_AUDIO_H

#include <stdint.h>
#include <pthread.h>

#include "queue.h"

typedef struct audio_data_s {
    int channels;
    int nsamples;
    int sample_rate;
    size_t sample_size;     // size of samples array
    int16_t samples[];      // flexable array
} audio_data_t;

typedef struct audio_fifo_s {
    queue_t *queue;         // generic queue to hold audio_data_t
    int total_samples;      // total samples in the queue
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} audio_fifo_t;

audio_data_t *audio_data_create();
void audio_data_destroy(audio_data_t *afd);

void audio_fifo_init(audio_fifo_t *af);
void audio_fifo_flush(audio_fifo_t *af);
audio_data_t *audio_fifo_dequeue(audio_fifo_t *af);

#endif
