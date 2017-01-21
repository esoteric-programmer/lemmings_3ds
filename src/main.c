#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <3ds.h>

#include "decode.h"
#include "import_level.h"
#include "import_main.h"
#include "draw.h"
#include "control.h"
#include "settings.h"
#include "gamespecific.h"
#include "gamespecific_2p.h"
#include "savegame.h"
#include "ingame.h"
#include "audio.h"
#include "menu.h"
#include "main.h"
#include "settings_menu.h"
#include "network_menu.h"

const char* PATH_ROOT = LEMMINGS_DIR;

volatile int suspended = 0;
aptHookCookie hookCookie;

int was_suspended() {
	if (suspended) {
		suspended = 0;
		return 1;
	}
	return 0;
}

void event_hook(APT_HookType hook_type, void* param) {
	switch (hook_type) {
		case APTHOOK_ONRESTORE:
		case APTHOOK_ONWAKEUP:
			suspended = 1;
			break;
		case APTHOOK_ONSUSPEND:
			break;
		default:
			break;
	}
}

// TODO:
// make xmas91 and xmas92 distinguishable in main menu (maybe add info to top screen)

/*
	original order of actions (not taken care of yet):

	process_skill_assignment
	add_new_lemming
	update_lemmings
*/

void die() {
	#ifndef NO_SF2D
	int i;
	for (i=0;i<4;i++) {
		begin_frame();
		clear(TOP_SCREEN);
		clear(BOTTOM_SCREEN);
		end_frame();
	}
	#endif
	consoleInit(GFX_TOP, NULL);
	consoleClear();
	printf(CONSOLE_RED);
	printf("\n                An error occured.\n\n");
	printf(CONSOLE_RESET);
	printf("Maybe some file of the original Lemmings game you\n");
	printf("tried to load is missing.\n");
	printf("Please make sure that ");
	printf(CONSOLE_RED);
	printf("all");
	printf(CONSOLE_RESET);
	printf(" files are in the\n");
	printf("correct directory and that no file is corrupted.\n\n\n");
	printf("Directories for DOS Lemmings games:\n\n");
	printf("Original Lemmings           /lemmings/orig\n");
	printf("Original Lemmings Demo      /lemmings/orig_demo\n");
	printf("Oh No! More Lemmings        /lemmings/ohno\n");
	printf("Oh No! More Lemmings Demo   /lemmings/ohno_demo\n");
	printf("Xmas Lemmings 1991          /lemmings/xmas91\n");
	printf("Xmas Lemmings 1992          /lemmings/xmas91\n");
	printf("Holiday Lemmings 1993       /lemmings/holi93\n");
	printf("Holiday Lemmings 1993 Demo  /lemmings/holi93_demo\n");
	printf("Holiday Lemmings 1994       /lemmings/holi94\n");
	printf("Holiday Lemmings 1994 Demo  /lemmings/holi94_demo\n\n\n");
	printf(CONSOLE_RED);
	printf("Besides that, this error may occur when the game\n");
	printf("runs out of memory.\n\n");
	printf(CONSOLE_RESET);
	printf("Press any button to exit.");
	while (aptMainLoop()) {
		hidScanInput();
		u32 kDown = hidKeysDown();
		if (kDown & (~KEY_TOUCH)) {
			break;
		}
		#ifdef NO_SF2D
		begin_frame();
		clear(BOTTOM_SCREEN);
		end_frame();
		#else
		gfxFlushBuffers();
		gfxSwapBuffers();
		#endif
	}
	// exit game
	aptUnhook(&hookCookie);
	gfxExit();
	exit(1);
}

