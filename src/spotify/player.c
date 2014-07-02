#include "player.h"
#include "audio.h"

extern sp_session *g_session;
extern audio_fifo_t g_audio_fifo;

/**
 *
 */
void player_play(sp_track *track) {
    if (!track)
        sp_session_player_load(g_session, track);

    sp_session_player_play(g_session, true);
}

/**
 *
 */
void player_pause() {
    // false translates to pause currently loaded track
    sp_session_player_play(g_session, false);
}

/**
 *
 */
void player_seek(int offset) {
    sp_session_player_seek(g_session, offset);
}

/**
 *  Stops the currently loaded track, then flushes the global audio fifo
 *  structure.
 */
void player_stop() {
    sp_session_player_unload(g_session);
    audio_fifo_flush(&g_audio_fifo);
}
