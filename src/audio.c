#include <stdio.h>
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
 *      5. write and free hardware params (finalize)
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
    snd_pcm_t *pcm_handle;
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
    if ((error = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
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

    // set sample format to signed 16 bit little endian
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
 * Opens the alsa pcm device and feeds it the given audio. The audio pointer
 * is cast to audio_fifo_t, and then each sample is gathered with
 * audio_fifo_dequeue().This function will be passed as a parameter to a
 * pthread, hence why the argument is a void pointer.
 *
 * The majority of this function was borrowed from the example "jukebox"
 * supplied with libspotify. Some variable names were changed and other
 * minor changes to make the code more readable.
 *
 * @param audio void pointer to an instance of audio_fifo_t
 */
static void *alsa_audio_start(void *audio)
{
    audio_fifo_t *af = (audio_fifo_t *) audio;
    snd_pcm_t *pcm_handle = NULL;
    int cur_rate = 0;
    int cur_channels = 0;

    int error;
    audio_data_t *ad;

    while (true) {
        ad = audio_fifo_dequeue(af);

        if (pcm_handle == NULL ||
            cur_rate != ad->sample_rate ||
            cur_channels != ad->channels) {

            if (pcm_handle)
                snd_pcm_close(pcm_handle);

            cur_rate = ad->sample_rate;
            cur_channels = ad->channels;

            pcm_handle = alsa_open(PCM_DEVICE, cur_rate, cur_channels);
            if (pcm_handle == NULL) {
                fprintf(stderr,
                        "ALSA: unable to open pcm device (%d channels %d Hz)\n",
                        cur_channels,
                        cur_rate);
                exit(EXIT_FAILURE);
            }

            error = snd_pcm_wait(pcm_handle, 1000);
            if (error >= 0)
                error = snd_pcm_avail_update(pcm_handle);

            if (error == -EPIPE)
                snd_pcm_prepare(pcm_handle);

            snd_pcm_writei(pcm_handle, ad->samples, ad->nsamples);
            free(ad);
        }
    }
}

/**
 * Allocates and returns a pointer to a new audio_data_t, no data is held in
 * the samples flexable array.
 *
 * @param channels number of channels
 * @param nsamples number of samples
 * @param rate sample rate of the audio
 *
 * @return pointer to new audio_data_t
 */
audio_data_t *audio_data_create(int channels, int nsamples, int sample_rate)
{
    // calculate sample size
    size_t sample_size = nsamples * sizeof(int16_t) * channels;

    // allocate with sample size
    audio_data_t *ad = malloc(sizeof(audio_data_t) + sample_size);

    ad->channels = channels;
    ad->nsamples = nsamples;
    ad->sample_rate = sample_rate;
    ad->sample_size = sample_size;

    return ad;
}

/**
 * Free the allocated memory of an audio_data_t.
 *
 * @param ad pointer to audio_data_t
 */
void audio_data_destroy(audio_data_t *ad)
{
    free(ad);
}

/**
 * Initializes the audio_fifo_t and spawns a new pthread to handle audio
 * playback.
 *
 * This function is borrowed from the example "jukebox" supplied with
 * libspotify. Some changes have been made to make it more readable.
 *
 * @param af address of audio_fifo_t allocated on the stack
 */
void audio_fifo_init(audio_fifo_t *af)
{
    pthread_t thread_id;

    // create queue
    af->queue = queue_create();

    // set total_samples to 0
    af->total_samples = 0;

    pthread_mutex_init(&af->mutex, NULL);
    pthread_cond_init(&af->cond, NULL);

    pthread_create(&thread_id, NULL, alsa_audio_start, af);
}

/**
 * Flushes the given audio_fifo queue of all members and resets the queue
 * length to 0. Thread safe.
 *
 * @param af audio_fifo_t
 */
void audio_fifo_flush(audio_fifo_t *af)
{
    pthread_mutex_lock(&af->mutex);

    // flush the queue
    queue_flush(af->queue);

    pthread_mutex_unlock(&af->mutex);
}

/**
 * Returns the data from the first element in the given audio_fifo_t queue.
 * Thread safe.
 *
 * @param af audio_fifo_t
 *
 * @return pointer to audio_fifo_t
 */
audio_data_t *audio_fifo_dequeue(audio_fifo_t *af)
{
    audio_data_t *ad;
    pthread_mutex_lock(&af->mutex);

    // wait until more audio data shows up
    while (queue_is_empty(af->queue))
        pthread_cond_wait(&af->cond, &af->mutex);

    // dequeue and cast back to audio_data_t
    ad = (audio_data_t *) queue_dequeue(af->queue);

    // update the total samples in the queue
    af->total_samples -= ad->nsamples;

    pthread_mutex_unlock(&af->mutex);
    return ad;
}
