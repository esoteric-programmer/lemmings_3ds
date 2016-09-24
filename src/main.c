#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <3ds.h>
#include <sf2d.h>

#include "decode.h"
#include "import.h"
#include "import_level.h"
#include "import_main.h"
#include "draw.h"
#include "control.h"
#include "settings.h"
#include "gamespecific.h"
#include "savegame.h"
#include "ingame.h"

#define MENU_ERROR                              0
#define MENU_ACTION_EXIT                        1
#define MENU_ACTION_START_SINGLE_PLAYER         2
#define MENU_ACTION_SELECT_LEVEL_SINGLE_PLAYER  3
#define MENU_ACTION_SETTINGS                    4 // not implemented yet
#define MENU_ACTION_LEVEL_SELECTED              5
#define RESULT_ACTION_NEXT                      6
#define RESULT_ACTION_CANCEL                    7

const char* PATH_ROOT = LEMMINGS_DIR;

// TODO:
// make xmas91 and xmas92 distinguishable in main menu (maybe add info to top screen)

/*
	original order of actions (not taken care of yet):

	process_skill_assignment
	add_new_lemming
	update_lemmings
*/


void tile_menu_background(struct RGB_Image* im_bottom, struct MainMenuData* menu_data) {
	memset(im_bottom->data,0,sizeof(u32)*im_bottom->width*im_bottom->height);
	s16 i,j;
	for (i=0;i<im_bottom->width;i+=320) {
			for (j=0;j<im_bottom->height;j+=104) {
				draw(im_bottom,
						menu_data->palette,
						menu_data->static_pictures[0]->data,
						i,j,
						menu_data->static_pictures[0]->width,
						menu_data->static_pictures[0]->height);
			}
	}
}


// draw level selection screen to im_bottom
void select_level(u8 game, struct RGB_Image* im_bottom, struct MainMenuData* menu_data, u8 progress[], u8 cur_level, u8 top_offset, const char* level_names) {

	tile_menu_background(im_bottom, menu_data);

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
	draw_menu_text_small(im_bottom,menu_data,0,0,msg);
}


