#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include <libspotify/api.h>

#define DEBUG
#include "debug.h"
#include "audio.h"

#define CLIENT "spoticli"


// values from appkey.c
extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;
extern const char *g_username;
extern const char *g_password;

// globals
static sp_session *g_session;
static audio_fifo_t g_audio_fifo;
static pthread_mutex_t g_notify_mutex;
static pthread_cond_t g_notify_cond;
static bool g_notify_do = false;
static bool g_playback_done = false;


// session callbacks
static void logged_in(sp_session *session, sp_error error)
{
    debug("logged_in called");
    
    if (error != SP_ERROR_OK) {
        fprintf(stderr, "Unable to login: %s\n", sp_error_message(error));
        exit(EXIT_FAILURE);
    }
}

static void logged_out(sp_session *session)
{
    debug("logged_out called");
}

static void metadata_updated(sp_session *session)
{
    debug("metadata_updated called");
}

static void notify_main_thread(sp_session *session)
{
    debug("notify_main_thread called");
    
    pthread_mutex_lock(&g_notify_mutex);
    g_notify_do = true;
    pthread_cond_signal(&g_notify_cond);
    pthread_mutex_unlock(&g_notify_mutex);
}

static void play_token_lost(sp_session *session)
{
    debug("play_token_lost called");

    audio_fifo_flush(&g_audio_fifo);

    // if currently playing a track, stop it.
}

static void log_message(sp_session *session, const char *message)
{
    debug("log_message called");
    log_info("%s", message);
}

static void end_of_track(sp_session *session)
{
    debug("end_of_track called");

    pthread_mutex_lock(&g_notify_mutex);
    g_playback_done = true;
    g_notify_do = true;
    pthread_cond_signal(&g_notify_cond);
    pthread_mutex_unlock(&g_notify_mutex);
}

// function borrowed from example "jukebox" provided with libspotify
static int music_delivery(sp_session *session,
                           const sp_audioformat *format,
                           const void *frames,
                           int num_frames)
{
    debug("music_delivery called");
    
    audio_fifo_t *af = &g_audio_fifo;
    audio_fifo_data_t *afd;
    size_t s;

    if (num_frames == 0)
        return 0;   // audio discontinuity

    pthread_mutex_lock(&af->mutex);

    // buffer one second of audio
    if (af->q_len > format->sample_rate) {
        pthread_mutex_unlock(&af->mutex);

        return 0;
    }

    s = num_frames * sizeof(int16_t) * format->channels;

    afd = malloc(sizeof(*afd) + s);
    memcpy(afd->samples, frames, s);

    afd->nsamples = num_frames;
    afd->rate = format->sample_rate;
    afd->channels = format->channels;

    TAILQ_INSERT_TAIL(&af->queue, afd, link);
    af->q_len += num_frames; // += nsamples

    pthread_cond_signal(&af->cond);
    pthread_mutex_unlock(&af->mutex);

    return num_frames;
}

static sp_session_callbacks session_callbacks = {
    .logged_in          = &logged_in,
    .logged_out         = &logged_out,
    .metadata_updated   = &metadata_updated,
    .notify_main_thread = &notify_main_thread,
    .music_delivery     = &music_delivery,
    .play_token_lost    = &play_token_lost,
    .log_message        = &log_message,
    .end_of_track       = &end_of_track
};

static sp_session_config session_config = {
    .api_version            = SPOTIFY_API_VERSION,
    .cache_location         = "tmp",
    .settings_location      = "tmp",
    .application_key        = g_appkey,
    .application_key_size   = 0,                    // set in main
    .user_agent             = CLIENT,
    .callbacks              = &session_callbacks,
    NULL,                   // TODO
};

int main(int argc, char **argv)
{
    sp_error error;
    sp_session *session;
    int next_timeout = 0;

    // set appkey size
    session_config.application_key_size = g_appkey_size;
    
    // create spotify session
    error = sp_session_create(&session_config, &session);
    if (error != SP_ERROR_OK) {
        fprintf(stderr, "Unable to create session: %s\n",
                sp_error_message(error));
        exit(EXIT_FAILURE);
    }

    // init and lock mutex
    pthread_mutex_init(&g_notify_mutex, NULL);
    pthread_cond_init(&g_notify_cond, NULL);
    
    // log in
    sp_session_login(session, g_username, g_password, 0, NULL);
    
    g_session = session;

    pthread_mutex_lock(&g_notify_mutex);

    while (true) {
        if (next_timeout == 0) {
            while (!g_notify_do)
                pthread_cond_wait(&g_notify_cond, &g_notify_mutex);
        } else {
            // current time
            struct timespec ts;

            // convert the next_timeout to a real time, realative to current time
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += next_timeout / 1000;
            ts.tv_nsec += (next_timeout % 1000) * 1E6;

            pthread_cond_timedwait(&g_notify_cond, &g_notify_mutex, &ts);
        }

        pthread_mutex_unlock(&g_notify_mutex);

        do {
            sp_session_process_events(session, &next_timeout);
        } while (next_timeout == 0);

        pthread_mutex_lock(&g_notify_mutex);
    }

    sp_session_release(session);

    return EXIT_SUCCESS;
}
