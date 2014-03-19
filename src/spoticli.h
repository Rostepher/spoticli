#ifndef _SPOTICLI_H_
#define _SPOTICLI_H_

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <libspotify/api.h>

#include "audio.h"


#define CLIENT_NAME         "spoticli"
#define CACHE_LOCATION      "tmp"
#define SETTINGS_LOCATION   "tmp"


// globals
extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;
extern const char *g_username;
extern const char *g_password;

extern sp_session *g_session;
extern audio_fifo_t g_audio_fifo;
extern pthread_mutex_t g_notify_mutex;
extern pthread_cond_t g_notify_cond;
extern bool g_notify_do;
extern bool g_playback_done;

#endif
