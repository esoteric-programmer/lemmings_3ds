#ifndef SAVEGAME_H
#define SAVEGAME_H
#define SAVEGAME_VERSION 2
#include <3ds.h>

struct SaveGame {
	// u8 array with one entry for each difficulty
	// of each supported lemmings game
	u8* progress;

	// audio_settings: 0 default; 1 disabled; 2 only fx; 3 enabled
	u8 audio_settings;
};

void read_savegame(struct SaveGame* savegame);
void write_savegame(struct SaveGame* savegame);
#endif
