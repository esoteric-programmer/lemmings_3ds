#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "menu.h"
#include "draw.h"
#include "audio.h"
#include "gamespecific.h"
#include "import_main.h"
#include "2p_button.h"

#define MAX(x,y) ((x)>=(y)?(x):(y))

// draw level selection screen to im_bottom
void select_level(
		u8 game,
		struct MainMenuData* menu_data,
		u8 progress[],
		u8 cur_level,
		u8 top_offset,
		const char* level_names) {
	tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);

	u8 rating = cur_level / import[game].num_of_level_per_difficulty;
	char msg[30*(40+1)+1];
	sprintf(msg,"\n  Select a level\n\n\n  Rating %s\n\n",
			import[game].difficulties[rating]);
	char* msg_ptr = msg + strlen(msg);
	u8 i;
	for (i=0;i<24;i++) {
		u8 lvl = i+top_offset;
		u8 level_no;
		const char* level_name;
		if (progress[lvl/import[game].num_of_level_per_difficulty] >=
				lvl%import[game].num_of_level_per_difficulty) {
			level_name = level_names+33*lvl;
		}else{
			level_name = "--------------------------------";
		}
		if (import[game].num_of_level_per_difficulty > 1) {
			level_no = lvl%import[game].num_of_level_per_difficulty+1;
		}else{
			level_no = lvl + 1;
		}
		if (lvl % import[game].num_of_level_per_difficulty == 0 && i != 0) {
			break;
		}
		sprintf(msg_ptr,"%s%02u: %s\n",
				i==cur_level-top_offset?"->":"  ",level_no,level_name);
		msg_ptr += strlen(msg_ptr);
	}
	draw_menu_text(BOTTOM_SCREEN_BACK,menu_data,0,0,msg,0,0.5f);
}