// draw main menu into im_bottom and texture_bot_screen.
int draw_main_menu(u8 game, u8 difficulty, struct MainMenuData* menu_data, struct RGB_Image* im_bottom, sf2d_texture** texture_bot_screen, int only_redraw_difficulty) {
	if (*texture_bot_screen) {
		sf2d_free_texture(*texture_bot_screen);
		*texture_bot_screen = 0;
	}
	if (!only_redraw_difficulty) {
		tile_menu_background(im_bottom, menu_data);

		draw(im_bottom,menu_data->palette,menu_data->static_pictures[2]->data,27,40,menu_data->static_pictures[2]->width,menu_data->static_pictures[2]->height);
		draw(im_bottom,menu_data->palette,menu_data->static_pictures[3]->data,174,40,menu_data->static_pictures[3]->width,menu_data->static_pictures[3]->height);
		draw(im_bottom,menu_data->palette,menu_data->static_pictures[4]->data,27,140,menu_data->static_pictures[4]->width,menu_data->static_pictures[4]->height);
		draw(im_bottom,menu_data->palette,menu_data->static_pictures[5]->data,174,140,menu_data->static_pictures[5]->width,menu_data->static_pictures[5]->height);
	}

	if (import[game].num_of_difficulty_graphics >= import[game].num_of_difficulties && import[game].num_of_difficulties > 1) {
		// draw current difficulty
		draw(im_bottom,menu_data->palette,menu_data->static_pictures[
				10+import[game].num_of_difficulty_graphics-difficulty
			]->data,174+33,140+25,menu_data->static_pictures[
				10+import[game].num_of_difficulty_graphics-difficulty
			]->width,menu_data->static_pictures[
				10+import[game].num_of_difficulty_graphics-difficulty
			]->height); // x-position: 33; y-position: 25
	}

	*texture_bot_screen = sf2d_create_texture(im_bottom->width, im_bottom->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	if (!*texture_bot_screen) {
		return 0; // error
	}

	sf2d_fill_texture_from_RGBA8(*texture_bot_screen, im_bottom->data, im_bottom->width, im_bottom->height);
	sf2d_texture_tile32(*texture_bot_screen);
	return 1;
}


// (re)initialize gamespecific data (top screen, palettes, menu screen, toolbar, ...)
int read_gamespecific_data(u8 game, struct MainMenuData* menu, struct MainInGameData* ingame, struct RGB_Image* im_top, sf2d_texture** texture_top_screen, struct RGB_Image* logo_scaled, sf2d_texture** texture_logo) {

	free_ingame_data_arrays(ingame);
	free_menu_data_arrays(menu);
	memset(ingame,0,sizeof(struct MainInGameData));
	memset(menu,0,sizeof(struct MainMenuData));
	if (*texture_logo) {
		sf2d_free_texture(*texture_logo);
		*texture_logo = 0;
	}
	if (*texture_top_screen) {
		sf2d_free_texture(*texture_top_screen);
		*texture_top_screen = 0;
	}
	*texture_top_screen = sf2d_create_texture(im_top->width, im_top->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	if (!(*texture_top_screen)) {
		return 0;
	}

	if (!read_main_menu(game, menu)) {
		memset(menu,0,sizeof(struct MainMenuData));
		return 0;
	}
	if (!read_main_ingame(game, ingame)) {
		free_menu_data_arrays(menu);
		memset(ingame,0,sizeof(struct MainInGameData));
		memset(menu,0,sizeof(struct MainMenuData));
		return 0;
	}
	tile_menu_background(im_top, menu);

	sf2d_fill_texture_from_RGBA8(*texture_top_screen, im_top->data, im_top->width, im_top->height);
	sf2d_texture_tile32(*texture_top_screen);


	struct RGB_Image* logo = (struct RGB_Image*)malloc(sizeof(struct RGB_Image)+sizeof(u32)*632*94);
	if (!logo) {
		free_ingame_data_arrays(ingame);
		free_menu_data_arrays(menu);
		memset(ingame,0,sizeof(struct MainInGameData));
		memset(menu,0,sizeof(struct MainMenuData));
		sf2d_free_texture(*texture_top_screen);
		*texture_top_screen = 0;
		return 0;
	}
	logo->width = 632;
	logo->height = 94;
	memset(logo->data,0,sizeof(u32)*logo->width*logo->height);
	draw(logo,menu->palette,menu->static_pictures[1]->data,0,0,menu->static_pictures[1]->width,menu->static_pictures[1]->height);
	scale(logo_scaled, 0.6, logo);
	free(logo);
	logo = 0;

	*texture_logo = sf2d_create_texture(logo_scaled->width, logo_scaled->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	if (!*texture_logo) {
		free_ingame_data_arrays(ingame);
		free_menu_data_arrays(menu);
		memset(ingame,0,sizeof(struct MainInGameData));
		memset(menu,0,sizeof(struct MainMenuData));
		sf2d_free_texture(*texture_top_screen);
		*texture_top_screen = 0;
		return 0;
	}

	sf2d_fill_texture_from_RGBA8(*texture_logo, logo_scaled->data, logo_scaled->width, logo_scaled->height);
	sf2d_texture_tile32(*texture_logo);
	return 1;
}


int main_menu(u8 games[], u8* game, int* lvl,
		struct MainMenuData* menu_data, struct MainInGameData* main_data,
		struct RGB_Image* im_top, struct RGB_Image* logo_scaled,
		sf2d_texture** texture_logo, sf2d_texture** texture_top_screen) {

	sf2d_texture* texture_bot_screen = 0;
	struct RGB_Image* im_bottom = (struct RGB_Image*)malloc(sizeof(struct RGB_Image)+sizeof(u32)*320*240);
	if (!im_bottom) {
		return MENU_ERROR; // error
	}
	im_bottom->width = 320;
	im_bottom->height = 240;

	if (!draw_main_menu(*game,
			*lvl/import[*game].num_of_level_per_difficulty,
			menu_data,im_bottom,&texture_bot_screen, 0)) {
		if (texture_bot_screen) {
			sf2d_free_texture(texture_bot_screen);
			texture_bot_screen = 0;
		}
		free(im_bottom);
		im_bottom = 0;
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
			if (texture_bot_screen) {
				sf2d_free_texture(texture_bot_screen);
				texture_bot_screen = 0;
			}
			free(im_bottom);
			im_bottom = 0;
			return MENU_ACTION_EXIT; // exit
		}

		if (kDown & (KEY_A | KEY_START)) {
			if (texture_bot_screen) {
				sf2d_free_texture(texture_bot_screen);
				texture_bot_screen = 0;
			}
			free(im_bottom);
			im_bottom = 0;
			return MENU_ACTION_START_SINGLE_PLAYER;
		}

		if (kDown & (KEY_B | KEY_SELECT | KEY_X)) {
			if (texture_bot_screen) {
				sf2d_free_texture(texture_bot_screen);
				texture_bot_screen = 0;
			}
			free(im_bottom);
			im_bottom = 0;
			return MENU_ACTION_SELECT_LEVEL_SINGLE_PLAYER;
		}

		if (kDown & KEY_UP) {
			if (*lvl/import[*game].num_of_level_per_difficulty < import[*game].num_of_difficulties-1) {
				*lvl = import[*game].num_of_level_per_difficulty*(*lvl/import[*game].num_of_level_per_difficulty) + import[*game].num_of_level_per_difficulty;

				if (texture_bot_screen) {
					sf2d_free_texture(texture_bot_screen);
					texture_bot_screen = 0;
				}

				if (!draw_main_menu(*game,
						*lvl/import[*game].num_of_level_per_difficulty,
						menu_data,im_bottom,&texture_bot_screen,1)) {
					if (texture_bot_screen) {
						sf2d_free_texture(texture_bot_screen);
						texture_bot_screen = 0;
					}
					free(im_bottom);
					im_bottom = 0;
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
						if (!read_gamespecific_data(*game, menu_data, main_data, im_top, texture_top_screen, logo_scaled, texture_logo)) {
							// error!
						}
						*lvl = 0;
						if (texture_bot_screen) {
							sf2d_free_texture(texture_bot_screen);
							texture_bot_screen = 0;
						}
						if (!draw_main_menu(*game,
								*lvl/import[*game].num_of_level_per_difficulty,
								menu_data,im_bottom,&texture_bot_screen,0)) {
							if (texture_bot_screen) {
								sf2d_free_texture(texture_bot_screen);
								texture_bot_screen = 0;
							}
							free(im_bottom);
							im_bottom = 0;
							return MENU_ERROR; // error
						}
						continue;
					}
				}
			}
		}
		if (kDown & KEY_DOWN) {
			if (*lvl/import[*game].num_of_level_per_difficulty > 0) {
				*lvl = import[*game].num_of_level_per_difficulty*(*lvl/import[*game].num_of_level_per_difficulty) - import[*game].num_of_level_per_difficulty;

				if (texture_bot_screen) {
					sf2d_free_texture(texture_bot_screen);
					texture_bot_screen = 0;
				}

				if (!draw_main_menu(*game,
						*lvl/import[*game].num_of_level_per_difficulty,
						menu_data,im_bottom,&texture_bot_screen,1)) {
					if (texture_bot_screen) {
						sf2d_free_texture(texture_bot_screen);
						texture_bot_screen = 0;
					}
					free(im_bottom);
					im_bottom = 0;
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
						if (!read_gamespecific_data(*game, menu_data, main_data, im_top, texture_top_screen, logo_scaled, texture_logo)) {
							if (texture_bot_screen) {
								sf2d_free_texture(texture_bot_screen);
								texture_bot_screen = 0;
							}
							free(im_bottom);
							im_bottom = 0;
							return MENU_ERROR; // error!
						}
						*lvl = import[*game].num_of_level_per_difficulty * (import[*game].num_of_difficulties-1);
						if (texture_bot_screen) {
							sf2d_free_texture(texture_bot_screen);
							texture_bot_screen = 0;
						}
						if (!draw_main_menu(*game,
								*lvl/import[*game].num_of_level_per_difficulty,
								menu_data,im_bottom,&texture_bot_screen,0)) {
							if (texture_bot_screen) {
								sf2d_free_texture(texture_bot_screen);
								texture_bot_screen = 0;
							}
							free(im_bottom);
							im_bottom = 0;
							return MENU_ERROR; // error
						}
						continue;
					}
				}

			}
		}
		sf2d_start_frame(GFX_TOP, GFX_LEFT);
		if (*texture_top_screen) {
			sf2d_draw_texture(*texture_top_screen, 0, 0);
		}
		if (*texture_logo) {
			sf2d_draw_texture(*texture_logo, 10, 20);
		}
		sf2d_end_frame();

		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		sf2d_draw_texture(texture_bot_screen, 0, 0);
		sf2d_end_frame();
		sf2d_swapbuffers();
	}
	if (texture_bot_screen) {
		sf2d_free_texture(texture_bot_screen);
		texture_bot_screen = 0;
	}
	return MENU_ERROR;
}


