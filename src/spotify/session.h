#ifndef SPOTICLI_SPOTIFY_SESSION_H
#define SPOTICLI_SPOTIFY_SESSION_H

#include <pthread.h>
#include <libspotify/api.h>

#include "audio.h"

void session_init();
void session_release();
void session_login(const char *username, const char *password);
void session_logout();

#endif // SPOTICLI_SPOTIFY_SESSION_H
