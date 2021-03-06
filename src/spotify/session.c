#include <stdlib.h>
#include <string.h>

#include "session.h"
#include "ui/ui.h"

#define DEBUG
#include "debug.h"

#define CLIENT_NAME "spoticli"


extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;

// global session handle
sp_session *g_session;
// global session notify mutex
pthread_mutex_t g_notify_mutex;
// global session notify cond
pthread_cond_t g_notify_cond;
// global audio fifo
audio_fifo_t g_audio_fifo;
// global bool updating the main thread
bool g_notify_do;
// global bool denoting if current playback is complete
bool g_playback_done;


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
    .application_key_size   = 0,
    .user_agent             = CLIENT_NAME,
    .callbacks              = &callbacks
};

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

    // init and lock mutex
    pthread_mutex_init(&g_notify_mutex, NULL);
    pthread_cond_init(&g_notify_cond, NULL);

    // set global session handle
    g_session = session;
    g_playback_done = false;
}

void session_release()
{
    if (!g_session)
        exit(EXIT_FAILURE);

    pthread_mutex_destroy(&g_notify_mutex);
    pthread_cond_destroy(&g_notify_cond);

    sp_session_release(g_session);
}

void session_login(const char *username, const char *password)
{
    if (!g_session) {
        debug("Unable to login without valid session\n");
        exit(EXIT_FAILURE);
    }

    sp_session_login(g_session, username, password, 0, NULL);
}

void session_logout()
{
    sp_connectionstate state;
    sp_error error;

    state = sp_session_connectionstate(g_session);
    if (!g_session && state != SP_CONNECTION_STATE_LOGGED_IN)
        exit(EXIT_FAILURE);

    error = sp_session_logout(g_session);
    if (error != SP_ERROR_OK) {
        fprintf(stderr, "Unable to logout: %s\n",
                sp_error_message(error));
        exit(EXIT_FAILURE);
    }
}

sp_search *session_search(const char *query)
{
    sp_search *search;
    // TODO
}

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

    pthread_mutex_lock(&g_notify_mutex);

    g_notify_do = true;
    pthread_cond_signal(&g_notify_cond);

    pthread_mutex_unlock(&g_notify_mutex);
}

static void play_token_lost(sp_session *session)
{
    debug("play_token_lost called\n");

    audio_fifo_flush(&g_audio_fifo);

    // if currently playing a track, stop it.
}

static void log_message(sp_session *session, const char *message)
{
    log_info("%s", message);
}

static void end_of_track(sp_session *session)
{
    debug("end_of_track called\n");

    pthread_mutex_lock(&g_notify_mutex);

    g_playback_done = true;
    g_notify_do = true;
    pthread_cond_signal(&g_notify_cond);

    pthread_mutex_unlock(&g_notify_mutex);
}

/**
 * Callback for when there is decompressed audio data available.
 *
 * @param session active sp_session
 * @param format audio format descriptor (sp_audioformat)
 * @param frames points to raw PCM data as described by format
 * @param num_frames number of available samples in frames.
 *
 * @return 0 if audio discontinuity, the number of frames consumed otherwise
 */
static int music_delivery(sp_session *session,
                          const sp_audioformat *format,
                          const void *frames,
                          int num_frames)
{
    debug("music_delivery called\n");

    audio_fifo_t *af = &g_audio_fifo;    // audio fifo
    audio_data_t *ad;                               // audio data

    // audio discontinuity, flush audio_fifo
    if (num_frames == 0)
        return 0;

    pthread_mutex_lock(&(af->mutex));

    // buffer one second of audio
    if (af->total_samples > format->sample_rate) {
        pthread_mutex_unlock(&(af->mutex));

        return 0;
    }

    // allocate audio_data_t
    ad = audio_data_create(format->channels, num_frames, format->sample_rate);

    // copy frames into samples array
    memcpy(ad->samples, frames, ad->sample_size);

    // enqueue ad
    queue_enqueue(af->queue, ad);

    // update total samples in queue
    af->total_samples += num_frames;    // += nsamples

    pthread_cond_signal(&(af->cond));
    pthread_mutex_unlock(&(af->mutex));

    return num_frames;
}
