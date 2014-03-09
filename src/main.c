#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <libspotify/api.h>


#define DEBUG
#include "debug.h"


extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;
extern const char *username;
extern const char *password;


// session callback prototypes
static void on_log_in(sp_session *session, sp_error error);
static void on_log_out(sp_session *session);
static void on_notify_main_thread(sp_session *session);
static void on_log_message(sp_session *session, const char *message);
static void on_end_of_track(sp_session *session);
static int on_music_delivered(sp_session *session,
                              const sp_audioformat *format,
                              const void *frames,
                              int num_frames);


static sp_session_callbacks session_callbacks = {
    .logged_in          = &on_log_in,
    .logged_out         = &on_log_out,
    .notify_main_thread = &on_notify_main_thread,
    .music_delivery     = &on_music_delivered,
    .log_message        = &on_log_message,
    .end_of_track       = &on_end_of_track
};

static sp_session_config session_config = {
    .api_version            = SPOTIFY_API_VERSION,
    .cache_location         = "tmp/spoticli",
    .settings_location      = "tmp/spoticli",
    .application_key        = g_appkey,
    .application_key_size   = 0,                    // set in main
    .user_agent             = "spoticli",
    .callbacks              = &session_callbacks,
    NULL                    // TODO
};


// session callback definitions
static void on_log_in(sp_session *session, sp_error error)
{
    debug("on_log_in called");
}

static void on_log_out(sp_session *session)
{
    debug("on_log_out called");
}

static void on_notify_main_thread(sp_session *session)
{
    debug("on_notify_main_thread called");
}

static void on_log_message(sp_session *session, const char *message)
{
    debug("on_log_message called");
}

static void on_end_of_track(sp_session *session)
{
    debug("on_end_of_track called");
}

static int on_music_delivered(sp_session *session,
                              const sp_audioformat *format,
                              const void *frames,
                              int num_frames)
{
    debug("on_music_delivered called");
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
        return EXIT_FAILURE;
    }
    
    // log in
    error = sp_session_login(session, username, password, 0, NULL);
    if (error != SP_ERROR_OK) {
        log_error("Unable to logging in: %s\n", sp_error_message(error));
        return EXIT_FAILURE;
    }

    // log out
    error = sp_session_logout(session);
    if (error != SP_ERROR_OK) {
        log_error("Unable to logging out: %s\n", sp_error_message(error));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
