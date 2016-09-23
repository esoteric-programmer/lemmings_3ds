#include "control.h"

struct Controls controls[] = {
	{0, 0, KEY_R, ACTION_MOVE_CURSOR_CPAD},
	{KEY_A, 0, 0, ACTION_CURSOR_CLICK},
	{0, KEY_A, 0, ACTION_CURSOR_HOLD},
	{KEY_B, 0, 0, ACTION_PAUSE},
	{KEY_X, 0, 0, ACTION_NEXT_SKILL},
	{KEY_DRIGHT, 0, 0, ACTION_NEXT_SKILL},
	{KEY_Y, 0, 0, ACTION_PREV_SKILL},
	{KEY_DLEFT, 0, 0, ACTION_PREV_SKILL},
	{0, KEY_DUP, 0, ACTION_INC_RATE},
	{0, KEY_DDOWN, 0, ACTION_DEC_RATE},
	{0, KEY_L, 0, ACTION_SPEED_UP},
	{0, KEY_R, 0, ACTION_SCROLL_CPAD},
	{0, KEY_CPAD_DOWN | KEY_R, 0, ACTION_MOVE_CURSOR_CPAD},
	{0, KEY_CPAD_UP | KEY_R, 0, ACTION_MOVE_CURSOR_CPAD},
	{0, KEY_SELECT, 0, ACTION_NONPRIORIZED_LEMMING},
	{KEY_START, 0, 0, ACTION_NUKE},
	{0, 0, 0, 0}
};

u32 get_action(u32 kDown, u32 kHeld) {
	u32 held = kDown | kHeld;
	u32 action = 0;
	int i;
	for (i=0;controls[i].action;i++) {
		if ((controls[i].down_keys & kDown) != controls[i].down_keys) {
			continue;
		}
		if ((controls[i].held_keys & held) != controls[i].held_keys) {
			continue;
		}
		if (controls[i].forbidden_keys & held) {
			continue;
		}
		action |= controls[i].action;
	}
	return action;
}

circlePosition cpad_to_movement(circlePosition circle_pos) {
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
	circlePosition ret = {0,0};
	ret.dx = circle_pos.dx/20;
	ret.dy = -circle_pos.dy/20;
	return ret;
}

