#ifndef INGAME_H
#define INGAME_H
#include <3ds.h>
#include "level.h"
#include "control.h"
#include "main_data.h"
#include "draw.h"

#define LEVEL_ERROR              0
#define LEVEL_TIMEOUT            1
#define LEVEL_NO_LEMMINGS_LEFT   2
#define LEVEL_EXIT_GAME        127


struct LevelResult {
	u8 lvl;
	u8 percentage_rescued;
	u8 percentage_needed;
	u8 exit_reason;
};

int level_step(
		struct MainInGameData* main_data,
		struct Level* level,
		// *lemming_inout = 1, iff a any lemming enters or exits the level (without dying)
		u8* lemming_inout); 

void render_level_frame(
		const char* level_id, // e.g. FUN 14
		struct MainInGameData* main_data,
		struct Level* level,
		struct InputState* io_state,
		u8 player);

// returns: percentage saved
struct LevelResult run_level(
		struct Level* level,
		const char* level_id,
		struct MainMenuData* menu_data,
		struct MainInGameData* main_data);

int show_result(
		u8 game,
		struct LevelResult result,
		struct MainMenuData* menu_data);

#endif