int level_select_menu(
		u8 games[],
		u8* game,
		u8* lvl,
		u8* progress,
		const char* level_names,
		struct MainMenuData* menu_data,
		struct MainInGameData* main_data) {

	// SHOW LEVEL SELECTION SCREEN!
	int redraw_selection = 1;
	int cur_lev = *lvl;
	int top;
	if ((*lvl%import[*game].num_of_level_per_difficulty)>20) {
		top = *lvl-20;
	}else{
		top = import[*game].num_of_level_per_difficulty
				* (*lvl/import[*game].num_of_level_per_difficulty);
	}
	if (top%import[*game].num_of_level_per_difficulty>6) {
		top = import[*game].num_of_level_per_difficulty
				* (top/import[*game].num_of_level_per_difficulty)+6;
	}
	int dwn = 0;
	int up = 0;

	u8 progress_offset = 0;
	u16 level_names_offset = 0;
	u8 i;
	for (i=0;i<*game;i++) {
		progress_offset += import[i].num_of_difficulties;
		level_names_offset += import[i].num_of_difficulties
				* import[i].num_of_level_per_difficulty;
	}
	u8 old_game = *game;

	u32 kDown;
	u32 kHeld;
	while (aptMainLoop()) {

		hidScanInput();
		kDown = hidKeysDown();
		kHeld = hidKeysHeld();
		//kUp = hidKeysUp();
		// hidTouchRead(&stylus);

		if (kHeld & KEY_DOWN) {
			dwn++;
		} else if (!((kDown | kHeld) & KEY_DOWN)) {
			dwn = 0;
		}
		if (kHeld & KEY_UP) {
			up++;
		} else if (!((kDown | kHeld) & KEY_UP)) {
			up = 0;
		}

		if (kDown & KEY_B) {
			if (old_game != *game) {
				*game = old_game;
				if (!read_gamespecific_data(*game, menu_data, main_data)) {
					// error!
					return MENU_ERROR; // error
				}
				if (!update_topscreen(menu_data)) {
					// error!
					clean_gamedata(menu_data, main_data);
					return MENU_ERROR; // error
				}
			}
			// exit selection
			return MENU_ACTION_EXIT;
		}

		if ((kDown & KEY_DOWN) || dwn >= 20) {
			if (dwn) {
				dwn = 18;
			}else{
				dwn = 1;
			}

			if (cur_lev % import[*game].num_of_level_per_difficulty
					< import[*game].num_of_level_per_difficulty-1) {
				cur_lev++;
				if (cur_lev % import[*game].num_of_level_per_difficulty == 0) {
					top = cur_lev;
				}
				if (cur_lev > top+20
						&& cur_lev%import[*game].num_of_level_per_difficulty
							< import[*game].num_of_level_per_difficulty-3) {
					top++;
				}
			}else if (kDown & KEY_DOWN){
				cur_lev = import[*game].num_of_level_per_difficulty
						* (cur_lev/import[*game].num_of_level_per_difficulty);
				top = cur_lev;
			}
			redraw_selection = 1;
		}

		if ((kDown & KEY_UP) || up >= 20) {
			if (up) {
				up = 18;
			}else{
				up = 1;
			}
			if (cur_lev % import[*game].num_of_level_per_difficulty) {
				cur_lev--;
				if (cur_lev % import[*game].num_of_level_per_difficulty
						== import[*game].num_of_level_per_difficulty-1) {
					top = import[*game].num_of_level_per_difficulty
							* (cur_lev/import[*game].num_of_level_per_difficulty)
							+ MAX(import[*game].num_of_level_per_difficulty-24,0);
				}
				if (cur_lev < top+3) {
					if (top % import[*game].num_of_level_per_difficulty != 0) {
						top--;
					}
				}
			}else if (kDown & KEY_UP){
				top = cur_lev + MAX(import[*game].num_of_level_per_difficulty-24,0);
				cur_lev += import[*game].num_of_level_per_difficulty-1;
			}
			redraw_selection = 1;
		}
		if (kDown & KEY_LEFT) {
			if (cur_lev/import[*game].num_of_level_per_difficulty > 0) {
				cur_lev = import[*game].num_of_level_per_difficulty
						* (cur_lev/import[*game].num_of_level_per_difficulty - 1);
				top = cur_lev;
				redraw_selection = 1;
			} else {
				// load previous game??
				if (*game > 0) {

					u8 prev_game = *game-1;
					while(!games[prev_game] && prev_game) {
						prev_game--;
					}
					if (games[prev_game]) {
						// LOAD IT!
						*game = prev_game;
						if (!read_gamespecific_data(*game, menu_data, main_data)) {
							// error!
							return MENU_ERROR; // error
						}
						if (!update_topscreen(menu_data)) {
							// error!
							clean_gamedata(menu_data, main_data);
							return MENU_ERROR; // error
						}
						cur_lev = import[*game].num_of_level_per_difficulty
								* (import[*game].num_of_difficulties-1);
						top = cur_lev;
						redraw_selection = 1;

						progress_offset = 0;
						level_names_offset = 0;
						u8 i;
						for (i=0;i<*game;i++) {
							progress_offset += import[i].num_of_difficulties;
							level_names_offset += import[i].num_of_difficulties
									* import[i].num_of_level_per_difficulty;
						}
					}
				}
			}
		}
		if (kDown & KEY_RIGHT) {
			if (cur_lev/import[*game].num_of_level_per_difficulty
					< import[*game].num_of_difficulties-1) {
				cur_lev = import[*game].num_of_level_per_difficulty
						* (cur_lev/import[*game].num_of_level_per_difficulty + 1);
				top = cur_lev;
				redraw_selection = 1;
			} else {
				// load next game??
				if (*game < LEMMING_GAMES-1) {

					u8 next_game = *game+1;
					while(!games[next_game] && next_game+1 < LEMMING_GAMES) {
						next_game++;
					}
					if (games[next_game]) {
						// LOAD IT!
						*game = next_game;
						if (!read_gamespecific_data(*game, menu_data, main_data)) {
							// error!
							return MENU_ERROR; // error
						}
						if (!update_topscreen(menu_data)) {
							// error!
							clean_gamedata(menu_data, main_data);
							return MENU_ERROR; // error
						}
						cur_lev = 0;
						top = cur_lev;
						redraw_selection = 1;

						progress_offset = 0;
						level_names_offset = 0;
						u8 i;
						for (i=0;i<*game;i++) {
							progress_offset += import[i].num_of_difficulties;
							level_names_offset += import[i].num_of_difficulties
									* import[i].num_of_level_per_difficulty;
						}
					}
				}
			}
		}
		if (kDown & (KEY_A | KEY_START)) {
			if (progress[progress_offset
					+ cur_lev/import[*game].num_of_level_per_difficulty]
					>=cur_lev%import[*game].num_of_level_per_difficulty) {
				// start level!!
				*lvl = cur_lev;
				return MENU_ACTION_LEVEL_SELECTED;
			}
		}

		if (redraw_selection) {
			select_level(
					*game,
					menu_data,
					progress+progress_offset,
					cur_lev,
					top,
					level_names+33*level_names_offset);
			redraw_selection = 0;
		}
		begin_frame();
		copy_from_backbuffer(TOP_SCREEN);
		copy_from_backbuffer(BOTTOM_SCREEN);
		end_frame();
	}
	return MENU_EXIT_GAME;
}

