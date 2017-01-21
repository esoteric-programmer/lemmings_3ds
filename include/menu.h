#ifndef MENU_H
#define MENU_H
#include <3ds.h>
#include "draw.h"
#include "import_main.h"
#include "savegame.h"

#define MENU_ERROR                              0
#define MENU_ACTION_EXIT                        1
#define MENU_ACTION_START_SINGLE_PLAYER         2
#define MENU_ACTION_SELECT_LEVEL_SINGLE_PLAYER  3
#define MENU_ACTION_SETTINGS                    4
#define MENU_ACTION_LEVEL_SELECTED              5
#define RESULT_ACTION_NEXT                      6
#define RESULT_ACTION_CANCEL                    7
#define MENU_ACTION_START_MULTI_PLAYER          8
#define MENU_HOST_REJECT_CLIENT                 9
#define MENU_CLIENT_QUIT                       10
#define MENU_EXIT_GAME                        127

int main_menu(
		u8 games[],
		u8* game,
		u8* lvl,
		struct MainMenuData* menu_data,
		struct MainInGameData* main_data,
		struct SaveGame* savegame);

int level_select_menu(
		u8 games[],
		u8* game,
		u8* lvl,
		u8* progress,
		const char* level_names,
		struct MainMenuData* menu_data,
		struct MainInGameData* main_data);

#endif
