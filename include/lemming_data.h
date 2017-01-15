#ifndef LEMMING_DATA_H
#define LEMMING_DATA_H
#include <3ds.h>

#define LEMACTION_WALK 0
#define LEMACTION_SPLATTER 1 // after falling down from too high
#define LEMACTION_EXPLODE 2 // fire ball and explosion particles
#define LEMACTION_FALL 3
#define LEMACTION_JUMP 4
#define LEMACTION_DIG 5
#define LEMACTION_CLIMB 6
#define LEMACTION_HOIST 7 // end of climbing
#define LEMACTION_BUILD 8
#define LEMACTION_BLOCK 9
#define LEMACTION_BASH 10
#define LEMACTION_FLOAT 11
#define LEMACTION_MINE 12
#define LEMACTION_DROWN 13 // in water
#define LEMACTION_EXIT 14
#define LEMACTION_FRY 15 // killed by flameblower etc.
#define LEMACTION_OHNO 16
#define LEMACTION_SHRUG 17 // builder finished buildung

#define LEMABILITY_CLIMB 1
#define LEMABILITY_FLOAT 2

#define LEM_MIN_Y -5 // HEAD_MIN_Y
#define LEM_MAX_Y 163 // LEMMING_MAX_Y
#define LEM_MAX_FALLING 60 // MAX_FALLDISTANCECOUNT

struct Lemming {
	u8 removed;
	u8 current_action;
	u8 timer; // start with 79 (frame based)
	s16 x;
	s16 y;
	s8 x_draw_offset;
	s8 y_draw_offset;
	u8 draw_action; // used to copy the shrugging-bug; draw current_action when bug is disabled
	u8 frame_offset;
	u16 fall_distance;
	u8 look_right;
	u8 abilities; // mask: LEMABILITY_CLIMB; LEMABILITY_FLOAT
	u8 float_index;
	u8 bricks_left;
	u8 blocking;
	u8 start_digging;
	u8 object_below;
	u8 object_in_front;
	u8 saved_object_map[9];
	u8 exit_counts_for; // player id the exiting lemming counts for
	u8 player; // owner of this lemming
};
#endif
