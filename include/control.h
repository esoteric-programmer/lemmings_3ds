#ifndef CONTROL_H
#define CONTROL_H
#include <3ds.h>

#define ACTION_MOVE_CURSOR_RIGHT     (1<< 0)
#define ACTION_MOVE_CURSOR_LEFT      (1<< 1)
#define ACTION_MOVE_CURSOR_UP        (1<< 2)
#define ACTION_MOVE_CURSOR_DOWN      (1<< 3)
#define ACTION_CURSOR_CLICK          (1<< 4)
#define ACTION_CURSOR_HOLD           (1<< 5)
#define ACTION_PAUSE                 (1<< 6)
#define ACTION_NEXT_SKILL            (1<< 7)
#define ACTION_PREV_SKILL            (1<< 8)
#define ACTION_INC_RATE              (1<< 9)
#define ACTION_DEC_RATE              (1<<10)
#define ACTION_SPEED_UP              (1<<11)
#define ACTION_SCROLL_RIGHT          (1<<12)
#define ACTION_SCROLL_LEFT           (1<<13)
#define ACTION_NONPRIORIZED_LEMMING  (1<<14)
#define ACTION_NUKE                  (1<<15)
#define ACTION_NUKE_IMMEDIATELY      (1<<16)
#define ACTION_QUIT                  (1<<17)
#define ACTION_SELECT_SKILL_CIMBER   (1<<18)
#define ACTION_SELECT_SKILL_FLOATER  (1<<19)
#define ACTION_SELECT_SKILL_BOMBER   (1<<20)
#define ACTION_SELECT_SKILL_BLOCKER  (1<<21)
#define ACTION_SELECT_SKILL_BUILDER  (1<<22)
#define ACTION_SELECT_SKILL_BASHER   (1<<23)
#define ACTION_SELECT_SKILL_MINER    (1<<24)
#define ACTION_SELECT_SKILL_DIGGER   (1<<25)
#define ACTION_MOVE_CURSOR_CPAD      (1<<26) // move according to CPAD position
#define ACTION_MOVE_CURSOR_CPAD_LR   (1<<27) // mirror LR (can be combined with mirror UD)
#define ACTION_MOVE_CURSOR_CPAD_UD   (1<<28) // mirror UD (can be combined with mirror LD)
#define ACTION_SCROLL_CPAD           (1<<29) // move according to CPAD position
#define ACTION_SCROLL_CPAD_LR        (1<<30) // mirror LR (can be combined with mirror UD)
#define ACTION_QUIT_GAME             (1<<31)

extern struct Controls {
	u32 down_keys;
	u32 held_keys;
	u32 forbidden_keys;
	u32 action;
} controls[];

u32 get_action(u32 kDown, u32 kHeld);
circlePosition cpad_to_movement(circlePosition circle_pos);
#endif