int main() {
	// INITIALIZATION

	int i;

	gfxInit(GSP_BGR8_OES,  GSP_BGR8_OES, false);
	consoleInit(GFX_TOP, NULL);
	consoleClear();
	printf("\n");
	printf("       ----- Lemmings for 3DS Loading ----- \n");
	printf("       ----------- Please Wait ------------ \n");
	gfxFlushBuffers();
	gfxSwapBuffers();

	aptHook(&hookCookie, event_hook, NULL);

	u8 games[LEMMING_GAMES];
	memset(games,0,LEMMING_GAMES);
	u8 game = 0;
	u8 lvl = 0;

	// count number of levels
	u16 overall_num_of_levels = 0;
	u16 overall_num_of_difficulties = 0;
	for (i=0;i<LEMMING_GAMES;i++) {
		overall_num_of_levels +=
				(u16)import[i].num_of_difficulties *
				(u16)import[i].num_of_level_per_difficulty;
		overall_num_of_difficulties += import[i].num_of_difficulties;
	}


	// test whether PATH_ROOT is a directory, otherwise set it to "."
	struct stat s;
	int err = stat(PATH_ROOT, &s);
	if(err == -1) {
		PATH_ROOT = ".";
	}else{
		if(!S_ISDIR(s.st_mode)) {
			PATH_ROOT = ".";
		}
	}

	// read level names
	char* level_names = (char*)malloc(33*overall_num_of_levels);
	if (!level_names) {
		die(0);
		return 0; // error
	}
	u16 offset = 0;
	for (i=0;i<LEMMING_GAMES;i++) {
		games[i] = read_level_names(i, level_names + offset);
		offset += 33*
				(u16)import[i].num_of_difficulties *
				(u16)import[i].num_of_level_per_difficulty;
	}

	// read save file
	u8* progress = (u8*)malloc(overall_num_of_difficulties);
	if (!progress) {
		die(0); // error
	}
	struct SaveGame savegame;
	memset(&savegame,0,sizeof(struct SaveGame));
	savegame.progress = progress;
	read_savegame(&savegame);
	if (savegame.last_game < LEMMING_GAMES) {
		if (games[savegame.last_game]) {
			game = savegame.last_game;
			if (savegame.last_level <
					import[game].num_of_difficulties
					* import[game].num_of_level_per_difficulty) {
				lvl = savegame.last_level;
			}
		}
	}

	// find first game the LEVEL files of which have been scanned successfully
	// and start with this game
	while (!games[game] && game < LEMMING_GAMES) {
		game++;
	}
	if (game == LEMMING_GAMES) {
		// no game data has been found
		// display error message
		printf(CONSOLE_RED);
		printf("\nTo run this game, you need to copy the files of\n");
		printf("at least one DOS Lemmings game to your SD card.\n");
		printf("Please make sure to copy the data into the\n");
		printf("correct directory corresponding to the game.\n\n\n");
		printf(CONSOLE_RESET);
		printf("Directories for DOS Lemmings games:\n\n");
		printf("Original Lemmings           /lemmings/orig\n");
		printf("Original Lemmings Demo      /lemmings/orig_demo\n");
		printf("Oh No! More Lemmings        /lemmings/ohno\n");
		printf("Oh No! More Lemmings Demo   /lemmings/ohno_demo\n");
		printf("Xmas Lemmings 1991          /lemmings/xmas91\n");
		printf("Xmas Lemmings 1992          /lemmings/xmas91\n");
		printf("Holiday Lemmings 1993       /lemmings/holi93\n");
		printf("Holiday Lemmings 1993 Demo  /lemmings/holi93_demo\n");
		printf("Holiday Lemmings 1994       /lemmings/holi94\n");
		printf("Holiday Lemmings 1994 Demo  /lemmings/holi94_demo\n\n\n");
		printf("You may check out the \"demos\" folder at\n");
		printf(CONSOLE_GREEN);
		printf("http://github.com/esoteric-programmer/lemmings_3ds");
		printf(CONSOLE_RESET);
		printf("to download free demo versions of Lemmings.\n\n");
		printf("Press any button to exit.");

		while (aptMainLoop()) {
			hidScanInput();
			u32 kDown = hidKeysDown();
			if (kDown & (~KEY_TOUCH)) {
				break;
			}
			gfxFlushBuffers();
			gfxSwapBuffers();
		}
		gfxExit();
		return 0; // error
	}


	// read 2p level names
	u8 num_2p_level[2] = {
			count_custom_levels(import_2p[0].level_path),
			count_custom_levels(import_2p[1].level_path)};
	char* name_2p_level = 0;
	if (num_2p_level[0] || num_2p_level[1]) {
		name_2p_level = (char*)malloc(33*(u16)(num_2p_level[0]+num_2p_level[1]));
		if (name_2p_level[0]) {
			if (!read_level_names_from_path(import_2p[0].level_path, &num_2p_level[0], name_2p_level)) {
				name_2p_level[0] = 0;
			}
		}
		if (name_2p_level[1]) {
			if (!read_level_names_from_path(import_2p[1].level_path, &num_2p_level[1], &name_2p_level[33*(u16)num_2p_level[0]])) {
				name_2p_level[1] = 0;
			}
		}
	}


	// initialize variables
	struct MainMenuData* menu_data =
			(struct MainMenuData*)malloc(sizeof(struct MainMenuData));
	if (!menu_data) {
		die(0); // error
	}
	memset(menu_data,0,sizeof(struct MainMenuData));

	struct MainInGameData* main_data =
			(struct MainInGameData*)malloc(sizeof(struct MainInGameData));
	if (!main_data) {
		die(0); // error
	}
	memset(main_data,0,sizeof(struct MainInGameData));

	// initialize drawing buffers, set gfx mode, and so on...
	init_drawing();

	init_audio();
	if (!read_gamespecific_data(game, menu_data, main_data)) {
		// error!
		die(1); // error
	}
	if (!update_topscreen(menu_data)) {
		// error!
		die(1); // error
	}


	// MAIN LOOP
	while(1) {
		int menu_selection = main_menu(
				games,
				&game,
				&lvl,
				menu_data,
				main_data,
				&savegame);
		if (menu_selection == MENU_ACTION_SELECT_LEVEL_SINGLE_PLAYER) {
			int level_selection = level_select_menu(
					games,
					&game,
					&lvl,
					savegame.progress,
					level_names,
					menu_data,
					main_data);
			switch (level_selection) {
				case MENU_EXIT_GAME:
					menu_selection = MENU_EXIT_GAME;
					break;
				case MENU_ACTION_EXIT:
					break;
				case MENU_ACTION_LEVEL_SELECTED:
					menu_selection = MENU_ACTION_START_SINGLE_PLAYER;
					break;
				case MENU_ERROR:
				default:
					die(1); // error
			}
		}
		if (menu_selection == MENU_ACTION_SETTINGS) {
			int result = settings_menu(&savegame, menu_data);
			switch (result) {
				case MENU_EXIT_GAME:
					menu_selection = MENU_EXIT_GAME;
					break;
				case MENU_ACTION_EXIT:
					break;
				case MENU_ERROR:
				default:
					die(1); // error
			}
		}
		if (menu_selection == MENU_ACTION_START_MULTI_PLAYER) {
			//int result =
			int result;
			do {
				result = network_menu(&savegame, num_2p_level, name_2p_level, menu_data, main_data);
				switch (result) {
					case MENU_EXIT_GAME:
						menu_selection = MENU_EXIT_GAME;
						break;
					case MENU_ACTION_EXIT:
						break;
					case MENU_ERROR:
					default:
						die(1); // error
				}
			}while(result == MENU_ACTION_START_MULTI_PLAYER);
		}

		if (menu_selection == MENU_ERROR) {
			die(1); // error
		}

		if (menu_selection == MENU_ACTION_START_SINGLE_PLAYER) {
			if (savegame.last_game != game || savegame.last_level != lvl) {
				savegame.last_game = game;
				savegame.last_level = lvl;
				write_savegame(&savegame);
			}
			while(1) {
				char level_id[32];
				struct Level* level = init_level_from_dat(game, lvl, level_id);
				if (!level) {
					die(1);
					break;
				}
				struct LevelResult lev_result = run_level(level, level_id, menu_data, main_data);
				free_objects(level->object_types);
				free(level);
				if (lev_result.exit_reason == LEVEL_ERROR) {
					// an error occured
					// error code may be coded in lev_result.lvl
					die(1);
					break;
				}
				if (lev_result.exit_reason == LEVEL_EXIT_GAME) {
					menu_selection = MENU_EXIT_GAME;
					break;
				}

				// process result
				lev_result.lvl = lvl;
				// find out whether level has been solved successful
				if (lev_result.percentage_rescued >= lev_result.percentage_needed) {
					// increment lvl counter to read next level
					u8 progress_offset = 0;
					u8 i;
					for (i=0;i<game;i++) {
						progress_offset += import[i].num_of_difficulties;
					}
					progress_offset += lvl/import[game].num_of_level_per_difficulty;

					if (progress[progress_offset]
							< lvl%import[game].num_of_level_per_difficulty+1) {
						progress[progress_offset] =
								lvl%import[game].num_of_level_per_difficulty+1;
					}
					lvl = (lvl+1) % (import[game].num_of_difficulties
							* import[game].num_of_level_per_difficulty);
					savegame.last_level = lvl;
					write_savegame(&savegame);
				}

				int result_screen = show_result(game,lev_result,menu_data);
				if (result_screen == RESULT_ACTION_NEXT) {
					continue;
				}
				if (result_screen == RESULT_ACTION_CANCEL) {
					break;
				}
				if (result_screen == MENU_EXIT_GAME) {
					menu_selection = MENU_EXIT_GAME;
					break;
				}
				die(1); // error
			}
		}

		if (menu_selection == MENU_ACTION_EXIT || menu_selection == MENU_EXIT_GAME) {
			break;
		}
	}
	aptUnhook(&hookCookie);
	deinit_audio();
	gfxExit();
	return 0;
}

