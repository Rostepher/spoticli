#ifndef _SPOTIFY_H_
#define _SPOTIFY_H_

#include <stdbool.h>
#include <pthread.h>

extern pthread_mutex_t g_notify_mutex;
extern pthread_cond_t g_notify_cond;
extern bool g_notify_do;


// session 
void session_init();
void session_release();
void session_login(const char *username, const char *password);

bool session_process_events(int *next_timeout);

#endif
