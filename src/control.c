#include <string.h>
#include <stdlib.h> // rand()
#include "control.h"
#include "settings.h"
#include "audio.h"
#include "lemming.h"

s16 cpad_to_movement(circlePosition circle_pos, u32 key);

u64 get_action(u32 kDown, u32 kHeld, circlePosition cpad, circlePosition* params) {
	// params[0] -> cursor movement
	// params[1] -> scroll direction
	if (params) {
		memset(params, 0, 2*sizeof(circlePosition));
	}

	int i;
	u32 modifiers = settings.key_bindings[0].modifier | settings.key_bindings[1].modifier;

	u32* controls[34];
	for (i=0;i<2;i++) {
		controls[0+i] = &settings.key_bindings[i].click;
		controls[2+i] = &settings.key_bindings[i].inc_rate; // hold
		controls[4+i] = &settings.key_bindings[i].dec_rate; // hold
		controls[6+i] = &settings.key_bindings[i].next_skill;
		controls[8+i] = &settings.key_bindings[i].prev_skill;
		controls[10+i] = &settings.key_bindings[i].pause;
		controls[12+i] = &settings.key_bindings[i].nuke;
		controls[14+i] = &settings.key_bindings[i].exit;
		controls[16+i] = &settings.key_bindings[i].speed_up;
		controls[18+i] = &settings.key_bindings[i].non_prio;
		controls[20+i] = &settings.key_bindings[i].step_one_frame;
		controls[22+i] = &settings.key_bindings[i].cursor_up; // special
		controls[24+i] = &settings.key_bindings[i].cursor_down; // special
		controls[26+i] = &settings.key_bindings[i].cursor_left; // special
		controls[28+i] = &settings.key_bindings[i].cursor_right; // special
		controls[30+i] = &settings.key_bindings[i].scroll_left; // special
		controls[32+i] = &settings.key_bindings[i].scroll_right; // special
	}
	#define CPAD (KEY_CPAD_UP | KEY_CPAD_DOWN | KEY_CPAD_LEFT | KEY_CPAD_RIGHT)
	u64 action_down[34] = {
		ACTION_CURSOR_CLICK,
		ACTION_CURSOR_CLICK,
		ACTION_INC_RATE,
		ACTION_INC_RATE,
		ACTION_DEC_RATE,
		ACTION_DEC_RATE,
		ACTION_NEXT_SKILL,
		ACTION_NEXT_SKILL,
		ACTION_PREV_SKILL,
		ACTION_PREV_SKILL,
		ACTION_PAUSE,
		ACTION_PAUSE,
		settings.dlbclick_nuke?ACTION_NUKE:ACTION_NUKE_IMMEDIATELY,
		settings.dlbclick_nuke?ACTION_NUKE:ACTION_NUKE_IMMEDIATELY,
		ACTION_QUIT, // TODO: immediately or dbl click required?
		ACTION_QUIT, // TODO: immediately or dbl click required?
		ACTION_SPEED_UP,
		ACTION_SPEED_UP,
		ACTION_NONPRIORIZED_LEMMING,
		ACTION_NONPRIORIZED_LEMMING,
		ACTION_STEP_FRAME,
		ACTION_STEP_FRAME,
		(*(controls[22]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_UP,
		(*(controls[23]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_UP,
		(*(controls[24]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_DOWN,
		(*(controls[25]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_DOWN,
		(*(controls[26]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_LEFT,
		(*(controls[27]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_LEFT,
		(*(controls[28]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_RIGHT,
		(*(controls[29]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_RIGHT,
		(*(controls[30]) & CPAD)?ACTION_SCROLL_PARAM:ACTION_SCROLL_LEFT,
		(*(controls[31]) & CPAD)?ACTION_SCROLL_PARAM:ACTION_SCROLL_LEFT,
		(*(controls[32]) & CPAD)?ACTION_SCROLL_PARAM:ACTION_SCROLL_RIGHT,
		(*(controls[33]) & CPAD)?ACTION_SCROLL_PARAM:ACTION_SCROLL_RIGHT
	};
	u64 action_held[34] = {
		ACTION_CURSOR_HOLD,
		ACTION_CURSOR_HOLD,
		ACTION_INC_RATE,
		ACTION_INC_RATE,
		ACTION_DEC_RATE,
		ACTION_DEC_RATE,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		ACTION_SPEED_UP,
		ACTION_SPEED_UP,
		ACTION_NONPRIORIZED_LEMMING,
		ACTION_NONPRIORIZED_LEMMING,
		0,
		0,
		(*(controls[22]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_UP,
		(*(controls[23]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_UP,
		(*(controls[24]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_DOWN,
		(*(controls[25]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_DOWN,
		(*(controls[26]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_LEFT,
		(*(controls[27]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_LEFT,
		(*(controls[28]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_RIGHT,
		(*(controls[29]) & CPAD)?ACTION_MOVE_CURSOR_PARAM:ACTION_MOVE_CURSOR_RIGHT,
		(*(controls[30]) & CPAD)?ACTION_SCROLL_PARAM:ACTION_SCROLL_LEFT,
		(*(controls[31]) & CPAD)?ACTION_SCROLL_PARAM:ACTION_SCROLL_LEFT,
		(*(controls[32]) & CPAD)?ACTION_SCROLL_PARAM:ACTION_SCROLL_RIGHT,
		(*(controls[33]) & CPAD)?ACTION_SCROLL_PARAM:ACTION_SCROLL_RIGHT
	};

	u64 ret = 0;
	u32 mask;
	for (mask = 1; mask != 0; mask <<= 1) {
		u32 used_modifiers = 0;
		u8 down = 255; // invalid
		u8 held = 255; // invalid
		if ((mask & (kDown | kHeld) & ~modifiers) == 0) {
			continue;
		}
		for (i=0;i<34;i++) {
			u32 ctrl = (*(controls[i]) & ~modifiers);
			u32 mod = (*(controls[i]) & modifiers);
			if (!ctrl) {
				continue;
			}
			if (mod == (mod & kHeld)) {
				if (ctrl == (ctrl & mask)) {
					if ((used_modifiers & kHeld) == used_modifiers) {
						if (mask & kDown) {
							down = i;
							held = i;
						}else if (mask & kHeld) {
							held = i;
						}
						used_modifiers = (kHeld & modifiers);
					}else if (used_modifiers != (kHeld & modifiers & *(controls[i]))){
						down = 255;
						held = 255;
						used_modifiers |= (kHeld & modifiers & *(controls[i]));
					}
				}
			}
		}
		s16 add = 0;
		if (mask & CPAD) {
			add = cpad_to_movement(cpad, mask);
		}
		int add_0 = 0;
		int add_1 = 0;
		if (held != 255) {
			ret |= action_held[held];
			if (action_held[held] & ACTION_MOVE_CURSOR_PARAM & ~(down!=255?action_down[down]:0)) {
				add_0 = held/2 - 10;
			}
			if (action_held[held] & ACTION_SCROLL_PARAM & ~(down!=255?action_down[down]:0)) {
				add_1 = held/2 - 14;
			}
		}
		if (down != 255) {
			ret |= action_down[down];
			if (action_down[down] & ACTION_MOVE_CURSOR_PARAM) {
				add_0 = down/2 - 10;
			}
			if (action_down[down] & ACTION_SCROLL_PARAM) {
				add_1 = down/2 - 14;
			}
		}
		switch (add_0) {
			case 1:
				// up
				params[0].dy -= add;
				break;
			case 2:
				// down
				params[0].dy += add;
				break;
			case 3:
				// left
				params[0].dx -= add;
				break;
			case 4:
				// right
				params[0].dx += add;
				break;
		}
		switch (add_1) {
			case 1:
				// left
				params[1].dx -= add;
				break;
			case 2:
				// right
				params[1].dx += add;
				break;
		}
	}
	return ret;
}


s16 cpad_to_movement(circlePosition circle_pos, u32 key) {
	if (circle_pos.dx > 160) {
		circle_pos.dx = 160;
	}
	if (circle_pos.dx < -160) {
		circle_pos.dx = -160;
	}
	if (circle_pos.dy > 160) {
		circle_pos.dy = 160;
	}
	if (circle_pos.dy < -160) {
		circle_pos.dy = -160;
	}
	if (circle_pos.dx < 40 && circle_pos.dx > -40) {
		circle_pos.dx = 0;
	}
	if (circle_pos.dy < 40 && circle_pos.dy > -40) {
		circle_pos.dy = 0;
	}
	if (circle_pos.dx >= 80) {
		circle_pos.dx -= 20;
	}
	if (circle_pos.dx <= -80) {
		circle_pos.dx += 20;
	}
	if (circle_pos.dy >= 80) {
		circle_pos.dy -= 20;
	}
	if (circle_pos.dy <= -80) {
		circle_pos.dy += 20;
	}
	if (circle_pos.dx >= 40) {
		circle_pos.dx -= 20;
	}
	if (circle_pos.dx <= -40) {
		circle_pos.dx += 20;
	}
	if (circle_pos.dy >= 40) {
		circle_pos.dy -= 20;
	}
	if (circle_pos.dy <= -40) {
		circle_pos.dy += 20;
	}
	circle_pos.dx/=20;
	circle_pos.dy/=-20;
	s16 ret = 0;
	if (key & KEY_CPAD_UP) {
		ret -= circle_pos.dy;
	}
	if (key & KEY_CPAD_DOWN) {
		ret += circle_pos.dy;
	}
	if (key & KEY_CPAD_LEFT) {
		ret -= circle_pos.dx;
	}
	if (key & KEY_CPAD_RIGHT) {
		ret += circle_pos.dx;
	}
	return ret;
}

void init_io_state(struct InputState* io_state, u16 x_pos) {
	if (!io_state) {
		return;
	}
	memset(io_state,0,sizeof(struct InputState));
	io_state->cursor.x = 142; // TODO: SCREEN_WIDTH/2 - 18 ??
	io_state->cursor.y = 92;
	io_state->x_pos = x_pos;
	if (io_state->x_pos >= (SCREEN_WIDTH-320)/2) {
		io_state->x_pos -= (SCREEN_WIDTH-320)/2;
	}else{
		io_state->x_pos = 0;
	}
	if (SCREEN_WIDTH < 1584 && io_state->x_pos + SCREEN_WIDTH >= 1584) {
		io_state->x_pos = 1584 - SCREEN_WIDTH;
	}
	if (SCREEN_WIDTH >= 1584) {
		io_state->x_pos = 0;
	}
}

int add_action(struct InputState* io_state, enum Action action, u8 param, u8 param2, u8 param3) {
	if (!io_state) {
		return 0; // error
	}
	if (io_state->num_actions >= MAX_ACTION_QUEUE_SIZE) {
		return 0; // no space left in queue
	}
	io_state->action_queue[io_state->num_actions].action = action;
	io_state->action_queue[io_state->num_actions].param = param;
	io_state->action_queue[io_state->num_actions].param2 = param2;
	io_state->action_queue[io_state->num_actions].param3 = param3;
	io_state->num_actions++;
	return 1;
}

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

int read_io(struct Level* level, struct InputState* io_state, u8 player) {
	u32 kDown;
	u32 kHeld;
	touchPosition stylus;
	circlePosition circle_pos;
	circlePosition params[2] = {{0,0}, {0,0}};
	hidScanInput();
	kDown = hidKeysDown();
	kHeld = hidKeysHeld();
	//kUp = hidKeysUp();
	hidTouchRead(&stylus);
	hidCircleRead(&circle_pos);
	// circle_pos = cpad_to_movement(circle_pos);
	u64 action = get_action(kDown, kHeld, circle_pos, params);
	if (action & ACTION_QUIT_GAME) {
		return 0; //exit game
	}
	if (action & ACTION_MOVE_CURSOR_RIGHT) {
		io_state->cursor.x += 2;
	}
	if (action & ACTION_MOVE_CURSOR_LEFT) {
		io_state->cursor.x -= 2;
	}
	if (action & ACTION_MOVE_CURSOR_UP) {
		io_state->cursor.y -= 2;
	}
	if (action & ACTION_MOVE_CURSOR_DOWN) {
		io_state->cursor.y += 2;
	}
	if (action & ACTION_MOVE_CURSOR_PARAM) {
		io_state->cursor.x += params[0].dx;
		io_state->cursor.y += params[0].dy;
	}
	if (io_state->cursor.y < 0) {
		io_state->cursor.y = 0;
	}
	if (io_state->cursor.y >= 200) {
		io_state->cursor.y = 200;
	}
	if (io_state->cursor.x < 0) {
		io_state->cursor.x = 0;
	}
	if (io_state->cursor.x >= SCREEN_WIDTH) {
		io_state->cursor.x = SCREEN_WIDTH-1;
	}
	if (io_state->cursor.x == 0 && io_state->cursor.y < 160) {
		action |= ACTION_SCROLL_LEFT;
	}
	if (io_state->cursor.x == SCREEN_WIDTH-1 && io_state->cursor.y < 160) {
		action |= ACTION_SCROLL_RIGHT;
	}
	if (((kDown | kHeld) & KEY_TOUCH) &&
			stylus.py >= BOTTOM_SCREEN_Y_OFFSET &&
			stylus.py < 200 + BOTTOM_SCREEN_Y_OFFSET &&
			stylus.px < SCREEN_WIDTH) {
		io_state->cursor.x = stylus.px;
		io_state->cursor.y = (s16)stylus.py - BOTTOM_SCREEN_Y_OFFSET;
		if (kDown & KEY_TOUCH) {
			action |= ACTION_CURSOR_CLICK;
			action |= ACTION_CURSOR_HOLD;
		}else{
			action |= ACTION_CURSOR_HOLD;
		}
	}
	if (io_state->cursor.x >= SCREEN_WIDTH) {
		io_state->cursor.x = SCREEN_WIDTH-1;
	}
	if (io_state->cursor.y > 200-7) {
		io_state->cursor.y = 200-7;
	}
	if (action & (ACTION_CURSOR_CLICK | ACTION_CURSOR_HOLD)) {
		if (io_state->cursor.y >= 176 && io_state->cursor.y < 200) {
			// clicked at panel
			switch (io_state->cursor.x / 16) {
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
						s16 new_x_pos = (io_state->cursor.x - 13*16) * 16;
						new_x_pos -= SCREEN_WIDTH / 2;
						if (new_x_pos < 0) {
							new_x_pos = 0;
						}else if (new_x_pos >= 1584 - SCREEN_WIDTH) {
							new_x_pos = (SCREEN_WIDTH <= 1584?1584 - SCREEN_WIDTH:0);
						}
						io_state->x_pos = new_x_pos;
					}
			}
		}
	}
	if (action & ACTION_PAUSE) {
		add_action(io_state, ACTIONQUEUE_TOGGLE_PAUSE, 0, 0, 0);
		io_state->time_since_nuke_pressed = 0;
	}
	if (action & ACTION_NEXT_SKILL) {
		io_state->skill = (io_state->skill+1)%8;
		play_sound(0x01);
	}
	if (action & ACTION_PREV_SKILL) {
		if (io_state->skill > 0) {
			io_state->skill--;
		}else{
			io_state->skill = 7;
		}
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_CIMBER) {
		io_state->skill = 0;
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_FLOATER) {
		io_state->skill = 1;
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_BOMBER) {
		io_state->skill = 2;
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_BLOCKER) {
		io_state->skill = 3;
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_BUILDER) {
		io_state->skill = 4;
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_BASHER) {
		io_state->skill = 5;
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_MINER) {
		io_state->skill = 6;
		play_sound(0x01);
	}
	if (action & ACTION_SELECT_SKILL_DIGGER) {
		io_state->skill = 7;
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
		add_action(io_state, ACTIONQUEUE_CHANGE_RATE, (u8)inc, 0, 0);
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
		add_action(io_state, ACTIONQUEUE_CHANGE_RATE, (u8)(-dec), 0, 0);
	}
	if (!(action & (ACTION_INC_RATE | ACTION_DEC_RATE))) {
		io_state->change_rate_hold = 0;
	}
	if (!level->paused && (action & (ACTION_NUKE | ACTION_NUKE_IMMEDIATELY))) {
		if ((action & ACTION_NUKE_IMMEDIATELY)
				|| (io_state->time_since_nuke_pressed
					&& io_state->time_since_nuke_pressed < 45)) {
			add_action(io_state, ACTIONQUEUE_NUKE, 0, 0, 0);
		}else{
			io_state->time_since_nuke_pressed = 1;
		}
	}
	if (action & ACTION_SCROLL_RIGHT) {
		io_state->x_pos+=3;
		if (io_state->x_pos > 1584-SCREEN_WIDTH) {
			if (1584>SCREEN_WIDTH) {
				io_state->x_pos = 1584-SCREEN_WIDTH;
			} else {
				io_state->x_pos = 0;
			}
		}
	}
	if (action & ACTION_SCROLL_LEFT) {
		if ((s16)io_state->x_pos >= 3) {
			io_state->x_pos-=3;
		}else{
			io_state->x_pos = 0;
		}
	}
	if (action & ACTION_SCROLL_PARAM) {
		s16 x = params[1].dx;
		if (x < 0) {
			if ((s16)io_state->x_pos >= -x) {
				io_state->x_pos+=x;
			}else{
				io_state->x_pos = 0;
			}
	}else{
			io_state->x_pos+=x;
			if (io_state->x_pos > 1584-SCREEN_WIDTH) {
				if (1584>SCREEN_WIDTH) {
					io_state->x_pos = 1584-SCREEN_WIDTH;
				} else {
					io_state->x_pos = 0;
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
		add_action(io_state, ACTIONQUEUE_NUKE, 1, 0, 0);
	}
	if (action & ACTION_NONPRIORIZED_LEMMING) {
		io_state->nonprio_lem = 1;
	}else{
		io_state->nonprio_lem = 0;
	}
	struct Lemming* lem1 = 0;
	struct Lemming* lem2 = 0;
	if (io_state->cursor.y < 160) {
		s16 l1idx = -1;
		s16 l2idx = -1;
		select_lemming(
				level->player[player].lemmings,
				(s16)io_state->x_pos + io_state->cursor.x,
				io_state->cursor.y,
				(action & ACTION_NONPRIORIZED_LEMMING)?1:0,
				&l1idx,
				&l2idx);
		if (l1idx >= 0 && l1idx < MAX_NUM_OF_LEMMINGS) {
			lem1 = &level->player[player].lemmings[l1idx];
			if (l2idx >= 0 && l2idx < MAX_NUM_OF_LEMMINGS) {
				lem2 = &level->player[player].lemmings[l2idx];
			}
		}
	}

	if (action & ACTION_CURSOR_CLICK) {
		if (lem1) {
			u8 param2 = 0xFF;
			if (lem2) {
				param2 = lem2 - level->player[player].lemmings;
			}
			add_action(io_state, ACTIONQUEUE_ASSIGN_SKILL, lem1 - level->player[player].lemmings, param2, io_state->skill);
		}
	}
	if (!level->paused) {
		if (io_state->time_since_nuke_pressed) {
			io_state->time_since_nuke_pressed++;
		}
	}else {
		if (action & ACTION_STEP_FRAME) {
			add_action(io_state, ACTIONQUEUE_FRAME_FORWARD, 1, 0, 0);
		}
	}
	return 1;
}

int process_action_queue(
		struct ActionQueue* action_queue,
		u8 num_actions,
		struct Level* level,
		u8 player_id,
		u8 multiplayer) {
	int changes = 0;
	// apply actions from action_queue
	u8 i;
	for (i=0; i<num_actions; i++) {
		switch (action_queue[i].action) {
			case ACTIONQUEUE_NUKE:
				if (!action_queue[i].param) {
					if (!multiplayer) {
						nuke(&level->player[player_id]);
					}else{
						// request nuke in 2p mode
						level->player[player_id].request_common_nuke = COMMON_NUKE_FRAME_INTERVAL;
						u8 p;
						u8 start_nuking = 1;
						for (p=0;p<level->num_players;p++) {
							if (!level->player[p].request_common_nuke) {
								start_nuking = 0;
								break;
							}
						}
						if (start_nuking) {
							for (p=0;p<level->num_players;p++) {
								nuke(&level->player[p]);
							}
						}
					}
				}else{
					if (!level->fade_out && !multiplayer) {
						level->fade_out = 1;
					}else{
						action_queue[i].action = ACTIONQUEUE_NOP;
					}
				}
				break;
			case ACTIONQUEUE_ASSIGN_SKILL:
				{
					struct Lemming* lem1 = &level->player[player_id].lemmings[action_queue[i].param];
					struct Lemming* lem2 = 0;
					if (action_queue[i].param2 < MAX_NUM_OF_LEMMINGS) {
						lem2 = &level->player[player_id].lemmings[action_queue[i].param2];
					}
					if (level->player[player_id].skills[action_queue[i].param3]) {
						u8 lem = assign_skill(action_queue[i].param3, lem1, lem2, level);
						if (lem) {
							level->player[player_id].skills[action_queue[i].param3]--;
							changes = 1;
							if (lem == 2) {
								action_queue[i].param = action_queue[i].param2;
							}
							action_queue[i].param2 = 0;
						}else{
							action_queue[i].action = ACTIONQUEUE_NOP;
						}
					} else {
						action_queue[i].action = ACTIONQUEUE_NOP;
					}
				}
				break;
			case ACTIONQUEUE_TOGGLE_PAUSE:
				if (!multiplayer) {
					level->paused = !level->paused;
				}else{
					action_queue[i].action = ACTIONQUEUE_NOP;
				}
				break;
			case ACTIONQUEUE_FRAME_FORWARD:
				if (!multiplayer) {
					s8 inc = (s8)action_queue[i].param;
					if (inc >= 0) {
						level->frame_step_forward += inc;
					}else{
						// NOT IMPLEMENTED YET
						action_queue[i].action = ACTIONQUEUE_NOP;
					}
				}else{
					action_queue[i].action = ACTIONQUEUE_NOP;
				}
				break;
			case ACTIONQUEUE_CHANGE_RATE:
				if (!multiplayer) {
					s8 inc = (s8)action_queue[i].param;
					if (inc >= 0) {
						if (level->cur_rate+inc < 100) {
							level->cur_rate += inc;
						}else{
							level->cur_rate = 99;
						}
					}else{
						if (level->cur_rate + inc >= level->rate) {
							level->cur_rate += inc;
						} else {
							level->cur_rate = level->rate;
						}
					}
				}else{
					action_queue[i].action = ACTIONQUEUE_NOP;
				}
				break;
			case ACTIONQUEUE_NOP:
			default:
				// no action or invalid action in queue
				action_queue[i].action = ACTIONQUEUE_NOP;
				break;
		}
	}
	return changes;
}

