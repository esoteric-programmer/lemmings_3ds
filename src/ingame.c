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

int level_step(
		struct MainInGameData* main_data,
		struct Level* level,
		u8* lemming_inout) {
	if (lemming_inout) {
		*lemming_inout = 0;
	}
	if (settings.glitch_entrance_pausing || !level->paused || level->frame_step_forward) {
		if (level->opening_counter <= 55) {
			level->opening_counter++;
			// do action depending on _new:
			if (level->opening_counter == 15) {
				// play sound: lets go
				play_sound(0x03);
			}else if (level->opening_counter == 35) {
				// play sound: opening entrance
				play_sound(0x02);
				// start opening of entrances;
				level->entrances_open = 1;
			}else if (level->opening_counter == 55) {
				// start background music
				play_music();
			}
		}
	}

	if (level->fade_in) {
		level->fade_in--;
	}else if (level->fade_out) {
		level->fade_out++;
		if (level->fade_out > FADE_OUT_DOSFRAMES) {
			// exit
			return 0;
		}
	}

	if (!level->paused || level->frame_step_forward) {
		// count down common nuke counter
		u8 p;
		for (p=0;p<level->num_players;p++) {
			if (level->player[p].request_common_nuke) {
				level->player[p].request_common_nuke--;
			}
		}
		int i;
		if (level->frame_step_forward) {
			level->frame_step_forward--;
		}
		if (level->entrances_open) {
			if (add_lemming(level)) {
				if (lemming_inout) {
					*lemming_inout = 1;
				}
			}
		}
		if (level->frames_left > 0 && !level->fade_out) {
			level->frames_left--;
		}
		u8 lemmings_exit = 0;
		if (lemming_inout) {
			for (p=0;p<level->num_players;p++) {
				u8 q;
				for (q=0;q<level->num_players;q++) {
					lemmings_exit+=level->player[p].rescued[q];
				}
			}
		}
		update_lemmings(level,main_data->masks);
		if (lemming_inout) {
			u8 lemmings_exit_afterwards = 0;
			for (p=0;p<level->num_players;p++) {
				u8 q;
				for (q=0;q<level->num_players;q++) {
					lemmings_exit_afterwards+=level->player[p].rescued[q];
				}
			}
			if (lemmings_exit_afterwards != lemmings_exit) {
				*lemming_inout = 1;
			}
		}
		for (i=0;i<32;i++) {
			// process object!
			struct ObjectType* obj_type = level->object_types[level->object_instances[i].type];
			if (!(level->object_instances[i].modifier & OBJECT_USED) || !obj_type) {
				continue;
			}
			if (level->object_instances[i].type == 1) {
				// start
				if (level->entrances_open) {
					if (level->object_instances[i].current_frame) {
						level->object_instances[i].current_frame++;
						if (level->object_instances[i].current_frame >= obj_type->end_frame) {
							level->object_instances[i].current_frame = 0;
						}
					}
				}
				continue;
			}
			if (obj_type->trigger == OBJECT_TRAP && !level->object_instances[i].current_frame) {
				// trap...
				continue;
			}
			level->object_instances[i].current_frame++;
			if (level->object_instances[i].current_frame >= obj_type->end_frame) {
				level->object_instances[i].current_frame = obj_type->start_frame; // TODO: set to 0 instead?
			}
		}
	}
	u8 lem_left = 0;
	u16 p;
	for (p=0; p<level->num_players; p++) {
		lem_left += count_lemmings(level->player[p].lemmings) + lemmings_left(&level->player[p]);
	}
	if (lem_left == 0 || level->frames_left == 0) {
		if (!level->fade_out) {
			level->fade_out = 1;
		}
	}
	return 1;
}

