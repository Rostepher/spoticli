#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <stdint.h>
#include <pthread.h>

typedef struct audio_fifo_data {
    // queue
    int channels;
    int rate;
    int nsamples;
    int16_t samples[]; 
} audio_fifo_data_t;

typedef struct audio_fifo {
    // queue
    int q_len;
    pthread_mutex mutex;
    pthread_cond cond;
} audio_fifo_t;

// function prototypes

#endif
