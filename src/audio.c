#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <alsa/asoundlib.h>

#include "audio.h"

#define PCM_DEVICE      "default"
#define PERIOD_SIZE     1024
#define BUFFER_SIZE     (PERIOD_SIZE * 4)


/** 
 * Opens and returns a handle to an alsa "pulse code modulator", which handles
 * playback. This function looks like it does a lot, but most of the code is
 * very much boilerplate code with tons of error checking. A basic outline of
 * the code is thus:
 *
 *      1. open a pcm device
 *      2. allocate and set hardware params struct
 *          * set access
 *          * set format
 *          * set sample rate
 *          * set channel number
 *      3. configure the period
 *      4. configure the buffer size
 *      5. write and free hardware params (finilize)
 *      6. allocate and set software params struct
 *      7. write and free software params
 *      8. prepare pcm device
 *      9. return pcm handle
 *
 * @param device device name
 * @param rate sample rate
 * @param channels channel count
 * 
 * @return a pointer to an alsa pcm handle
 */
static snd_pcm_t *alsa_open(char *device, int rate, int channels)
{
    int error;
    int dir;
    snd_pcm_t *pcm_handle
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;
    snd_pcm_uframes_t period_size;
	snd_pcm_uframes_t buffer_size;

    // open pcm and return NULL if it fails
    if (snd_pcm_open(&pcm_handle, device, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        fprintf(stderr, "ALSA: Error opening PCM device %s\n", device);
        return NULL;
    }
    
    // allocate the hardware params struct
    if ((error = snd_pcm_hw_params_mlloc(&hw_params)) < 0) {
        fprintf(stderr, "ALSA: unable to allocate hardware param struct (%s)\n",
                snd_strerror(error));
        snd_pcm_close(pcm_handle);
        return NULL;
    }
    
    // intialize the hardware params struct
    if ((error = snd_pcm_hw_params_any(pcm_handle, hw_params)) < 0) {
        fprintf(stderr, "ALSA: unable to initialize hardware param struct (%s)\n",
                snd_strerror(error));
        snd_pcm_close(pcm_handle);
        return NULL;
    }

    // set access type to interleaved
    if ((error = snd_pcm_hw_params_set_access(pcm_handle,
                    hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "ALSA: unable to set access type (%s)\n",
                snd_strerror(error));
        snd_pcm_close(pcm_handle);
        return NULL;
    }

    // set sample format
    if ((error = snd_pcm_hw_params_set_format(pcm_handle,
                    hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf(stderr, "ALSA: unable to set sample format (%s)\n",
                snd_strerror(error));
        snd_pcm_close(pcm_handle);
        return NULL;
    }

    // set sample rate
    if ((error = snd_pcm_hw_params_set_rate(pcm_handle,
                    hw_params, rate, 0)) < 0) {
        fprintf(stderr, "ALSA: unable to set sample rate (%s)\n",
                snd_strerror(error));
        snd_pcm_close(pcm_handle);
        return NULL;
    }

    // set channel count
    if ((error = snd_pcm_hw_params_set_channels(pcm_handle,
                    hw_params, channels)) < 0) {
        fprintf(stderr, "ALSA: unable to set channel count (%s)\n",
                snd_strerror(error));
        snd_pcm_close(pcm_handle);
        return NULL;
    }

    // configure the period
    dir = 0;
    period_size = PERIOD_SIZE;
    if ((error = snd_pcm_hw_params_set_period_size_near(pcm_handle,
                    hw_params, &period_size, &dir)) < 0) {
        fprintf(stderr, "ALSA: unable to set period size %lu (%s)\n",
                period_size, snd_strerror(error));
        snd_pcm_close(pcm_handle);
        return NULL;
    }

    // configure the buffer size
    buffer_size = BUFFER_SIZE;
    if ((error = snd_pcm_hw_params_set_buffer_size_near(pcm_handle,
                    hw_params, &buffer_size)) < 0) {
        fprintf(stderr, "ALSA: unable to set buffer size %lu (%s)\n",
                buffer_size, snd_strerror(error));
        snd_pcm_close(pcm_handle);
        return NULL;
    }

    // write the hw params
    if ((error = snd_pcm_hw_params(pcm_handle, hw_params)) < 0) {
        fprintf(stderr, "ALSA: unable to configure hardware params (%s)\n",
                snd_strerror(error));
        snd_pcm_close(pcm_handle);
        return NULL;
    }

    // free the hw params
    snd_pcm_hw_params_free(hw_params);

    // allocate sw_params
    if ((error = snd_pcm_sw_params_malloc(&sw_params)) < 0) {
        fprintf(stderr, "ALSA: unable to allocate software params (%s)\n",
                snd_strerror(error));
        snd_pcm_close(pcm_handle);
        return NULL;
    }

    // configure wakeup threshold
    if ((error = snd_pcm_sw_params_set_avail_min(pcm_handle,
                    sw_params, PERIOD_SIZE)) < 0) {
        fprintf(stderr, "ALSA: unable to configure wakeup threshold (%s)\n",
                snd_strerror(error));
        snd_pcm_close(pcm_handle);
        return NULL;
    }

    // configure start threshold
    if ((error = snd_pcm_sw_params_set_start_threshold(pcm_handle,
                    sw_params, 0)) < 0) {
        fprintf(stderr, "ALSA: unable to configure start threshold (%s)\n",
                snd_strerror(error));
        snd_pcm_close(pcm_handle);
        return NULL;
    }

    // write the sw params
    if ((error = snd_pcm_sw_params(pcm_handle, sw_params)) < 0) {
        fprintf(stderr, "ALSA: unable to configure software params (%s)\n",
                snd_strerror(error));
        snd_pcm_close(pcm_handle);
        return NULL;
    }

    // free the sw params
    snd_pcm_sw_params_free(sw_params);

    // prepare the audio device for playback
    if ((error = snd_pcm_prepare(pcm_handle)) < 0) {
        fprintf(stderr, "ALSA: unable to prepare audio device for playback (%s)\n",
                snd_strerror(error));
        snd_pcm_close(pcm_handle);
        return NULL;
    }

    // return the handle
    return pcm_handle;
}

/**
 * TODO
 */
static void *alsa_audio_start()
{
    snd_pcm_t *pcm_handle = NULL;

    ... // get rate and channels

    pcm_handle = alsa_open(PCM_DEVICE, rate, channels);
    if (!pcm_handle) {
        fprintf(stderr, "Unable to open ALSA device (%d channels %d Hz)\n",
                channels, rate);
        exit(EXIT_FAILURE);
    }

    ... // stuff

    snd_pcm_writei(pcm_handle, .../* samples */, .../* nsamples */);
}

/**
 * TODO
 */
void audio_init()
{

}
