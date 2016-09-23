#define SAVEGAME_VERSION 1
#include <3ds.h>
// progress: u8 array with one entry for each difficulty of each supported lemmings game
void read_savegame(u8* progress);
void write_savegame(u8* progress);
