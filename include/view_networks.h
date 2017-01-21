#ifndef VIEW_NETWORKS_H
#define VIEW_NETWORKS_H
#include <3ds.h>
#include "menu.h"
#include "savegame.h"

int host_game(struct SaveGame* savegame, u8 num_levels[2], char* level_names, struct MainMenuData* menu_data, struct MainInGameData* main_data);
int connect_to_network(const udsNetworkScanInfo* scan_info, struct MainMenuData* menu_data, struct MainInGameData* main_data);
#endif
