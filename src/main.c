#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "audio.h"
#include "session.h"
#include "ui.h"

#define DEBUG
#include "debug.h"


// externals ///////////////////////////////////////////////////////////////////
extern const char *g_username;
extern const char *g_password;
extern session_t *g_session;

// function prototypes /////////////////////////////////////////////////////////
static void cleanup();
static void sigint_handler(int sig);
static void sigterm_handler(int sig);
static void sigsegv_handler(int sig);


// main ////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    int next_timeout = 0;

    // register signal handlers
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGSEGV, sigsegv_handler);
    
    // initialize session
    session_init();

    // initialize ui
    ui_init();

    // login to spotify
    session_login(g_username, g_password);

    pthread_mutex_lock(&(g_session->notify_mutex));

    while (true) {
        if (next_timeout == 0) {
            while (!g_session->notify_do)
                pthread_cond_wait(&(g_session->notify_cond), &(g_session->notify_mutex));
        } else {
            // current time
            struct timespec ts;

            // convert the next_timeout to a real time, realative to current time
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += next_timeout / 1000;
            ts.tv_nsec += (next_timeout % 1000) * 1E6;

            pthread_cond_timedwait(&(g_session->notify_cond), &(g_session->notify_mutex), &ts);
        }

        pthread_mutex_unlock(&(g_session->notify_mutex));

        do {
            sp_session_process_events(g_session->sp_session, &next_timeout);
        } while (next_timeout == 0);

        pthread_mutex_lock(&(g_session->notify_mutex));
    }

    // exit ui
    ui_release();

    cleanup();

    return EXIT_SUCCESS;
}

static void cleanup()
{
    if (g_session) {
        if (g_session->state == SESSION_ONLINE)
            session_logout();

        session_release();
    }
}

static void sigint_handler(int sig)
{
    debug("SIGINT caught\n");
    cleanup();
    exit(sig);
}

static void sigterm_handler(int sig)
{
    debug("SIGTERM caught\n");
    cleanup();
    exit(sig);
}

static void sigsegv_handler(int sig)
{
    debug("SIGSEGV caught\n");
    cleanup();
    exit(sig);
}
