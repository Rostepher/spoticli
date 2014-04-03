#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "audio.h"
#include "session.h"

#define DEBUG
#include "debug.h"


// externals ///////////////////////////////////////////////////////////////////
extern const char *g_username;
extern const char *g_password;
extern sp_session *g_session;
extern pthread_mutex_t g_notify_mutex;
extern pthread_cond_t g_notify_cond;
extern bool g_notify_do;
extern bool g_playback_done;
extern bool g_is_logged_in;


// function prototypes /////////////////////////////////////////////////////////
static void sigint_handler(int sig);


// main ////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    int next_timeout = 0;

    signal(SIGINT, sigint_handler);
    
    session_init();
    session_login(g_username, g_password);

    pthread_mutex_lock(&g_notify_mutex);

    while (true) {
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
            sp_session_process_events(g_session, &next_timeout);
        } while (next_timeout == 0);

        pthread_mutex_lock(&g_notify_mutex);
    }

    session_logout();
    session_release();

    return EXIT_SUCCESS;
}

static void sigint_handler(int sig)
{
    debug("SIGINT caught\n");

    if (g_is_logged_in)
        session_logout();

    // if session exists release it
    if (g_session)
        session_release();
    
    exit(sig);
}
