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

#define CLIENT "spoticli"


// values from appkey.c
extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;
extern const char *username;
extern const char *password;

// globals
static sp_session *g_session = NULL;
static pthread_mutex_t g_notify_mutex;
static pthread_cond_t g_notify_cond;
static bool g_notify_do = false;


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

static void notify_main_thread(sp_session *session)
{
    debug("notify_main_thread called");
    
    pthread_mutex_lock(&g_notify_mutex);
    g_notify_do = true;
    pthread_cond_signal(&g_notify_cond);
    pthread_mutex_unlock(&g_notify_mutex);
}

static void log_message(sp_session *session, const char *message)
{
    debug("log_message called");
    log_info("%s", message);
}

static void end_of_track(sp_session *session)
{
    debug("end_of_track called");
}

static int music_delivery(sp_session *session,
                           const sp_audioformat *format,
                           const void *frames,
                           int num_frames)
{
    debug("music_delivery called");
    return 1;
}

static sp_session_callbacks session_callbacks = {
    .logged_in          = &logged_in,
    .logged_out         = &logged_out,
    .notify_main_thread = &notify_main_thread,
    .music_delivery     = &music_delivery,
    .log_message        = &log_message,
    .end_of_track       = &end_of_track
};

static sp_session_config session_config = {
    .api_version            = SPOTIFY_API_VERSION,
    .cache_location         = "tmp",
    .settings_location      = "tmp",
    .application_key        = g_appkey,
    .application_key_size   = 0,                    // set in main
    .user_agent             = "spoticli",
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
    sp_session_login(session, username, password, 0, NULL);
    
    g_session = session;

    pthread_mutex_lock(&g_notify_mutex);

    do {
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
    } while (true);

    sp_session_release(session);

    return EXIT_SUCCESS;
}
