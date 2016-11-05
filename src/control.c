#include <string.h>
#include "control.h"
#include "settings.h"

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