int level_select_menu(u8 games[], u8* game, int* lvl, u8* progress, const char* level_names,
		struct MainMenuData* menu_data, struct MainInGameData* main_data,
		struct RGB_Image* im_top, struct RGB_Image* logo_scaled,
		sf2d_texture** texture_logo, sf2d_texture** texture_top_screen) {

	sf2d_texture* texture_bot_screen = 0;
	struct RGB_Image* im_bottom = (struct RGB_Image*)malloc(sizeof(struct RGB_Image)+sizeof(u32)*320*240);
	if (!im_bottom) {
		return MENU_ERROR; // error
	}
	im_bottom->width = 320;
	im_bottom->height = 240;

	// SHOW LEVEL SELECTION SCREEN!
	int redraw_selection = 1;
	int cur_lev = *lvl;
	int top = ((*lvl%import[*game].num_of_level_per_difficulty)>20?*lvl-20:import[*game].num_of_level_per_difficulty*(*lvl/import[*game].num_of_level_per_difficulty));
	if (top%import[*game].num_of_level_per_difficulty>6) {
		top = import[*game].num_of_level_per_difficulty*(top/import[*game].num_of_level_per_difficulty)+6;
	}
	int dwn = 0;
	int up = 0;

	u8 progress_offset = 0;
	u16 level_names_offset = 0;
	u8 i;
	for (i=0;i<*game;i++) {
		progress_offset += import[i].num_of_difficulties;
		level_names_offset += import[i].num_of_difficulties * import[i].num_of_level_per_difficulty;
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
		if (redraw_selection) {
			if (texture_bot_screen) {
				sf2d_free_texture(texture_bot_screen);
				texture_bot_screen = 0;
			}

			select_level(*game,im_bottom, menu_data, progress+progress_offset, cur_lev, top, level_names+33*level_names_offset);
			texture_bot_screen = sf2d_create_texture(im_bottom->width, im_bottom->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
			if (!texture_bot_screen) {
				free(im_bottom);
				im_bottom = 0;
				return MENU_ERROR; // error
			}
			sf2d_fill_texture_from_RGBA8(texture_bot_screen, im_bottom->data, im_bottom->width, im_bottom->height);
			sf2d_texture_tile32(texture_bot_screen);
			redraw_selection = 0;
		}

		if (kDown & KEY_B) {
			if (old_game != *game) {
				*game = old_game;
				if (!read_gamespecific_data(*game, menu_data, main_data, im_top, texture_top_screen, logo_scaled, texture_logo)) {
					if (texture_bot_screen) {
						sf2d_free_texture(texture_bot_screen);
						texture_bot_screen = 0;
					}
					free(im_bottom);
					im_bottom = 0;
					return MENU_ERROR; // error
				}
			}
			// exit selection
			if (texture_bot_screen) {
				sf2d_free_texture(texture_bot_screen);
				texture_bot_screen = 0;
			}
			free(im_bottom);
			im_bottom = 0;
			return MENU_ACTION_EXIT;
		}

		if ((kDown & KEY_DOWN) || dwn >= 20) {
			if (dwn) {
				dwn = 18;
			}else{
				dwn = 1;
			}

			if (cur_lev % import[*game].num_of_level_per_difficulty < import[*game].num_of_level_per_difficulty-1) {
				cur_lev++;
				if (cur_lev % import[*game].num_of_level_per_difficulty == 0) {
					top = cur_lev;
				}
				if (cur_lev > top+20 && cur_lev%import[*game].num_of_level_per_difficulty < import[*game].num_of_level_per_difficulty-3) {
					top++;
				}
			}else if (kDown & KEY_DOWN){
				cur_lev = import[*game].num_of_level_per_difficulty*(cur_lev/import[*game].num_of_level_per_difficulty);
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
				if (cur_lev % import[*game].num_of_level_per_difficulty == import[*game].num_of_level_per_difficulty-1) {
					top = import[*game].num_of_level_per_difficulty*(cur_lev/import[*game].num_of_level_per_difficulty) + (import[*game].num_of_level_per_difficulty-24>0?import[*game].num_of_level_per_difficulty-24:0);
				}
				if (cur_lev < top+3) {
					if (top % import[*game].num_of_level_per_difficulty != 0) {
						top--;
					}
				}
			}else if (kDown & KEY_UP){
				top = cur_lev + (import[*game].num_of_level_per_difficulty-24>0?import[*game].num_of_level_per_difficulty-24:0);
				cur_lev += import[*game].num_of_level_per_difficulty-1;
			}
			redraw_selection = 1;
		}

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

		if (kDown & KEY_LEFT) {
			if (cur_lev/import[*game].num_of_level_per_difficulty > 0) {
				cur_lev = import[*game].num_of_level_per_difficulty*(cur_lev/import[*game].num_of_level_per_difficulty - 1);
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
						if (!read_gamespecific_data(*game, menu_data, main_data, im_top, texture_top_screen, logo_scaled, texture_logo)) {
							if (texture_bot_screen) {
								sf2d_free_texture(texture_bot_screen);
								texture_bot_screen = 0;
							}
							free(im_bottom);
							im_bottom = 0;
							return MENU_ERROR; // error!
						}
						cur_lev = import[*game].num_of_level_per_difficulty * (import[*game].num_of_difficulties-1);
						top = cur_lev;
						redraw_selection = 1;

						progress_offset = 0;
						level_names_offset = 0;
						u8 i;
						for (i=0;i<*game;i++) {
							progress_offset += import[i].num_of_difficulties;
							level_names_offset += import[i].num_of_difficulties * import[i].num_of_level_per_difficulty;
						}

						continue;
					}
				}

			}
		}
		if (kDown & KEY_RIGHT) {
			if (cur_lev/import[*game].num_of_level_per_difficulty < import[*game].num_of_difficulties-1) {
				cur_lev = import[*game].num_of_level_per_difficulty*(cur_lev/import[*game].num_of_level_per_difficulty + 1);
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
						if (!read_gamespecific_data(*game, menu_data, main_data, im_top, texture_top_screen, logo_scaled, texture_logo)) {
							// error!
						}
						cur_lev = 0;
						top = cur_lev;
						redraw_selection = 1;

						progress_offset = 0;
						level_names_offset = 0;
						u8 i;
						for (i=0;i<*game;i++) {
							progress_offset += import[i].num_of_difficulties;
							level_names_offset += import[i].num_of_difficulties * import[i].num_of_level_per_difficulty;
						}

						continue;
					}
				}
			}
		}
		if (kDown & (KEY_A | KEY_START)) {
			if (progress[progress_offset + cur_lev/import[*game].num_of_level_per_difficulty]>=cur_lev%import[*game].num_of_level_per_difficulty) {
				// start level!!
				*lvl = cur_lev;
				if (texture_bot_screen) {
					sf2d_free_texture(texture_bot_screen);
					texture_bot_screen = 0;
				}
				free(im_bottom);
				im_bottom = 0;
				return MENU_ACTION_LEVEL_SELECTED;
			}
		}

		sf2d_start_frame(GFX_TOP, GFX_LEFT);
		if (*texture_top_screen) {
			sf2d_draw_texture(*texture_top_screen, 0, 0);
		}
		if (*texture_logo) {
			sf2d_draw_texture(*texture_logo, 10, 20);
		}
		sf2d_end_frame();

		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		sf2d_draw_texture(texture_bot_screen, 0, 0);
		sf2d_end_frame();
		sf2d_swapbuffers();
	}
	if (texture_bot_screen) {
		sf2d_free_texture(texture_bot_screen);
		texture_bot_screen = 0;
	}
	free(im_bottom);
	im_bottom = 0;
	return MENU_ERROR;
}