void render_level_frame(
		const char* level_id, // e.g. FUN 14
		struct MainInGameData* main_data,
		struct Level* level,
		struct InputState* io_state,
		u8 player) {
	// compute fading
	float fade = 1.0;
	if (level->fade_in) {
		fade = ((float)(FADE_IN_DOSFRAMES-level->fade_in))
				/ ((float)FADE_IN_DOSFRAMES);
	}else if (level->fade_out) {
		fade = ((float)(FADE_OUT_DOSFRAMES-level->fade_out))
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
			level->player[player].x_pos,
			level,
			main_data,
			level_palette_faded);

	char text[41+10];
	// example: "FALLER  1     OUT 3    IN  0%  TIME 4-54"
	char tmp_text[10];
	memset(tmp_text,' ',9);
	tmp_text[9] = 0;
	u8 n_lem = 0;
	struct Lemming* lem1 = 0;
	if (io_state->cursor.y < 160) {
		s16 l1idx = -1;
		s16 l2idx = -1;
		n_lem = select_lemming(
				level->player[player].lemmings,
				(s16)io_state->x_pos + io_state->cursor.x,
				io_state->cursor.y,
				io_state->nonprio_lem,
				&l1idx,
				&l2idx);
		if (l1idx >= 0 && l1idx < MAX_NUM_OF_LEMMINGS) {
			lem1 = &level->player[player].lemmings[l1idx];
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
	int min = (level->frames_left+FPS-1)/FPS/60;
	int sec = (level->frames_left+FPS-1)/FPS%60;
	u16 percentage = 0;
	u16 resc = 0;
	if (level->player[player].max_lemmings != 0) {
		u8 i;
		for (i=0;i<level->num_players;i++) {
			resc += (u16)level->player[player].rescued[i];
		}
		percentage = (100*resc) / ((u16)level->player[player].max_lemmings);
	}
	u8 num_lems = count_lemmings(level->player[player].lemmings);
	if (level->num_players == 1) {
		sprintf(text,"%s     OUT%2u    IN%3u%%  TIME %d-%02d",
				tmp_text,num_lems<99?num_lems:99,
				percentage<100?percentage:100,
				(min<10)?min:9, (min<10)?sec:59);
	}else if (level->num_players == 2) {
		u8 num_lems_2p = count_lemmings(level->player[1-player].lemmings);
		sprintf(text,"%s     OUT%2u:%2u         TIME %d-%02d",
				tmp_text,num_lems<99?num_lems:99,
				num_lems_2p<99?num_lems_2p:99,
				(min<10)?min:9, (min<10)?sec:59);
	}else{
		sprintf(text,"%s                      TIME %d-%02d",
				tmp_text,
				(min<10)?min:9, (min<10)?sec:59);
	}
	draw_toolbar(
			main_data,
			level,
			io_state,
			text,
			highperf_palette_faded,
			player);
	clear_rectangle(
			BOTTOM_SCREEN,
			0,
			BOTTOM_SCREEN_Y_OFFSET+200,
			SCREEN_WIDTH,
			240-(BOTTOM_SCREEN_Y_OFFSET+200));


	draw(
			BOTTOM_SCREEN,
			io_state->cursor.x-7,
			io_state->cursor.y-7+BOTTOM_SCREEN_Y_OFFSET,
			lem1?cursor_active_data:cursor_data,
			14,
			14,
			level_palette_faded);

	clear_rectangle(BOTTOM_SCREEN,0,0,SCREEN_WIDTH,BOTTOM_SCREEN_Y_OFFSET);
	char above_text[100];
	const char* lvl_name_stripped = level->name;
	while (*lvl_name_stripped == ' ') {
		lvl_name_stripped++;
	}
	if (level->num_players == 1) {
		sprintf(above_text,
				"%s%s%s\n"
				"                     You need %3u%% of %2u",
				level_id?level_id:"",
				level_id?" ":"",
				lvl_name_stripped,
				level->percentage_needed,
				level->player[player].max_lemmings);
	}else if (level->num_players == 2) {
		// TODO: don't show rescued lemmings as text, but as graphic
		u16 resc_2p = 0;
		if (level->player[1-player].max_lemmings != 0) {
			u8 i;
			for (i=0;i<level->num_players;i++) {
				resc_2p += (u16)level->player[1-player].rescued[i];
			}
		}
		sprintf(above_text,
				"%s%s%s\n"
				"        In %3u:%3u  Total %2u:%2u  Rate %02u",
				level_id?level_id:"",
				level_id?" ":"",
				lvl_name_stripped,
				resc<999?resc:999,
				resc_2p<999?resc_2p:999,
				level->player[player].max_lemmings,
				level->player[1-player].max_lemmings,
				level->cur_rate<99?level->cur_rate:99);
	}else{
		sprintf(above_text,
				"%s%s%s",
				level_id?level_id:"",
				level_id?" ":"",
				lvl_name_stripped);
				
	}
	draw_highperf_text(
			BOTTOM_SCREEN,
			0,
			0,
			main_data,
			above_text,
			highperf_palette_faded);

	if (level->num_players == 2) {
		// TODO: show rescued lemmings
	}

	/* copy_from_backbuffer(TOP_SCREEN); // not really necessary every time
	draw_level( // only for testing purpose
			TOP_SCREEN,
			0,
			80,
			400,
			160,
			level,
			main_data);
	*/
	end_frame();
}

struct LevelResult run_level(
		struct Level* level,
		const char* level_id,
		struct MainMenuData* menu_data,
		struct MainInGameData* main_data) {
	// show black screen while loading
	begin_frame();
	clear(BOTTOM_SCREEN);
	end_frame();

	struct InputState io_state;
	init_io_state(&io_state, level->player[0].x_pos);

	// initialize level
	struct LevelResult result = { 0xFF, 0, 0xFF, LEVEL_ERROR };

	main_data->high_perf_palette[7] = level->palette[8];
	init_lemmings(level);

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

			if (!read_io(level, &io_state, 0)) {
				result.exit_reason = LEVEL_EXIT_GAME;
				return result;
			}
		}while(1);

		// apply user input
		level->player[0].x_pos = io_state.x_pos;
		// apply actions from action_queue
		process_action_queue(io_state.action_queue, io_state.num_actions, level, 0, 0);
		io_state.num_actions = 0;

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
			}else if (level->speed_up){
				next_frame += MS_PER_FRAME_SUPERLEM;
			}else{
				next_frame += MS_PER_FRAME;
			}

			if (!level_step(
					main_data,
					level,
					0)) { // simulate one DOS frame (one step)
				next_frame = 0;
				break;
			}
		}while(1);

		// draw frame
		render_level_frame(
					level_id,
					main_data,
					level,
					&io_state,
					0);
	}
	stop_audio();
	if (!apt_result) {
		result.exit_reason = LEVEL_EXIT_GAME;
		return result;
	}

	if ((u16)level->player[0].max_lemmings > 0) {
		result.percentage_rescued = ((u16)level->player[0].rescued[0])*100
				/ (u16)level->player[0].max_lemmings;
	}else{
		result.percentage_rescued = 0;
	}
	result.percentage_needed = level->percentage_needed;

	if (result.percentage_rescued >= result.percentage_needed) {
		next_music();
	}

	if (!level->frames_left) {
		result.exit_reason = LEVEL_TIMEOUT;
	}else{
		result.exit_reason = LEVEL_NO_LEMMINGS_LEFT;
	}

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
