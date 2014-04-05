#include <stdlib.h>
#include <string.h>

#include "session.h"
#include "ui.h"

#define DEBUG
#include "debug.h"


#define CLIENT "spoticli"


// appkey //////////////////////////////////////////////////////////////////////
extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;


// globals /////////////////////////////////////////////////////////////////////
session_t *g_session = NULL;


// session callback prototypes /////////////////////////////////////////////////
static void logged_in(sp_session *session, sp_error);
static void logged_out(sp_session *session);
static void metadata_updated(sp_session *session);
static void notify_main_thread(sp_session *session);
static void play_token_lost(sp_session *session);
static void log_message(sp_session *session, const char *message);
static void end_of_track(sp_session *session);
static int music_delivery(sp_session *session,
                          const sp_audioformat *format,
                          const void *frames,
                          int num_frames);


// session config //////////////////////////////////////////////////////////////
static sp_session_callbacks callbacks = {
    .logged_in          = &logged_in,
    .logged_out         = &logged_out,
    .metadata_updated   = &metadata_updated,
    .notify_main_thread = &notify_main_thread,
    .music_delivery     = &music_delivery,
    .play_token_lost    = &play_token_lost,
    .log_message        = &log_message,
    .end_of_track       = &end_of_track
};

static sp_session_config config = {
    .api_version            = SPOTIFY_API_VERSION,
    .cache_location         = "",
    .settings_location      = "",
    .application_key        = g_appkey,
    .application_key_size   = 0,  // set in session_init
    .user_agent             = CLIENT,
    .callbacks              = &callbacks
};


// session handlers ////////////////////////////////////////////////////////////
void session_init()
{
    sp_error error;
    sp_session *session;

    // set appkey size
    config.application_key_size = g_appkey_size;
    
    // create spotify session
    error = sp_session_create(&config, &session);
    if (error != SP_ERROR_OK) {
        fprintf(stderr, "Unable to create session: %s\n",
                sp_error_message(error));
        exit(EXIT_FAILURE);
    }

    g_session = malloc(sizeof(session_t));

    // init and lock mutex
    pthread_mutex_init(&(g_session->notify_mutex), NULL);
    pthread_cond_init(&(g_session->notify_cond), NULL);

    g_session->notify_do = false;
    g_session->playback_done = false;

    // set global session handle
    g_session->sp_session = session;
}

void session_release()
{
    if (!g_session)
        exit(EXIT_FAILURE);

    sp_session_release(g_session->sp_session);
    free(g_session);
}

void session_login(const char *username, const char *password)
{
    sp_session_login(g_session->sp_session, username, password, 0, NULL);
    g_session->state = SESSION_ONLINE;
}

void session_logout()
{
    sp_error error;

    error = sp_session_logout(g_session->sp_session);
    if (error != SP_ERROR_OK) {
        fprintf(stderr, "Unable to logout: %s\n",
                sp_error_message(error));
        exit(EXIT_FAILURE);
    }

    g_session->state = SESSION_OFFLINE;
}

void session_search(const char *query)
{
    // TODO
}

void session_play(sp_track *track)
{
    // TODO
}

void session_pause()
{
    // TODO
}

void session_stop()
{
    // TODO
}


// session callbacks ///////////////////////////////////////////////////////////
static void logged_in(sp_session *session, sp_error error)
{
    debug("logged_in called\n");
    
    if (error != SP_ERROR_OK) {
        fprintf(stderr, "Unable to login: %s\n", sp_error_message(error));
        exit(EXIT_FAILURE);
    }
}

static void logged_out(sp_session *session)
{
    debug("logged_out called\n");
}

static void metadata_updated(sp_session *session)
{
    debug("metadata_updated called\n");
}

static void notify_main_thread(sp_session *session)
{
    debug("notify_main_thread called\n");
    
    if (!g_session)
        exit(EXIT_FAILURE);

    pthread_mutex_lock(&(g_session->notify_mutex));
    g_session->notify_do = true;
    pthread_cond_signal(&(g_session->notify_cond));
    pthread_mutex_unlock(&(g_session->notify_mutex));
}

static void play_token_lost(sp_session *session)
{
    debug("play_token_lost called\n");

    audio_fifo_flush(&(g_session->audio_fifo));

    // if currently playing a track, stop it.
}

static void log_message(sp_session *session, const char *message)
{
    log_info("%s", message);
}

static void end_of_track(sp_session *session)
{
    debug("end_of_track called\n");

    pthread_mutex_lock(&(g_session->notify_mutex));
    g_session->playback_done = true;
    g_session->notify_do = true;
    pthread_cond_signal(&(g_session->notify_cond));
    pthread_mutex_unlock(&(g_session->notify_mutex));
}

// function borrowed from example "jukebox" provided with libspotify
static int music_delivery(sp_session *session,
                           const sp_audioformat *format,
                           const void *frames,
                           int num_frames)
{
    debug("music_delivery called\n");
    
    audio_fifo_t *af = &(g_session->audio_fifo);
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
