#ifndef AUDIO_H
#define AUDIO_H
#include <3ds.h>

#define AUDIO_ERROR 0
#define AUDIO_DISABLED 1
#define AUDIO_ONLY_FX 2
#define AUDIO_ENABLED 3

extern u8 cur_song;
extern u8 audio_active;

void init_audio();
int read_adlib_dat(u8 game);
void start_music(u8 track);
void stop_audio();
void play_sound(u8 sound);
void update_audio();
void deinit_audio();

#endif
