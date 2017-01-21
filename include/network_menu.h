#ifndef NETWORK_MENU_H
#define NETWORK_MENU_H
#include <3ds.h>
#include "import_main.h"
#include "savegame.h"

int get_aligned_username(char username[41+2*6], const udsNodeInfo* nodeinfo);
int network_menu(struct SaveGame* savegame, u8 num_levels[2], char* level_names, struct MainMenuData* menu_data, struct MainInGameData* main_data);
int show_network_error(struct MainMenuData* menu_data);
#endif
