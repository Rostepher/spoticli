#ifndef SPOTICLI_SESSION_H
#define SPOTICLI_SESSION_H

#include <pthread.h>
#include <stdbool.h>

#include <libspotify/api.h>

#include "audio.h"

typedef enum session_state {
    SESSION_OFFLINE,
    SESSION_ONLINE,
    SESSION_LOGGED_IN,
    SESSION_LOGGED_OUT,
    SESSION_ERROR
} session_state_t;

typedef struct session_s {
    session_state_t state;
    sp_session *sp_session; // spotify session handle

    pthread_mutex_t notify_mutex;
    pthread_cond_t notify_cond;

    audio_fifo_t audio_fifo;

    bool notify_do;
    bool playback_done;
} session_t;

void session_init();
void session_release();
void session_login(const char *username, const char *password);
void session_logout();

void session_search(const char *query);
void session_play(sp_track *track);
void session_pause();
void session_stop();

#endif
