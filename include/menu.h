#ifndef MENU_H
#define MENU_H
#include <3ds.h>
#include "draw.h"
#include "import_main.h"

#define MENU_ERROR                              0
#define MENU_ACTION_EXIT                        1
#define MENU_ACTION_START_SINGLE_PLAYER         2
#define MENU_ACTION_SELECT_LEVEL_SINGLE_PLAYER  3
#define MENU_ACTION_SETTINGS                    4 // not implemented yet
#define MENU_ACTION_LEVEL_SELECTED              5
#define RESULT_ACTION_NEXT                      6
#define RESULT_ACTION_CANCEL                    7

int main_menu(u8 games[], u8* game, int* lvl,
		struct MainMenuData* menu_data, struct MainInGameData* main_data,
		struct RGB_Image* im_top, struct RGB_Image* logo_scaled,
		sf2d_texture** texture_logo, sf2d_texture** texture_top_screen);

int level_select_menu(u8 games[], u8* game, int* lvl, u8* progress, const char* level_names,
		struct MainMenuData* menu_data, struct MainInGameData* main_data,
		struct RGB_Image* im_top, struct RGB_Image* logo_scaled,
		sf2d_texture** texture_logo, sf2d_texture** texture_top_screen);

#endif
