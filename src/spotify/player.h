#ifndef SPOTICLI_SPOTIFY_PLAYER_H
#define SPOTICLI_SPOTIFY_PLAYER_H

#include <libspotify/api.h>

void player_play(sp_track *track);
void player_pause();
void player_seek(int offset);
void player_stop();

#endif // SPOTICLI_SPOTIFY_PLAYER_H
