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
#include "main.h"

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

struct InputState {
	// number of (input-)frames since user pressed a button
	// to change the release rate of lemmings
	u8 change_rate_hold;
	// time elapsed since user pressed the nuke button the last time.
	// 0 if this is long ago.
	u8 time_since_nuke_pressed;
	u8 speed_up;
	u8 nonprio_lem;
};


int read_io(
		struct Level* level,
		struct LevelState* state,
		struct Lemming lemmings[MAX_NUM_OF_LEMMINGS],
		struct InputState* io_state) {
	u32 kDown;
	u32 kHeld;
	touchPosition stylus;
	circlePosition circle_pos;
	hidScanInput();
	kDown = hidKeysDown();
	kHeld = hidKeysHeld();
	//kUp = hidKeysUp();
	hidTouchRead(&stylus);
	hidCircleRead(&circle_pos);
	circle_pos = cpad_to_movement(circle_pos);

	u32 action = get_action(kDown, kHeld);
	if (action & ACTION_QUIT_GAME) {
		return 0; //exit game
	}
	if (action & ACTION_MOVE_CURSOR_RIGHT) {
		state->cursor.x += 2;
	}
	if (action & ACTION_MOVE_CURSOR_LEFT) {
		state->cursor.x -= 2;
	}
	if (action & ACTION_MOVE_CURSOR_UP) {
		state->cursor.y -= 2;
	}
	if (action & ACTION_MOVE_CURSOR_DOWN) {
		state->cursor.y += 2;
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
		state->cursor.x += x;
		state->cursor.y += y;
	}
	if (state->cursor.y < 0) {
		state->cursor.y = 0;
	}
	if (state->cursor.y >= 200) {
		state->cursor.y = 200;
	}
	if (state->cursor.x < 0) {
		state->cursor.x = 0;
	}
	if (state->cursor.x >= SCREEN_WIDTH) {
		state->cursor.x = SCREEN_WIDTH-1;
	}
	if (state->cursor.x == 0 && state->cursor.y < 160) {
		action |= ACTION_SCROLL_LEFT;
	}
	if (state->cursor.x == SCREEN_WIDTH-1 && state->cursor.y < 160) {
		action |= ACTION_SCROLL_RIGHT;
	}
	if (((kDown | kHeld) & KEY_TOUCH) &&
			stylus.py >= BOTTOM_SCREEN_Y_OFFSET &&
			stylus.py < 200 + BOTTOM_SCREEN_Y_OFFSET &&
			stylus.px < SCREEN_WIDTH) {
		state->cursor.x = stylus.px;
		state->cursor.y = (s16)stylus.py - BOTTOM_SCREEN_Y_OFFSET;
		if (kDown & KEY_TOUCH) {
			action |= ACTION_CURSOR_CLICK;
			action |= ACTION_CURSOR_HOLD;
		}else{
			action |= ACTION_CURSOR_HOLD;
		}
	}
	if (state->cursor.x >= SCREEN_WIDTH) {
		state->cursor.x = SCREEN_WIDTH-1;
	}
	if (state->cursor.y >= 200-6) {
		state->cursor.y = 200-7;
	}
	if (action & (ACTION_CURSOR_CLICK | ACTION_CURSOR_HOLD)) {
		if (state->cursor.y >= 176 && state->cursor.y < 200) {
			// clicked at panel
			switch (state->cursor.x / 16) {
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
						s16 new_x_pos = (state->cursor.x - 13*16) * 16;
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
		state->paused = !state->paused;
		io_state->time_since_nuke_pressed = 0;
	}
	if (action & ACTION_NEXT_SKILL) {
		state->selected_skill = (state->selected_skill+1)%8;
		play_sound(0x01);
	}
	if (action & ACTION_PREV_SKILL) {
		if (state->selected_skill > 0) {
			state->selected_skill--;
		}else{
			state->selected_skill = 7;
		}
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_CIMBER) {
		state->selected_skill = 0;
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_FLOATER) {
		state->selected_skill = 1;
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_BOMBER) {
		state->selected_skill = 2;
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_BLOCKER) {
		state->selected_skill = 3;
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_BUILDER) {
		state->selected_skill = 4;
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_BASHER) {
		state->selected_skill = 5;
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_MINER) {
		state->selected_skill = 6;
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_DIGGER) {
		state->selected_skill = 7;
		play_sound(0x01);
	}
	if (action & ACTION_INC_RATE) {
		s8 inc = 0;
		if (!io_state->change_rate_hold) {
			inc = step_width_start();
			io_state->change_rate_hold++;
		}else{
			io_state->change_rate_hold++;
			if (io_state->change_rate_hold % 12 == 0) {
				io_state->change_rate_hold = 6;
				inc = step_width_running();
			}
		}
		if (state->cur_rate+inc < 100) {
			state->cur_rate += inc;
		} else {
			state->cur_rate = 99;
		}
	}
	if (action & ACTION_DEC_RATE) {
		s8 dec = 0;
		if (!io_state->change_rate_hold) {
			dec = step_width_start();
			io_state->change_rate_hold++;
		}else{
			io_state->change_rate_hold++;
			if (io_state->change_rate_hold % 12 == 0) {
				io_state->change_rate_hold = 6;
				dec = step_width_running();
			}
		}
		if (state->cur_rate > dec+1) {
			state->cur_rate -= dec;
		} else {
			state->cur_rate = 1;
		}
		if (state->cur_rate < level->info.rate) {
			state->cur_rate = level->info.rate;
		}
	}
	if (!(action & (ACTION_INC_RATE | ACTION_DEC_RATE))) {
		io_state->change_rate_hold = 0;
	}
	if (!state->paused && (action & (ACTION_NUKE | ACTION_NUKE_IMMEDIATELY))) {
		if ((action & ACTION_NUKE_IMMEDIATELY)
				|| (io_state->time_since_nuke_pressed
					&& io_state->time_since_nuke_pressed < 45)) {
			nuke(state,level);
		}else{
			io_state->time_since_nuke_pressed = 1;
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
	if (action & ACTION_SPEED_UP) {
		io_state->speed_up = 1;
	}else{
		io_state->speed_up = 0;
	}
	if (action & ACTION_QUIT) {
		if (!state->fade_out) {
			state->fade_out = 1;
		}
	}
	if (action & ACTION_NONPRIORIZED_LEMMING) {
		io_state->nonprio_lem = 1;
	}else{
		io_state->nonprio_lem = 0;
	}
	struct Lemming* lem1 = 0;
	struct Lemming* lem2 = 0;
	if (state->cursor.y < 160) {
		s16 l1idx = -1;
		s16 l2idx = -1;
		select_lemming(
				lemmings,
				(s16)level->info.x_pos + state->cursor.x,
				state->cursor.y,
				(action & ACTION_NONPRIORIZED_LEMMING)?1:0,
				&l1idx,
				&l2idx);
		if (l1idx >= 0 && l1idx < MAX_NUM_OF_LEMMINGS) {
			lem1 = lemmings+l1idx;
			if (l2idx >= 0 && l2idx < MAX_NUM_OF_LEMMINGS) {
				lem2 = lemmings+l2idx;
			}
		}
	}

	if (action & ACTION_CURSOR_CLICK) {
		if (lem1) {
			if (level->info.skills[state->selected_skill]) {
				if (assign_skill(state->selected_skill, lem1, lem2, level)) {
					level->info.skills[state->selected_skill]--;
				}
			}
		}
	}
	if (!state->paused) {
		if (io_state->time_since_nuke_pressed) {
			io_state->time_since_nuke_pressed++;
		}
	}
	return 1;
}

int level_step(
		u8 game,
		u8 lvl,
		struct MainInGameData* main_data,
		struct Level* level,
		struct LevelState* state,
		struct Lemming lemmings[MAX_NUM_OF_LEMMINGS]) {
	if (ENABLE_ENTRANCE_PAUSING_GLITCH || !state->paused) {
		if (state->opening_counter <= 55) {
			state->opening_counter++;
			// do action depending on _new:
			if (state->opening_counter == 15) {
				// play sound: lets go
				play_sound(0x03);
			}else if (state->opening_counter == 35) {
				// play sound: opening entrance
				play_sound(0x02);
				// start opening of entrances;
				state->entrances_open = 1;
			}else if (state->opening_counter == 55) {
				// start background music
				play_music();
			}
		}
	}

	if (state->fade_in) {
		state->fade_in --;
	}else if (state->fade_out) {
		state->fade_out ++;
		if (state->fade_out > FADE_OUT_DOSFRAMES) {
			// exit
			return 0;
		}
	}

	if (!state->paused) {
		int i;

		if (state->entrances_open) {
			add_lemming(lemmings,
					level,
					state);
		}
		if (state->frames_left > 0 && !state->fade_out) {
			state->frames_left--;
		}
		update_lemmings(lemmings,level,state,main_data->masks);
		for (i=0;i<32;i++) {
			// process object!
			if (!(level->obj[i].modifier & OBJECT_USED)) {
				continue;
			}
			if (level->obj[i].type == 1) {
				// start
				if (state->entrances_open) {
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
	if (count_lemmings(lemmings) + lemmings_left(state,level->info.lemmings) == 0
			|| state->frames_left == 0) {
		if (!state->fade_out) {
			state->fade_out = 1;
		}
	}
	return 1;
}

void render_level_frame(
		u8 game,
		u8 lvl,
		struct MainInGameData* main_data,
		struct Level* level,
		struct LevelState* state,
		struct Lemming lemmings[MAX_NUM_OF_LEMMINGS],
		struct InputState* io_state) {
	// compute fading
	float fade = 1.0;
	if (state->fade_in) {
		fade = ((float)(FADE_IN_DOSFRAMES-state->fade_in))
				/ ((float)FADE_IN_DOSFRAMES);
	}else if (state->fade_out) {
		fade = ((float)(FADE_OUT_DOSFRAMES-state->fade_out))
				/ ((float)FADE_OUT_DOSFRAMES);
	}
	u32 level_palette_faded[16];
	memcpy(level_palette_faded,level->palette,4*16);
	fade_palette(level_palette_faded, fade);
	u32 highperf_palette_faded[16];
	memcpy(highperf_palette_faded,main_data->high_perf_palette,4*16);
	fade_palette(highperf_palette_faded, fade);

	begin_frame();
	//clear(BOTTOM_SCREEN);
	draw_level(
			BOTTOM_SCREEN,
			0,
			BOTTOM_SCREEN_Y_OFFSET,
			320,
			160,
			level,
			lemmings,
			main_data,
			level_palette_faded);

	char text[41+10];
	// example: "FALLER  1     OUT 3    IN  0%  TIME 4-54"
	char tmp_text[10];
	memset(tmp_text,' ',9);
	tmp_text[9] = 0;
	u8 n_lem = 0;
	struct Lemming* lem1 = 0;
	if (state->cursor.y < 160) {
		s16 l1idx = -1;
		s16 l2idx = -1;
		n_lem = select_lemming(
				lemmings,
				(s16)level->info.x_pos + state->cursor.x,
				state->cursor.y,
				io_state->nonprio_lem,
				&l1idx,
				&l2idx);
		if (l1idx >= 0 && l1idx < MAX_NUM_OF_LEMMINGS) {
			lem1 = lemmings+l1idx;
		}
	}
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
	int min = (state->frames_left+FPS-1)/FPS/60;
	int sec = (state->frames_left+FPS-1)/FPS%60;
	u16 percentage = 0;
	if (level->info.lemmings != 0) {
		percentage = (100*(u16)level->rescued) / ((u16)level->info.lemmings);
	}
	u8 num_lems = count_lemmings(lemmings);
	sprintf(text,"%s     OUT%2u    IN%3u%%  TIME %d-%02d",
			tmp_text,num_lems<99?num_lems:99,
			percentage<100?percentage:100,
			(min<10)?min:9, (min<10)?sec:59);
	draw_toolbar(
			main_data,
			level,
			state,
			lemmings,
			text,
			highperf_palette_faded);
	clear_rectangle(
			BOTTOM_SCREEN,
			0,
			BOTTOM_SCREEN_Y_OFFSET+200,
			SCREEN_WIDTH,
			240-(BOTTOM_SCREEN_Y_OFFSET+200));


	draw(
			BOTTOM_SCREEN,
			state->cursor.x-7,
			state->cursor.y-7+BOTTOM_SCREEN_Y_OFFSET,
			lem1?cursor_active_data:cursor_data,
			14,
			14,
			level_palette_faded);

	clear_rectangle(BOTTOM_SCREEN,0,0,SCREEN_WIDTH,BOTTOM_SCREEN_Y_OFFSET);
	char above_text[100];
	const char* lvl_name_stripped = level->info.name;
	while (*lvl_name_stripped == ' ') {
		lvl_name_stripped++;
	}
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
	draw_highperf_text(
			BOTTOM_SCREEN,
			0,
			0,
			main_data,
			above_text,
			highperf_palette_faded);

	/* copy_from_backbuffer(TOP_SCREEN); // not really necessary every time
	draw_level( // only for testing purpose
			TOP_SCREEN,
			0,
			80,
			400,
			160,
			level,
			lemmings,
			main_data);
	*/
	end_frame();
}


struct LevelResult run_level(
		u8 game,
		u8 lvl,
		struct MainMenuData* menu_data,
		struct MainInGameData* main_data) {
	// show black screen while loading
	begin_frame();
	clear(BOTTOM_SCREEN);
	end_frame();

	// initialize level
	struct LevelResult result = { 0xFF, 0, 0xFF, LEVEL_ERROR };
	struct Level* level = (struct Level*)malloc(sizeof(struct Level));
	if (!level){
		return result; // error
	}
	memset(level,0,sizeof(struct Level));

	struct InputState io_state;
	memset(&io_state,0,sizeof(struct InputState));
	struct LevelState state;
	memset(&state,0,sizeof(struct LevelState));
	struct Lemming lemmings[MAX_NUM_OF_LEMMINGS];
	if (!read_level(game, lvl, level)) {
		free_objects(level->o);
		free(level);
		return result; // error
	}
	init_level_state(&state,level);
	main_data->high_perf_palette[7] = level->palette[8];
	init_lemmings(lemmings);


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
	prepare_music(game, lvl);

	u64 next_frame = osGetTime();
	u64 next_input = next_frame;
	int apt_result;
	// main loop
	while ((apt_result = aptMainLoop())) {

		if (!next_frame) {
			break;
		}

		// read user input
		do {
			// timing...
			u64 time = osGetTime();
			if (next_input > time) {
				break;
			}
			if (was_suspended()) {
				next_input = time;
				next_frame = time;
			}
			next_input += INPUT_SAMPLING_MILLIS;
			
			if (!read_io(level, &state, lemmings, &io_state)) {
				result.exit_reason = LEVEL_EXIT_GAME;
				free_objects(level->o);
				free(level);
				return result;
			}
		}while(1);

		// update lemmings
		do {
			update_audio();

			// timing...
			u64 time = osGetTime();
			if (next_frame > time) {
				break;
			}
			if (was_suspended()) {
				next_input = time;
				next_frame = time;
			}
			if (io_state.speed_up) {
				next_frame += MS_PER_FRAME_SPEED_UP;
			}else if (level->info.speed_up){
				next_frame += MS_PER_FRAME_SUPERLEM;
			}else{
				next_frame += MS_PER_FRAME;
			}

			if (!level_step(
					game,
					lvl,
					main_data,
					level,
					&state,
					lemmings)) { // simulate one DOS frame (one step)
				next_frame = 0;
				break;
			}
		}while(1);

		// draw frame
		render_level_frame(
					game,
					lvl,
					main_data,
					level,
					&state,
					lemmings,
					&io_state);
	}
	stop_audio();
	if (!apt_result) {
		result.exit_reason = LEVEL_EXIT_GAME;
		free_objects(level->o);
		free(level);
		return result;
	}

	result.lvl = lvl;
	if ((u16)level->info.lemmings > 0) {
		result.percentage_rescued = ((u16)level->rescued)*100
				/ (u16)level->info.lemmings;
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
	free_objects(level->o);
	free(level);
	return result;
}

int show_result(
		u8 game,
		struct LevelResult result,
		struct MainMenuData* menu_data) {
	if (result.exit_reason == LEVEL_ERROR) {
		return MENU_ERROR;
	}
	if (result.exit_reason == LEVEL_EXIT_GAME) {
		return MENU_EXIT_GAME;
	}
	tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);

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
	draw_menu_text(
			BOTTOM_SCREEN_BACK,
			menu_data,
			0,
			(result.exit_reason == LEVEL_TIMEOUT?8:0) + (msg_lines<=5?8:0),
			message,
			0,
			1.0f);

	while (aptMainLoop()) {
		hidScanInput();
		u32 kDown = hidKeysDown();
		if (kDown & (KEY_A | KEY_START | KEY_X | KEY_TOUCH)) {
			// load next level!
			return RESULT_ACTION_NEXT;
		}
		if (kDown & KEY_B) {
			return RESULT_ACTION_CANCEL;
		}
		begin_frame();
		copy_from_backbuffer(BOTTOM_SCREEN);
		copy_from_backbuffer(TOP_SCREEN);
		end_frame();
	}
	return MENU_EXIT_GAME;
}
