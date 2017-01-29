#ifndef NETWORK_RUN_LEVEL_H
#define NETWORK_RUN_LEVEL_H
#include <3ds.h>
#include "level.h"
#include "main_data.h"

int server_prepare_level(
		udsBindContext* bindctx,
		const u8* lemmings, // number of lemmings the players start with
		u8 game_id,
		u8 level_id,
		struct Level* output);

// important: overwrites settings.glitch_direct_drop.
// therefore the local value has to be stored before this function is called
int client_prepare_level(
		udsBindContext* bindctx,
		const u8* lemmings, // number of lemmings the players start with
		u8* lvl_id,
		u8 game_id,
		struct Level* output);

int server_run_level(
		udsBindContext* bindctx,
		struct Level* level,
		const char* level_id,
		u8* lemmings, // number of lemmings the players have rescued
		struct MainMenuData* menu_data,
		struct MainInGameData* main_data);
int client_run_level(
		udsBindContext* bindctx,
		struct Level* level,
		const char* level_id,
		u8* lemmings,
		u16* won,
		struct MainMenuData* menu_data,
		struct MainInGameData* main_data);

int server_send_result(
		udsBindContext* bindctx,
		u8 lemmings[2],
		u16 won[2]);

#define CHUNK_SIZE (UDS_DATAFRAME_MAXSIZE - sizeof(struct NW_LevelData_Chunk))
#endif
