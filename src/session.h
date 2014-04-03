#ifndef _SESSION_H_
#define _SESSION_H_

#include <libspotify/api.h>

void session_init();
void session_release();
void session_login(const char *username, const char *password);
void session_logout();

void session_search(const char *query);
void session_play(sp_track *track);
void session_pause();
void session_stop();

#endif