// level->info.percentage_needed
int show_result(u8 game, struct LevelResult result,
		struct MainMenuData* menu_data,
		sf2d_texture** texture_logo, sf2d_texture** texture_top_screen) {

	sf2d_texture* texture_bot_screen = 0;
	struct RGB_Image* im_bottom = (struct RGB_Image*)malloc(sizeof(struct RGB_Image)+sizeof(u32)*320*240);
	if (!im_bottom) {
		return MENU_ERROR; // error
	}
	im_bottom->width = 320;
	im_bottom->height = 240;

	tile_menu_background(im_bottom, menu_data);

	int msg_id;
	if (result.percentage_rescued >= 100) {
		msg_id = 0;
	}else if (result.percentage_rescued <= 0) {
		msg_id = 8;
	}else if (result.percentage_rescued > result.percentage_needed+20) {
		msg_id = 1;
	}else if (result.percentage_rescued > result.percentage_needed) {
		msg_id = 2;
	}else if (result.percentage_rescued == result.percentage_needed) {
		msg_id = 3;
	}else if (result.percentage_rescued == result.percentage_needed-1) {
		msg_id = 4;
	}else if (result.percentage_rescued >= result.percentage_needed-5) {
		msg_id = 5;
	}else if (result.percentage_rescued >= result.percentage_needed/2) {
		msg_id = 6;
	}else{
		msg_id = 7;
	}

	char message[20*(15+1)+1];
	sprintf(message,
			"%s\n"
			"  You rescued %3u%%  \n"
			"  You needed  %3u%%  \n"
			"\n%s\n"
			"  Press A or touch  \n"
			"%s"
			"  Press B for menu  \n",
			(result.timeout?"  Your time is up!   \n":"    All lemmings    \n   accounted for.   \n"),
			result.percentage_rescued,
			result.percentage_needed<0?0:(result.percentage_needed>100?101:result.percentage_needed),
			import[game].messages[msg_id],
			(result.percentage_rescued >= result.percentage_needed?"   for next level   \n":"   to retry level   \n")
	);
	draw_menu_text(im_bottom,menu_data,0,result.timeout?16:8,message);

	texture_bot_screen = sf2d_create_texture(im_bottom->width, im_bottom->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	if (!texture_bot_screen) {
		free(im_bottom);
		im_bottom = 0;
		return MENU_ERROR;
	}

	sf2d_fill_texture_from_RGBA8(texture_bot_screen, im_bottom->data, im_bottom->width, im_bottom->height);
	sf2d_texture_tile32(texture_bot_screen);

	u32 kDown;
	// u32 kHeld;
	//touchPosition stylus;
	while (aptMainLoop()) {

		hidScanInput();
		kDown = hidKeysDown();
		// kHeld = hidKeysHeld();
		//kUp = hidKeysUp();

		if (kDown & (KEY_A | KEY_START | KEY_X | KEY_TOUCH)) {
			// load next level!
			if (texture_bot_screen) {
				sf2d_free_texture(texture_bot_screen);
				texture_bot_screen = 0;
			}
			free(im_bottom);
			im_bottom = 0;
			return RESULT_ACTION_NEXT;
		}
		if (kDown & KEY_B) {
			if (texture_bot_screen) {
				sf2d_free_texture(texture_bot_screen);
				texture_bot_screen = 0;
			}
			free(im_bottom);
			im_bottom = 0;
			return RESULT_ACTION_CANCEL;
		}


		sf2d_start_frame(GFX_TOP, GFX_LEFT);
		if (*texture_top_screen) {
			sf2d_draw_texture(*texture_top_screen, 0, 0);
		}
		if (*texture_logo) {
			sf2d_draw_texture(*texture_logo, 10, 20);
		}
		sf2d_end_frame();

		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		sf2d_draw_texture(texture_bot_screen, 0, 0);
		sf2d_end_frame();
		sf2d_swapbuffers();
	}
	if (texture_bot_screen) {
		sf2d_free_texture(texture_bot_screen);
		texture_bot_screen = 0;
	}
	free(im_bottom);
	im_bottom = 0;
	return MENU_ERROR;
}


int main() {


	// INITIALIZATION

	int i;

	gfxInitDefault();
	//sf2d_init();
	consoleInit(GFX_TOP, NULL);
	consoleClear();
	printf("\n");
	printf("        ------ 3DS Lemmings Loading ------  \n");
	printf("        ---------- Please Wait -----------  \n");
	gfxFlushBuffers();
	gfxSwapBuffers();
	gfxExit();

	u8 games[LEMMING_GAMES];
	memset(games,0,LEMMING_GAMES);
	u8 game = 0;

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
		return 1; // error
	}
	u16 offset = 0;
	for (i=0;i<LEMMING_GAMES;i++) {
		games[i] = read_level_names(i, level_names + offset);
		offset += 33*
				(u16)import[i].num_of_difficulties *
				(u16)import[i].num_of_level_per_difficulty;
	}

	// find first game the LEVEL files of which have been scanned successfully
	// and start with this game
	while (!games[game] && game < LEMMING_GAMES) {
		game++;
	}
	if (game == LEMMING_GAMES) {
		// no game data has been found
		// TODO: display error message
		return 1; // error
	}

	// TODO: read configuration file
	// LEFTHANDED.DAT hack; TODO: remove once configuration file is implemented
	char lh_fn[64];
	sprintf(lh_fn,"%s/LEFTHANDED.DAT", PATH_ROOT);
	FILE* lefthand = fopen(lh_fn,"r");
	if (lefthand) {
		controls[0].forbidden_keys = KEY_L;
		controls[10].held_keys = KEY_R;
		controls[11].held_keys = KEY_L;
		controls[12].held_keys = KEY_CPAD_DOWN | KEY_L;
		controls[13].held_keys = KEY_CPAD_UP | KEY_L;
		fclose(lefthand);
	}

	// read save file
	u8* progress = (u8*)malloc(overall_num_of_difficulties);
	if (!progress) {
		return 1; // error
	}
	read_savegame(progress);


	// initialize variables

	struct MainMenuData* menu_data = (struct MainMenuData*)malloc(sizeof(struct MainMenuData));
	if (!menu_data) {
		return 1; // error
	}
	memset(menu_data,0,sizeof(struct MainMenuData));

	struct RGB_Image* logo_scaled = (struct RGB_Image*)malloc(sizeof(struct RGB_Image)+sizeof(u32)*380*57);
	if (!logo_scaled) {
		return 1; // error
	}
	logo_scaled->width = 380;
	logo_scaled->height = 57;

	struct RGB_Image* im_top = (struct RGB_Image*)malloc(sizeof(struct RGB_Image)+sizeof(u32)*400*320);
	if (!im_top) {
		return 1; // error
	}
	im_top->width = 400;
	im_top->height = 320;

	struct MainInGameData* main_data = (struct MainInGameData*)malloc(sizeof(struct MainInGameData));
	if (!main_data) {
		return 1; // error
	}
	memset(main_data,0,sizeof(struct MainInGameData));

	// initialize sf2d, create textures
	sf2d_init();
	sf2d_texture* texture_logo = 0;
	sf2d_texture* texture_top_screen = 0;

	if (!read_gamespecific_data(game, menu_data, main_data, im_top, &texture_top_screen, logo_scaled, &texture_logo)) {
		return 1; // error
	}

	// begin with first level
	int lvl = 0;


	// GAME LOOP

	while(1) {
		int menu_selection = main_menu(games, &game, &lvl, menu_data, main_data, im_top, logo_scaled, &texture_logo, &texture_top_screen);


		if (menu_selection == MENU_ACTION_SELECT_LEVEL_SINGLE_PLAYER) {
			int level_selection = level_select_menu(games, &game, &lvl,
					progress, level_names,
					menu_data, main_data,
					im_top, logo_scaled,
					&texture_logo, &texture_top_screen);
			switch (level_selection) {
				case MENU_ACTION_EXIT:
					break;
				case MENU_ACTION_LEVEL_SELECTED:
					menu_selection = MENU_ACTION_START_SINGLE_PLAYER;
					break;
				case MENU_ERROR:
				default:
					return 1; // error
			}
		}

		if (menu_selection == MENU_ACTION_EXIT) {
			break;
		}

		if (menu_selection == MENU_ERROR) {
			return 1; // error
		}



		if (menu_selection == MENU_ACTION_START_SINGLE_PLAYER) {
			while(1) {
				struct LevelResult lev_result = run_level(game, lvl, main_data,
						texture_top_screen, texture_logo);

				// process result
				// find out whether level has been solved successful
				if (lev_result.percentage_rescued >= lev_result.percentage_needed) {
					// increment lvl counter to read next level
					u8 progress_offset = 0;
					u8 i;
					for (i=0;i<game;i++) {
						progress_offset += import[i].num_of_difficulties;
					}
					progress_offset += lvl/import[game].num_of_level_per_difficulty;

					if (progress[progress_offset] < lvl%import[game].num_of_level_per_difficulty+1) {
						progress[progress_offset] = lvl%import[game].num_of_level_per_difficulty+1;
						write_savegame(progress);
					}
					lvl = (lvl+1)%(import[game].num_of_difficulties*import[game].num_of_level_per_difficulty);
					if (lvl == 0) {
						// TODO: show congratulation message
						// TODO: increase game variable?
					}
				}

				int result_screen = show_result(game,
						lev_result,
						menu_data, &texture_logo, &texture_top_screen);
				if (result_screen == RESULT_ACTION_NEXT) {
					continue;
				}
				if (result_screen == RESULT_ACTION_CANCEL) {
					break;
				}
				return 1; // error
			}
		}
	}

	sf2d_fini();
	return 0;
}

