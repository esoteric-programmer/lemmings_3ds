#include <stdio.h>
#include <stdlib.h> // rand()
#include <string.h>

#include "ingame.h"
#include "control.h"
#include "lemming.h"
#include "settings.h"
#include "gamespecific.h"
#include "import_level.h"
#include "cursor.h"
#include "audio.h"
#include "menu.h"

#define BOTTOM_SCREEN_Y_OFFSET 32

// returns step width on rate changing
// step_width_start(): single/first click
// step_width_running(): mouse button held down
// the implementation below simulates the behaviour of the DOS game
// in my DOSBox environment. only from observation, not from disassembly.
u8 step_width_start() {
	if (rand()%2 == 0) {
		if (rand()%2==0) {
			return 1;
		}else{
			return 4;
		}
	}else{
		if (rand()%3==0) {
			if (rand()%7==0) {
				return 0;
			}else{
				return 2;
			}
		}else{
			return 3;
		}
	}
}
u8 step_width_running() {
	return rand()%3?4:3;
}

struct LevelResult run_level(u8 game, u8 lvl,
		struct MainInGameData* main_data,
		sf2d_texture* texture_top_screen, sf2d_texture* texture_logo) {

	struct LevelResult result = { 0xFF, 0, 0xFF, LEVEL_ERROR };

	struct Level* level = (struct Level*)malloc(sizeof(struct Level));
	if (!level){
		return result; // error
	}
	memset(level,0,sizeof(struct Level));

	int frame = 0;
	int opening_counter = 0;
	int wait_rate = 0;
	u8 nuke_key_dbl = 0;
	struct LevelState state;
	memset(&state,0,sizeof(struct LevelState));
	struct Lemming lemmings[MAX_NUM_OF_LEMMINGS];
	sf2d_texture* texture_above = 0;

	struct RGB_Image* im = (struct RGB_Image*)malloc(sizeof(struct RGB_Image)+sizeof(u32)*200*SCREEN_WIDTH);
	if (!im) {
		return result; // error
	}
	im->width = SCREEN_WIDTH;
	im->height = 200;
	struct RGB_Image* im_above_level = (struct RGB_Image*)malloc(
			sizeof(struct RGB_Image)+
			sizeof(u32)*BOTTOM_SCREEN_Y_OFFSET*SCREEN_WIDTH);
	if (!im_above_level) {
		free(im);
		return result; // error
	}
	im_above_level->width = SCREEN_WIDTH;
	im_above_level->height = BOTTOM_SCREEN_Y_OFFSET;

	if (!read_level(game, lvl, level)) {
		free(im);
		free(im_above_level);
		free_objects(level->o);
		return result; // error
	}
	init_level_state(&state,level);
	init_lemmings(lemmings);

	struct RGB_Image* cursor_palette = (struct RGB_Image*)malloc(sizeof(struct RGB_Image)+sizeof(u32)*14*14);
	if (!cursor_palette) {
		free(im);
		free(im_above_level);
		free_objects(level->o);
		return result; // error
	}
	cursor_palette->width = 14;
	cursor_palette->height = 14;
	memset(cursor_palette->data,0,sizeof(u32)*14*14);
	draw(cursor_palette, level->palette.vga, cursor_data, 0, 0, 14, 14);

	// load cursor (with palette colors)
	// minimum size is 64x64; however, sf2d has imolemented minimum size of 8x8.
	// this is an ugly workaround: initiale as 64x64, change size to 14x14 manually.
	sf2d_texture* cursor = sf2d_create_texture(64, 64, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	if (!cursor) {
		free(im);
		free(im_above_level);
		free(cursor_palette);
		free_objects(level->o);
		return result; // error
	}
	cursor->width = 14;
	cursor->height = 14;
	sf2d_fill_texture_from_RGBA8(cursor, cursor_palette->data, 14, 14);
	sf2d_texture_tile32(cursor);



	struct RGB_Image* cursor_active_palette = (struct RGB_Image*)malloc(sizeof(struct RGB_Image)+sizeof(u32)*14*14);
	if (!cursor_active_palette) {
		free(im);
		free(im_above_level);
		free(cursor_palette);
		sf2d_free_texture(cursor);
		free_objects(level->o);
		return result; // error
	}
	cursor_active_palette->width = 14;
	cursor_active_palette->height = 14;
	memset(cursor_active_palette->data,0,sizeof(u32)*14*14);
	draw(cursor_active_palette, level->palette.vga, cursor_active_data, 0, 0, 14, 14);


	sf2d_texture* cursor_active = sf2d_create_texture(64, 64, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	if (!cursor_active) {
		free(im);
		free(im_above_level);
		free(cursor_palette);
		sf2d_free_texture(cursor);
		free(cursor_active_palette);
		free_objects(level->o);
		return result; // error
	}
	cursor_active->width = 14;
	cursor_active->height = 14;
	sf2d_fill_texture_from_RGBA8(cursor_active, cursor_active_palette->data, 14, 14);
	sf2d_texture_tile32(cursor_active);


	if (level->info.x_pos >= (SCREEN_WIDTH-320)/2) {
		level->info.x_pos -= (SCREEN_WIDTH-320)/2;
	}else{
		level->info.x_pos = 0;
	}
	if (SCREEN_WIDTH < 1584 && level->info.x_pos + SCREEN_WIDTH >= 1584) {
		level->info.x_pos = 1584 - SCREEN_WIDTH;
	}
	if (SCREEN_WIDTH >= 1584) {
		level->info.x_pos = 0;
	}
	int update_above = 1;
	u16 fade_in = FADE_IN_FRAMES;
	u16 fade_out = 0;

	u32 kDown;
	u32 kHeld;
	touchPosition stylus;
	circlePosition circle_pos;

	int apt_result;
	while ((apt_result = aptMainLoop())) {
		update_audio();
		hidScanInput();
		kDown = hidKeysDown();
		kHeld = hidKeysHeld();
		//kUp = hidKeysUp();
		hidTouchRead(&stylus);
		hidCircleRead(&circle_pos);
		circle_pos = cpad_to_movement(circle_pos);

		u32 action = get_action(kDown, kHeld);
		if (action & ACTION_QUIT_GAME) {
			result.exit_reason = LEVEL_EXIT_GAME;
			return result;
		}
		if (action & ACTION_MOVE_CURSOR_RIGHT) {
			state.cursor.x += 2;
		}
		if (action & ACTION_MOVE_CURSOR_LEFT) {
			state.cursor.x -= 2;
		}
		if (action & ACTION_MOVE_CURSOR_UP) {
			state.cursor.y -= 2;
		}
		if (action & ACTION_MOVE_CURSOR_DOWN) {
			state.cursor.y += 2;
		}
		if (action & ACTION_MOVE_CURSOR_CPAD) {
			s16 x = circle_pos.dx;
			s16 y = circle_pos.dy;
			if (action & ACTION_MOVE_CURSOR_CPAD_LR) {
				x = -x;
			}
			if (action & ACTION_MOVE_CURSOR_CPAD_UD) {
				y = -y;
			}
			state.cursor.x += x;
			state.cursor.y += y;
		}
		if (state.cursor.y < 0) {
			state.cursor.y = 0;
		}
		if (state.cursor.y >= 200) {
			state.cursor.y = 200;
		}
		if (state.cursor.x < 0) {
			state.cursor.x = 0;
		}
		if (state.cursor.x >= SCREEN_WIDTH) {
			state.cursor.x = SCREEN_WIDTH-1;
		}
		if (state.cursor.x == 0 && state.cursor.y < 160) {
			action |= ACTION_SCROLL_LEFT;
		}
		if (state.cursor.x == SCREEN_WIDTH-1 && state.cursor.y < 160) {
			action |= ACTION_SCROLL_RIGHT;
		}
		if (((kDown | kHeld) & KEY_TOUCH) &&
				stylus.py >= BOTTOM_SCREEN_Y_OFFSET &&
				stylus.py < 200 + BOTTOM_SCREEN_Y_OFFSET &&
				stylus.px < SCREEN_WIDTH) {
			state.cursor.x = stylus.px;
			state.cursor.y = (s16)stylus.py - BOTTOM_SCREEN_Y_OFFSET;
			if (kDown & KEY_TOUCH) {
				action |= ACTION_CURSOR_CLICK;
				action |= ACTION_CURSOR_HOLD;
			}else{
				action |= ACTION_CURSOR_HOLD;
			}
		}
		if (state.cursor.x >= SCREEN_WIDTH) {
			state.cursor.x = SCREEN_WIDTH-1;
		}
		if (state.cursor.y >= 200-6) {
			state.cursor.y = 200-7;
		}
		if (action & (ACTION_CURSOR_CLICK | ACTION_CURSOR_HOLD)) {
			if (state.cursor.y >= 176 && state.cursor.y < 200) {
				// clicked at panel
				switch (state.cursor.x / 16) {
					case  0:
						// decrement rate
						action |= ACTION_DEC_RATE;
						break;
					case 1:
						// increment rate
						action |= ACTION_INC_RATE;
						break;
					case 2:
						if (action & ACTION_CURSOR_CLICK) {
							action |=  ACTION_SELECT_SKILL_CIMBER;
						}
						break;
					case 3:
						if (action & ACTION_CURSOR_CLICK) {
							action |=  ACTION_SELECT_SKILL_FLOATER;
						}
						break;
					case 4:
						if (action & ACTION_CURSOR_CLICK) {
							action |=  ACTION_SELECT_SKILL_BOMBER;
						}
						break;
					case 5:
						if (action & ACTION_CURSOR_CLICK) {
							action |=  ACTION_SELECT_SKILL_BLOCKER;
						}
						break;
					case 6:
						if (action & ACTION_CURSOR_CLICK) {
							action |=  ACTION_SELECT_SKILL_BUILDER;
						}
						break;
					case 7:
						if (action & ACTION_CURSOR_CLICK) {
							action |=  ACTION_SELECT_SKILL_BASHER;
						}
						break;
					case 8:
						if (action & ACTION_CURSOR_CLICK) {
							action |=  ACTION_SELECT_SKILL_MINER;
						}
						break;
					case 9:
						if (action & ACTION_CURSOR_CLICK) {
							action |=  ACTION_SELECT_SKILL_DIGGER;
						}
						break;
					case 10:
						if (action & ACTION_CURSOR_CLICK) {
							action |=  ACTION_PAUSE;
						}
						break;
					case 11:
						if (action & ACTION_CURSOR_CLICK) {
							action |=  ACTION_NUKE;
						}
						break;
					case 12:
						// nothing
						break;
					default:
						{
							// touched at minimap
							s16 new_x_pos = (state.cursor.x - 13*16) * 16;
							new_x_pos -= SCREEN_WIDTH / 2;
							if (new_x_pos < 0) {
								new_x_pos = 0;
							}else if (new_x_pos >= 1584 - SCREEN_WIDTH) {
								new_x_pos = (SCREEN_WIDTH <= 1584?1584 - SCREEN_WIDTH:0);
							}
							level->info.x_pos = new_x_pos;
						}
				}
			}
		}
		if (action & ACTION_PAUSE) {
			state.paused = !state.paused;
			nuke_key_dbl = 0;
		}
		if (action & ACTION_NEXT_SKILL) {
			state.selected_skill = (state.selected_skill+1)%8;
			play_sound(0x01);
		}
		if (action & ACTION_PREV_SKILL) {
			if (state.selected_skill > 0) {
				state.selected_skill--;
			}else{
				state.selected_skill = 7;
			}
			play_sound(0x01);
		}
		if (action & ACTION_SELECT_SKILL_CIMBER) {
			state.selected_skill = 0;
			play_sound(0x01);
		}
		if (action & ACTION_SELECT_SKILL_FLOATER) {
			state.selected_skill = 1;
			play_sound(0x01);
		}
		if (action & ACTION_SELECT_SKILL_BOMBER) {
			state.selected_skill = 2;
			play_sound(0x01);
		}
		if (action & ACTION_SELECT_SKILL_BLOCKER) {
			state.selected_skill = 3;
			play_sound(0x01);
		}
		if (action & ACTION_SELECT_SKILL_BUILDER) {
			state.selected_skill = 4;
			play_sound(0x01);
		}
		if (action & ACTION_SELECT_SKILL_BASHER) {
			state.selected_skill = 5;
			play_sound(0x01);
		}
		if (action & ACTION_SELECT_SKILL_MINER) {
			state.selected_skill = 6;
			play_sound(0x01);
		}
		if (action & ACTION_SELECT_SKILL_DIGGER) {
			state.selected_skill = 7;
			play_sound(0x01);
		}
		if (action & ACTION_INC_RATE) {
			s8 inc = 0;
			if (!wait_rate) {
				inc = step_width_start();
				wait_rate++;
			}else{
				wait_rate++;
				if (wait_rate % 12 == 0) {
					wait_rate = 6;
					inc = step_width_running();
				}
			}
			if (state.cur_rate+inc < 100) {
				state.cur_rate += inc;
			} else {
				state.cur_rate = 99;
			}
		}
		if (action & ACTION_DEC_RATE) {
			s8 dec = 0;
			if (!wait_rate) {
				dec = step_width_start();
				wait_rate++;
			}else{
				wait_rate++;
				if (wait_rate % 12 == 0) {
					wait_rate = 6;
					dec = step_width_running();
				}
			}
			if (state.cur_rate > dec+1) {
				state.cur_rate -= dec;
			} else {
				state.cur_rate = 1;
			}
			if (state.cur_rate < level->info.rate) {
				state.cur_rate = level->info.rate;
			}
		}
		if (!(action & (ACTION_INC_RATE | ACTION_DEC_RATE))) {
			wait_rate = 0;
		}
		if (!state.paused && (action & (ACTION_NUKE | ACTION_NUKE_IMMEDIATELY))) {
			if ((action & ACTION_NUKE_IMMEDIATELY) || (nuke_key_dbl && nuke_key_dbl < 45)) {
				nuke(level);
				update_above = 1;
			}else{
				nuke_key_dbl = 1;
			}
		}
		if (action & ACTION_SCROLL_RIGHT) {
			level->info.x_pos+=3;
			if (level->info.x_pos > 1584-SCREEN_WIDTH) {
				if (1584>SCREEN_WIDTH) {
					level->info.x_pos = 1584-SCREEN_WIDTH;
				} else {
					level->info.x_pos = 0;
				}
			}
		}
		if (action & ACTION_SCROLL_LEFT) {
			if ((s16)level->info.x_pos >= 3) {
				level->info.x_pos-=3;
			}else{
				level->info.x_pos = 0;
			}
		}
		if (action & ACTION_SCROLL_CPAD) {
			s16 x = circle_pos.dx;
			if (action & ACTION_SCROLL_CPAD_LR) {
				x = -x;
			}
			if (x < 0) {
				if ((s16)level->info.x_pos >= -x) {
					level->info.x_pos+=x;
				}else{
					level->info.x_pos = 0;
				}
			}else{
				level->info.x_pos+=x;
				if (level->info.x_pos > 1584-SCREEN_WIDTH) {
					if (1584>SCREEN_WIDTH) {
						level->info.x_pos = 1584-SCREEN_WIDTH;
					} else {
						level->info.x_pos = 0;
					}
				}
			}
		}
		if (level->info.speed_up) {
			action |= ACTION_SPEED_UP;
		}

		struct Lemming* lem1 = 0;
		struct Lemming* lem2 = 0;
		u8 n_lem = 0;
		if (state.cursor.y < 160) {
			s16 l1idx = -1;
			s16 l2idx = -1;
			n_lem = select_lemming(lemmings, (s16)level->info.x_pos + state.cursor.x, state.cursor.y, (action & ACTION_NONPRIORIZED_LEMMING)?1:0, &l1idx, &l2idx);
			if (l1idx >= 0 && l1idx < MAX_NUM_OF_LEMMINGS) {
				lem1 = lemmings+l1idx;
				if (l2idx >= 0 && l2idx < MAX_NUM_OF_LEMMINGS) {
					lem2 = lemmings+l2idx;
				}
			}
		}

		if (action & ACTION_CURSOR_CLICK) {
			if (lem1) {
				if (level->info.skills[state.selected_skill]) {
					if (assign_skill(state.selected_skill, lem1, lem2, level)) {
						level->info.skills[state.selected_skill]--;
					}
				}
			}
		}

		draw_level(im,level);
		u8 num_lems = draw_lemmings(lemmings, main_data->lemmings_anim, main_data->masks, level->palette.vga, im, level->info.x_pos);
		char text[41+10];
		// example: "FALLER  1     OUT 3    IN  0%  TIME 4-54"
		char tmp_text[10];
		memset(tmp_text,' ',9);
		tmp_text[9] = 0;
		const char* lem_desc = get_lemming_description(lem1);
		if (lem_desc) {
			int i=0;
			while (*lem_desc && i<7) {
				tmp_text[i] = *lem_desc;
				lem_desc++;
				i++;
			}
			sprintf(tmp_text+7,"%2u",n_lem<99?n_lem:99);
		}
		int min = (state.frames_left+FPS-1)/FPS/60;
		int sec = (state.frames_left+FPS-1)/FPS%60;
		u16 percentage = 0;
		if (level->info.lemmings != 0) {
			percentage = (100*(u16)level->rescued) / ((u16)level->info.lemmings);
		}
		sprintf(text,"%s     OUT%2u    IN%3u%%  TIME %d-%02d",
				tmp_text,num_lems<99?num_lems:99,
				percentage<100?percentage:100,
				(min<10)?min:9, (min<10)?sec:59);
		draw_toolbar(im, main_data, level, &state, lemmings, text);

		if (num_lems + lemmings_left(level->info.lemmings) == 0 ||
				state.frames_left == 0 ||
				(action & ACTION_QUIT)) {
			if (!fade_out) {
				fade_out = 1;
			}
		}

		if (update_above || !texture_above) {
			char above_text[100];
			const char* lvl_name_stripped = level->info.name;
			while (*lvl_name_stripped == ' ') {
				lvl_name_stripped++;
			}
			memset(im_above_level->data,0,im_above_level->width*im_above_level->height*4);
			u8 level_no;
			if (import[game].num_of_level_per_difficulty > 1) {
				level_no = (lvl%import[game].num_of_level_per_difficulty)+1;
			}else{
				level_no = lvl+1;
			}
			sprintf(above_text,
					"%s%2u %s\n                     You need %3u%% of %2u",
					import[game].difficulties[lvl/import[game].num_of_level_per_difficulty],
					level_no,
					lvl_name_stripped,level->info.percentage_needed,
					level->info.lemmings);
			draw_highperf_text(im_above_level, main_data, 0, above_text);
			if (texture_above) {
				sf2d_free_texture(texture_above);
			}
			texture_above = sf2d_create_texture(
					im_above_level->width,
					im_above_level->height,
					TEXFMT_RGBA8, SF2D_PLACE_RAM);
			if (texture_above) {
				update_above = 0;
				sf2d_fill_texture_from_RGBA8(texture_above,
						im_above_level->data,
						im_above_level->width, im_above_level->height);
				sf2d_texture_tile32(texture_above);
			}
		}


		sf2d_texture* texture = sf2d_create_texture(
				im->width, im->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
		if (!texture) {
			free(im);
			free(im_above_level);
			free(cursor_palette);
			sf2d_free_texture(cursor);
			free(cursor_active_palette);
			sf2d_free_texture(cursor_active);
			free_objects(level->o);
			return result; // error
		}
		sf2d_fill_texture_from_RGBA8(texture, im->data, im->width, im->height);
		sf2d_texture_tile32(texture);
		sf2d_start_frame(GFX_TOP, GFX_LEFT);
		if (texture_top_screen) {
			sf2d_draw_texture(texture_top_screen, 0, 0);
		}
		if (texture_logo) {
			sf2d_draw_texture(texture_logo, 10, 20);
		}
		sf2d_end_frame();

		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		u32 fade = 0xFF;
		if (fade_in) {
			fade = ((FADE_IN_FRAMES-fade_in)*0xFF) / FADE_IN_FRAMES;
		}else if (fade_out) {
			fade = ((FADE_OUT_FRAMES-fade_out)*0xFF) / FADE_OUT_FRAMES;
		}
		fade = (fade>0xFF?0xFF:fade);
		fade = fade | (fade<<8) | (fade<<16) | 0xFF000000;
		sf2d_draw_texture_blend(texture, 0, BOTTOM_SCREEN_Y_OFFSET,fade);
		sf2d_draw_texture_blend(lem1?cursor_active:cursor,
				state.cursor.x-7,
				state.cursor.y-7+BOTTOM_SCREEN_Y_OFFSET,
				fade);
		if (texture_above) {
			sf2d_draw_texture_blend(texture_above, 0, 0, fade);
		}
		sf2d_end_frame();
		sf2d_swapbuffers();
		sf2d_free_texture(texture);

		if (ENABLE_ENTRANCE_PAUSING_GLITCH || !state.paused) {
			if (opening_counter / FRAMES_PER_DOS_FRAME <= 55) {
				int _old = opening_counter / FRAMES_PER_DOS_FRAME;
				if (action & ACTION_SPEED_UP) {
					opening_counter += FRAMES_PER_DOS_FRAME;
				}else{
					opening_counter++;
				}
				int _new = opening_counter / FRAMES_PER_DOS_FRAME;
				if (_new != _old) {
					// do action depending on _new:
					if (_new == 15) {
						// play sound: lets go
						play_sound(0x03);
					}else if (_new == 35) {
						// play sound: opening entrance
						play_sound(0x02);
						// start opening of entrances;
						state.entrances_open = 1;
					}else if (_new == 55) {
						// start background music
						play_music(game, lvl);
					}
				}
			}
		}

		if (fade_in) {
			if (fade_in >= ((action & ACTION_SPEED_UP)?3:1)) {
				fade_in -= ((action & ACTION_SPEED_UP)?3:1);
			}else{
				fade_in = 0;
			}
		}else if (fade_out) {
			fade_out += ((action & ACTION_SPEED_UP)?3:1);
			if (fade_out > FADE_OUT_FRAMES) {
				break;
			}
		}

		if (!state.paused) {
			if (nuke_key_dbl) {
				nuke_key_dbl+=(action & ACTION_SPEED_UP)?FRAMES_PER_DOS_FRAME:1;
			}
			int i;
			if (!(action & ACTION_SPEED_UP)) {
				frame++; // don't animate in each frame!
			}
			if (frame % FRAMES_PER_DOS_FRAME == 0 || (action & ACTION_SPEED_UP)) {
				if (frame % FRAMES_PER_DOS_FRAME == 0) {
					frame = 0;
				}
				if (state.entrances_open) {
					add_lemming(lemmings,
							&(level->entrances),
							state.cur_rate,
							level->info.lemmings);
				}
				if (state.frames_left > 0 && !fade_out) {
					state.frames_left--;
				}
				update_lemmings(lemmings,level,main_data->masks);
				for (i=0;i<32;i++) {
					// process object!
					if (!(level->obj[i].modifier & OBJECT_USED)) {
						continue;
					}
					if (level->obj[i].type == 1) {
						// start
						if (state.entrances_open) {
							struct Object* o = level->o[level->obj[i].type];
							if (o && level->obj[i].current_frame) {
								level->obj[i].current_frame++;
								if (level->obj[i].current_frame >= o->end_frame) {
									level->obj[i].current_frame = 0;
								}
							}
						}
						continue;
					}
					struct Object* o = level->o[level->obj[i].type];
					if (!o) {
						continue;
					}
					if (o->trigger == 4 && !level->obj[i].current_frame) {
						// trap...
						continue;
					}
					level->obj[i].current_frame++;
					if (level->obj[i].current_frame >= o->end_frame) {
						level->obj[i].current_frame = o->start_frame; // TODO: set to 0 instead?
					}
				}
			}
		}
	}
	stop_audio();
	if (!apt_result) {
		result.exit_reason = LEVEL_EXIT_GAME;
		return result;
	}

	result.lvl = lvl;
	if ((u16)level->info.lemmings > 0) {
		result.percentage_rescued = ((u16)level->rescued)*100 / (u16)level->info.lemmings;
	}else{
		result.percentage_rescued = 0;
	}
	result.percentage_needed = level->info.percentage_needed;

	if (result.percentage_rescued >= result.percentage_needed) {
		next_music(game);
	}

	if (!state.frames_left) {
		result.exit_reason = LEVEL_TIMEOUT;
	}else{
		result.exit_reason = LEVEL_NO_LEMMINGS_LEFT;
	}
	if (texture_above) {
		sf2d_free_texture(texture_above);
	}
	free(im);
	free(im_above_level);
	free(cursor_palette);
	sf2d_free_texture(cursor);
	free(cursor_active_palette);
	sf2d_free_texture(cursor_active);
	free_objects(level->o);
	return result;
}

int show_result(u8 game, struct LevelResult result,
		struct MainMenuData* menu_data,
		sf2d_texture** texture_logo, sf2d_texture** texture_top_screen) {

	if (result.exit_reason == LEVEL_ERROR) {
		return MENU_ERROR;
	}
	if (result.exit_reason == LEVEL_EXIT_GAME) {
		return MENU_EXIT_GAME;
	}
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
	// result message (like: "oh dear, not even one poor lemming")
	const char* msg = import[game].messages[msg_id];
	int msg_lines = 4;

	// whole screen text (like "all lemmings accounted for. you rescued 0% oh dear...")
	char message[20*(15+1)+1];

	// find out whether a special message has to be shown
	int i;
	for (i=0;i<import[game].num_of_special_messages;i++) {
		if (import[game].special_messages[i].level == result.lvl) {
			// we need to show a special message
			msg = import[game].special_messages[i].message;
			msg_lines = import[game].special_messages[i].lines_of_message;
			break;
		}
	}

	sprintf(message,
			"%s%s"
			"  You rescued %3u%%  \n"
			"  You needed  %3u%%  \n"
			"%s%s%s"
			"  Press A or touch  \n"
			"%s"
			"  Press B for menu  \n",
			(result.exit_reason == LEVEL_TIMEOUT?
					"  Your time is up!   \n":
					"    All lemmings    \n   accounted for.   \n"),
			msg_lines<=4?"\n":"",
			result.percentage_rescued,
			result.percentage_needed<0?
					0:
					(result.percentage_needed>100?
							101:
							result.percentage_needed),
			msg_lines<=7?"\n":"",
			msg,
			msg_lines<=6?"\n":"",
			(result.percentage_rescued >= result.percentage_needed?
					"   for next level   \n":
					"   to retry level   \n")
	);
	draw_menu_text(im_bottom,menu_data,0,
		(result.exit_reason == LEVEL_TIMEOUT?8:0) + (msg_lines<=5?8:0),
		message);

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
	return MENU_EXIT_GAME;
}