// draw main menu into im_bottom and texture_bot_screen.
int draw_main_menu(
		u8 game,
		u8 difficulty,
		struct MainMenuData* menu_data,
		int only_redraw_difficulty, int only_redraw_music) {
	tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);
	draw(
			BOTTOM_SCREEN_BACK,
			27,
			40,
			menu_data->static_pictures[2]->data,
			menu_data->static_pictures[2]->width,
			menu_data->static_pictures[2]->height,
			menu_data->palette);
	draw(
			BOTTOM_SCREEN_BACK,
			174,
			40,
			menu_data->static_pictures[3]->data,
			menu_data->static_pictures[3]->width,
			menu_data->static_pictures[3]->height,
			menu_data->palette);
	draw(
			BOTTOM_SCREEN_BACK,
			174,
			140,
			menu_data->static_pictures[5]->data,
			menu_data->static_pictures[5]->width,
			menu_data->static_pictures[5]->height,
			menu_data->palette);

	if (import[game].num_of_difficulty_graphics >= import[game].num_of_difficulties
			&& import[game].num_of_difficulties > 1) {
		// draw current difficulty
		draw(
				BOTTOM_SCREEN_BACK,
				174+33,
				140+25,
				menu_data->static_pictures[
						10+import[game].num_of_difficulty_graphics-difficulty
				]->data,
				menu_data->static_pictures[
						10+import[game].num_of_difficulty_graphics-difficulty
				]->width,
				menu_data->static_pictures[
						10+import[game].num_of_difficulty_graphics-difficulty
				]->height,
				menu_data->palette); // x-position: 33; y-position: 25
	}
	draw(
			BOTTOM_SCREEN_BACK,
			27,
			140,
			menu_data->static_pictures[4]->data,
			menu_data->static_pictures[4]->width,
			menu_data->static_pictures[4]->height,
			menu_data->palette);
	draw(
			BOTTOM_SCREEN_BACK,
			27+27,
			140+26,
			settings_icon,
			64,
			31,
			menu_data->palette);

	// draw 2p button
	switch (game) {
		case LEMMINGS_DEMO:
		case ORIGINAL_LEMMINGS:
		case OH_NO_DEMO:
		case OH_NO_MORE_LEMMINGS:
			draw(
					BOTTOM_SCREEN_BACK,
					174+11,
					40+27,
					main_menu_button_2p,
					93,
					27,
					menu_data->palette);
			break;
		case HOLIDAY_LEMMINGS_91:
		case HOLIDAY_LEMMINGS_92:
		case HOLIDAY_93_DEMO:
		case HOLIDAY_LEMMINGS_93:
		case HOLIDAY_94_DEMO:
		case HOLIDAY_LEMMINGS_94:
			draw(
					BOTTOM_SCREEN_BACK,
					174+11,
					40+25,
					main_menu_xmas_button_2p,
					93,
					27,
					menu_data->palette);
			break;
			break;
		default:
			// invalid / not supported
			break;
	}
	
/*
	if (!audio_error() && (settings.sfx_volume || settings.music_volume)) {
		int picture_id = (!settings.music_volume?9:8);
		draw(
				BOTTOM_SCREEN_BACK,
				27+27,
				140+26,
				menu_data->static_pictures[picture_id]->data,
				menu_data->static_pictures[picture_id]->width,
				menu_data->static_pictures[picture_id]->height,
				menu_data->palette);
	}
*/
	return 1;
}

