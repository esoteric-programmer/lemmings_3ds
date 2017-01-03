#ifndef AUDIO_H
#define AUDIO_H
#include <3ds.h>

void init_audio();
void update_volume();
int audio_error();
int import_audio(u8 game);
int is_custom_sound(u8 sound);
void next_music();
void prepare_music(u8 game, u8 lvl);
void play_music();
void stop_audio();
void play_sound(u8 sound);
void update_audio();
void deinit_audio();
#endif
