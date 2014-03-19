#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include "spoticli.h"
#include "session.h"
#include "audio.h"

#define DEBUG
#include "debug.h"


// global initalizations
sp_session *g_session = NULL;
audio_fifo_t g_audio_fifo;
pthread_mutex_t g_notify_mutex;
pthread_cond_t g_notify_cond;
bool g_notify_do = false;
bool g_playback_done = false;


int main(int argc, char **argv)
{
    int next_timeout = 0;

    // instantiate global session handle
    session_init();

    // init and lock mutex
    pthread_mutex_init(&g_notify_mutex, NULL);
    pthread_cond_init(&g_notify_cond, NULL);
    
    // log in
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

    // release session
    session_release();

    return EXIT_SUCCESS;
}
