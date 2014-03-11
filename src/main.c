#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <libspotify/api.h>


#define DEBUG
#include "debug.h"


extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;
extern const char *username;
extern const char *password;


// session callback prototypes
static void logged_in(sp_session *session, sp_error error);
static void logged_out(sp_session *session);
static void notify_main_thread(sp_session *session);
static void log_message(sp_session *session, const char *message);
static void end_of_track(sp_session *session);
static int music_delivery(sp_session *session,
                              const sp_audioformat *format,
                              const void *frames,
                              int num_frames);


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
    .cache_location         = "",
    .settings_location      = "",
    .application_key        = g_appkey,
    .application_key_size   = 0,                    // set in main
    .user_agent             = "spoticli",
    .callbacks              = &session_callbacks,
    NULL                    // TODO
};


// session callback definitions
static void logged_in(sp_session *session, sp_error error)
{
    debug("logged_in called");
}

static void logged_out(sp_session *session)
{
    debug("logged_out called");
}

static void notify_main_thread(sp_session *session)
{
    debug("notify_main_thread called");
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


int main(int argc, char **argv)
{
    sp_error error;
    sp_session *session;

    // create spotify session
    session_config.application_key_size = g_appkey_size;
    error = sp_session_create(&session_config, &session);
    if (error != SP_ERROR_OK) {
        log_error("Unable to create session: %s\n", sp_error_message(error));
        exit(EXIT_FAILURE);
    }

    debug("username: %s", username);
    debug("password: %s", password);

    // log in
    sp_session_login(session, username, password, 0, NULL);

    printf("logged in?: %s\n", (sp_session_connectionstate(session) == SP_CONNECTION_STATE_LOGGED_IN ? "yes" : "no"));

    return EXIT_SUCCESS;
}
