#ifndef SAVEGAME_H
#define SAVEGAME_H
#define SAVEGAME_VERSION 5
#include <3ds.h>

struct SaveGame {
	// u8 array with one entry for each difficulty
	// of each supported lemmings game
	u8* progress;
	u8 multiplayer_progress[2];
	u8 last_game;
	u8 last_level;
};

void read_savegame(struct SaveGame* savegame);
void write_savegame(struct SaveGame* savegame);
#endif
