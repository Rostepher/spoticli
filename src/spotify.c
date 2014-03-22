#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <libspotify/api.h>

#include "audio.h"
#include "queue.h"
#include "spotify.h"

#define DEBUG
#include "debug.h"

#define CLIENT_NAME         "spoticli"
#define CACHE_LOCATION      "tmp"
#define SETTINGS_LOCATION   "tmp"


// global variables ////////////////////////////////////////////////////////////
extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;

static sp_session *g_session = NULL;
static audio_fifo_t g_audio_fifo;
static bool g_playback_done = false;

pthread_mutex_t g_notify_mutex;
pthread_cond_t g_notify_cond;
bool g_notify_do = false;


// session callbacks ///////////////////////////////////////////////////////////
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


// session structs /////////////////////////////////////////////////////////////
static sp_session_callbacks session_callbacks = {
    .logged_in = &logged_in,
    .logged_out = &logged_out,
    .metadata_updated = &metadata_updated,
    .notify_main_thread = &notify_main_thread,
    .music_delivery = &music_delivery,
    .play_token_lost = &play_token_lost,
    .log_message = &log_message,
    .end_of_track = &end_of_track
};

static sp_session_config session_config = {
    .api_version = SPOTIFY_API_VERSION,
    .cache_location = CACHE_LOCATION,
    .settings_location = SETTINGS_LOCATION,
    .application_key = g_appkey,
    .application_key_size = 0,      // set in main
    .user_agent = CLIENT_NAME,
    .callbacks = &session_callbacks
};


// session management //////////////////////////////////////////////////////////
void session_init()
{
    sp_error error;
    sp_session *session;

    // set appkey size
    session_config.application_key_size = g_appkey_size;

    // create spotify session
    error = sp_session_create(&session_config, &session);
    if (error != SP_ERROR_OK) {
        fprintf(stderr, "Unable to create session (%s)\n",
                sp_error_message(error));
        exit(EXIT_FAILURE);
    }
}

void session_release()
{
    if (g_session)
        sp_session_release(g_session);
}

void session_login(const char *username, const char *password)
{
    sp_error error;

    error = sp_session_login(g_session, username, password, 0, NULL);
    if (error != SP_ERROR_OK) {
        fprintf(stderr, "Unable to login to spotify (%s)\n",
                sp_error_message(error));
        
        // if there is a session release it
        if (g_session)
            session_release();
        
        exit(EXIT_FAILURE);
    }
}

void session_logout()
{
    if (g_session && (sp_session_user(g_session)))
        sp_session_logout(g_session);
}


// playlist management /////////////////////////////////////////////////////////


// track management ////////////////////////////////////////////////////////////


// browsing management /////////////////////////////////////////////////////////


// events handling /////////////////////////////////////////////////////////////
bool session_process_events(int *next_timeout)
{
    if (!g_session)
        return false;

    do {
        sp_session_process_events(g_session, next_timeout);
    } while (*next_timeout == 0);

    return true;
}
