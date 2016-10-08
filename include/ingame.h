#ifndef INGAME_H
#define INGAME_H
#include <3ds.h>
#include <sf2d.h>
#include "level.h"
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

// returns: percentage saved
struct LevelResult run_level(u8 game, u8 lvl,
		struct MainInGameData* main_data,
		sf2d_texture* texture_top_screen, sf2d_texture* texture_logo);

int show_result(u8 game, struct LevelResult result,
		struct MainMenuData* menu_data,
		sf2d_texture** texture_logo, sf2d_texture** texture_top_screen);

#endif
