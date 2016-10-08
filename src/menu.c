#include <malloc.h>
#include <stdio.h>
#include <3ds.h>
#include <sf2d.h>
#include "menu.h"
#include "draw.h"
#include "audio.h"
#include "gamespecific.h"
#include "import_main.h"

void clean_menu_bottom_graphics(sf2d_texture** texture_bot_screen, struct RGB_Image** im_bottom) {
	if (texture_bot_screen) {
		if (*texture_bot_screen) {
			sf2d_free_texture(*texture_bot_screen);
			*texture_bot_screen = 0;
		}
	}
	if (im_bottom) {
		if (*im_bottom) {
			free(*im_bottom);
			*im_bottom = 0;
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
				clean_menu_bottom_graphics(0, &im_bottom);
				return MENU_ERROR; // error
			}
			sf2d_fill_texture_from_RGBA8(texture_bot_screen, im_bottom->data, im_bottom->width, im_bottom->height);
			sf2d_texture_tile32(texture_bot_screen);
			redraw_selection = 0;
		}

		if (kDown & KEY_B) {
			if (old_game != *game) {
				*game = old_game;
				if (!read_gamespecific_data(*game, menu_data, main_data)) {
					// error!
					clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
					return MENU_ERROR; // error
				}
				if (!draw_topscreen(menu_data, im_top, texture_top_screen, logo_scaled, texture_logo)) {
					// error!
					clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
					clean_gamedata(menu_data, main_data);
					return MENU_ERROR; // error
				}
			}
			// exit selection
			clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
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
						if (!read_gamespecific_data(*game, menu_data, main_data)) {
							// error!
							clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
							return MENU_ERROR; // error
						}
						if (!draw_topscreen(menu_data, im_top, texture_top_screen, logo_scaled, texture_logo)) {
							// error!
							clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
							clean_gamedata(menu_data, main_data);
							return MENU_ERROR; // error
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
						if (!read_gamespecific_data(*game, menu_data, main_data)) {
							// error!
							clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
							return MENU_ERROR; // error
						}
						if (!draw_topscreen(menu_data, im_top, texture_top_screen, logo_scaled, texture_logo)) {
							// error!
							clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
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
				clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
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
	clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
	return MENU_ERROR;
}


// draw main menu into im_bottom and texture_bot_screen.
int draw_main_menu(u8 game, u8 difficulty, struct MainMenuData* menu_data, struct RGB_Image* im_bottom, sf2d_texture** texture_bot_screen, int only_redraw_difficulty, int only_redraw_music) {
	if (*texture_bot_screen) {
		sf2d_free_texture(*texture_bot_screen);
		*texture_bot_screen = 0;
	}
	if (!only_redraw_difficulty && !only_redraw_music) {
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

	if (!only_redraw_difficulty) {
		draw(im_bottom,menu_data->palette,menu_data->static_pictures[4]->data,27,140,menu_data->static_pictures[4]->width,menu_data->static_pictures[4]->height);
		if (is_audio_enabled()) {
			draw(im_bottom,menu_data->palette,menu_data->static_pictures[8]->data,27+27,140+26,menu_data->static_pictures[8]->width,menu_data->static_pictures[8]->height);
		}else if (is_audio_only_fx()) {
			draw(im_bottom,menu_data->palette,menu_data->static_pictures[9]->data,27+27,140+26,menu_data->static_pictures[9]->width,menu_data->static_pictures[9]->height);
		}
	}

	*texture_bot_screen = sf2d_create_texture(im_bottom->width, im_bottom->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	if (!*texture_bot_screen) {
		return 0; // error
	}

	sf2d_fill_texture_from_RGBA8(*texture_bot_screen, im_bottom->data, im_bottom->width, im_bottom->height);
	sf2d_texture_tile32(*texture_bot_screen);
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
			menu_data,im_bottom,&texture_bot_screen, 0,0)) {
		clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
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
			clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
			return MENU_ACTION_EXIT; // exit
		}

		if (kDown & (KEY_A | KEY_START)) {
			clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
			return MENU_ACTION_START_SINGLE_PLAYER;
		}

		if (kDown & (KEY_B | KEY_SELECT | KEY_X)) {
			clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
			return MENU_ACTION_SELECT_LEVEL_SINGLE_PLAYER;
		}

		if (kDown & KEY_Y) {
			int update = toggle_audio();
			if (update) {
				if (!draw_main_menu(*game,
						*lvl/import[*game].num_of_level_per_difficulty,
						menu_data,im_bottom,&texture_bot_screen,0,1)) {
					clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
					return MENU_ERROR; // error
				}
			}
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
						menu_data,im_bottom,&texture_bot_screen,1,0)) {
					clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
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
							clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
							return MENU_ERROR; // error
						}
						if (!draw_topscreen(menu_data, im_top, texture_top_screen, logo_scaled, texture_logo)) {
							// error!
							clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
							clean_gamedata(menu_data, main_data);
							return MENU_ERROR; // error
						}
						*lvl = 0;
						if (texture_bot_screen) {
							sf2d_free_texture(texture_bot_screen);
							texture_bot_screen = 0;
						}
						if (!draw_main_menu(*game,
								*lvl/import[*game].num_of_level_per_difficulty,
								menu_data,im_bottom,&texture_bot_screen,0,0)) {
							clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
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

				if (!draw_main_menu(*game,
						*lvl/import[*game].num_of_level_per_difficulty,
						menu_data,im_bottom,&texture_bot_screen,1,0)) {
					clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
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
							clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
							return MENU_ERROR; // error
						}
						if (!draw_topscreen(menu_data, im_top, texture_top_screen, logo_scaled, texture_logo)) {
							// error!
							clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
							clean_gamedata(menu_data, main_data);
							return MENU_ERROR; // error
						}
						*lvl = import[*game].num_of_level_per_difficulty * (import[*game].num_of_difficulties-1);
						if (texture_bot_screen) {
							sf2d_free_texture(texture_bot_screen);
							texture_bot_screen = 0;
						}
						if (!draw_main_menu(*game,
								*lvl/import[*game].num_of_level_per_difficulty,
								menu_data,im_bottom,&texture_bot_screen,0,0)) {
							clean_menu_bottom_graphics(&texture_bot_screen, &im_bottom);
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
