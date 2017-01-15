#ifndef GAMESPECIFIC_2P_H
#define GAMESPECIFIC_2P_H
#include <3ds.h>
#include "gamespecific.h"

struct GameSpecific2P {
	const char* const level_path;
	const char* const ressource_path;
	const u32* const ingame_palette;
	const u8 size_swap_exit;
	const u8* const swap_exit;
};
extern const struct GameSpecific2P import_2p[2];
#endif
