#ifndef IMPORT_MAIN_H
#define IMPORT_MAIN_H
#include <3ds.h>
#include "main_data.h"

int read_main_ingame(u8 game, struct MainInGameData* data);
void free_ingame_data_arrays(struct MainInGameData* data); // free memory pointed to by lemmings_anim and masks

int read_main_menu(u8 game, struct MainMenuData* data);
void free_menu_data_arrays(struct MainMenuData* data); // free memory pointed to by static_pictures
#endif
