#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <stdint.h>
#include <pthread.h>

#include "queue.h"

typedef struct audio_fifo_data {
    TAILQ_ENTRY(audio_fifo_data) link; 
    int channels;
    int rate;
    int nsamples;
    int16_t samples[]; 
} audio_fifo_data_t;

typedef struct audio_fifo {
    TAILQ_HEAD(, audio_fifo_data) queue;
    int q_len;
    pthread_mutex mutex;
    pthread_cond cond;
} audio_fifo_t;

// function prototypes
void audio_init(audio_fifo_t *af);
void audio_fifo_flush(audio_fifo_t *af);
audio_fifo_data_t *audio_get(audio_fifo_t *af);

#endif