int main_menu(
		u8 games[],
		u8* game,
		u8* lvl,
		struct MainMenuData* menu_data,
		struct MainInGameData* main_data,
		struct SaveGame* savegame) {

	if (!draw_main_menu(*game,
			*lvl/import[*game].num_of_level_per_difficulty,
			menu_data,0,0)) {
		return MENU_ERROR; // error
	}

	u32 kDown;
	u32 kHeld;
	touchPosition stylus;
	while (aptMainLoop()) {
		hidScanInput();
		kDown = hidKeysDown();
		kHeld = hidKeysHeld();
		//kUp = hidKeysUp();
		hidTouchRead(&stylus);
		if (kDown & KEY_TOUCH) {
			if (stylus.py >= 40 && stylus.py < 40+61) {
				// touched at top row
				if (stylus.px >= 27 && stylus.px < 27+120) {
					// touched at game start
					kDown |= KEY_A;
				}
				if (stylus.px >= 174 && stylus.px < 174+120) {
					// touched at level select
					kDown |= KEY_SELECT;
				}
			}else if (stylus.py >= 140 && stylus.py < 140+61) {
				// touched at bottom row
				if (stylus.px >= 27 && stylus.px < 27+120) {
					// touched at left lemming (audio)
					kDown |= KEY_Y;
				}
				if (stylus.px >= 174 && stylus.px < 174+120) {
					// touhed at right lemming (difficulty)
					if (stylus.py < 140+35) {
						kDown |= KEY_UP;
					}else{
						kDown |= KEY_DOWN;
					}
				}
			}
		}

		if ((kHeld & KEY_L) && (kHeld & KEY_R)) {
			return MENU_ACTION_EXIT; // exit
		}

		if (kDown & (KEY_A | KEY_START)) {
			return MENU_ACTION_SELECT_LEVEL_SINGLE_PLAYER; // MENU_ACTION_START_SINGLE_PLAYER;
		}

		if (kDown & (KEY_SELECT | KEY_X)) {
			return MENU_ACTION_START_MULTI_PLAYER; // MENU_ACTION_SELECT_LEVEL_SINGLE_PLAYER;
		}

		if (kDown & KEY_Y) {
			return MENU_ACTION_SETTINGS;
			/*
			int update = !audio_error();
			if (update) {
				if (settings.music_volume) {
					settings.music_volume = 0;
					settings.sfx_volume = 0;
				}else if (settings.sfx_volume) {
					settings.music_volume = 100;
				}else{
					settings.sfx_volume = 100;
				}
				if (savegame) {
					write_savegame(savegame);
				}
				if (!draw_main_menu(*game,
						*lvl/import[*game].num_of_level_per_difficulty,
						menu_data,0,1)) {
					return MENU_ERROR; // error
				}
			}
			*/
		}

		if (kDown & KEY_UP) {
			if (*lvl/import[*game].num_of_level_per_difficulty
					< import[*game].num_of_difficulties-1) {
				*lvl = import[*game].num_of_level_per_difficulty
						* (*lvl/import[*game].num_of_level_per_difficulty)
						+ import[*game].num_of_level_per_difficulty;

				if (!draw_main_menu(*game,
						*lvl/import[*game].num_of_level_per_difficulty,
						menu_data,1,0)) {
					return MENU_ERROR; // error
				}

			}else{
				// select next game
				if (*game < LEMMING_GAMES-1) {

					u8 next_game = *game+1;
					while(!games[next_game] && next_game+1 < LEMMING_GAMES) {
						next_game++;
					}
					if (games[next_game]) {
						// LOAD IT!
						*game = next_game;
						if (!read_gamespecific_data(*game, menu_data, main_data)) {
							// error!
							return MENU_ERROR; // error
						}
						*lvl = 0;
						if (!draw_main_menu(*game,
								*lvl/import[*game].num_of_level_per_difficulty,
								menu_data,0,0)) {
							return MENU_ERROR; // error
						}
						if (!update_topscreen(menu_data)) {
							return MENU_ERROR; // error
						}
						continue;
					}
				}
			}
		}
		if (kDown & KEY_DOWN) {
			if (*lvl/import[*game].num_of_level_per_difficulty > 0) {
				*lvl = import[*game].num_of_level_per_difficulty
						* (*lvl/import[*game].num_of_level_per_difficulty)
						- import[*game].num_of_level_per_difficulty;

				if (!draw_main_menu(*game,
						*lvl/import[*game].num_of_level_per_difficulty,
						menu_data,1,0)) {
					return MENU_ERROR; // error
				}
			}else{
				// select next game
				if (*game > 0) {
					u8 prev_game = *game-1;
					while(!games[prev_game] && prev_game) {
						prev_game--;
					}
					if (games[prev_game]) {
						// LOAD IT!
						*game = prev_game;
						if (!read_gamespecific_data(*game, menu_data, main_data)) {
							// error!
							return MENU_ERROR; // error
						}
						*lvl = import[*game].num_of_level_per_difficulty
								* (import[*game].num_of_difficulties-1);
						if (!draw_main_menu(*game,
								*lvl/import[*game].num_of_level_per_difficulty,
								menu_data,0,0)) {
							return MENU_ERROR; // error
						}
						if (!update_topscreen(menu_data)) {
							return MENU_ERROR; // error
						}
						continue;
					}
				}

			}
		}
		begin_frame();
		copy_from_backbuffer(TOP_SCREEN);
		copy_from_backbuffer(BOTTOM_SCREEN);
		end_frame();
	}
	return MENU_EXIT_GAME;
}
