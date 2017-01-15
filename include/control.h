#ifndef CONTROL_H
#define CONTROL_H
#include <3ds.h>
#include "level.h"

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
#define ACTION_MOVE_CURSOR_PARAM     (1<<26) // mirror UD (can be combined with mirror LD)
#define ACTION_SCROLL_PARAM          (1<<27) // mirror LR (can be combined with mirror UD)
#define ACTION_QUIT_GAME             (1<<28)
#define ACTION_STEP_FRAME            (1<<29)

#define BOTTOM_SCREEN_Y_OFFSET 32

#define MAX_ACTION_QUEUE_SIZE 3
struct ActionQueue {
	enum Action {
		ACTIONQUEUE_NOP, // no action
		ACTIONQUEUE_NUKE, // 0 = nuke; 1 = exit immediately
		ACTIONQUEUE_ASSIGN_SKILL, // to Lemming with id stored in param (lem1) or param2 (lem2); skill stored in param3
		ACTIONQUEUE_TOGGLE_PAUSE,
		ACTIONQUEUE_FRAME_FORWARD, // (s8)param: number of frames (handling of negative values not implemented yet)
		ACTIONQUEUE_CHANGE_RATE // (s8)param = changing
	} action;
	u8 param;
	u8 param2;
	u8 param3;
};

struct InputState {
	// number of (input-)frames since user pressed a button
	// to change the release rate of lemmings
	u8 change_rate_hold;
	// time elapsed since user pressed the nuke button the last time.
	// 0 if this is long ago.
	u8 time_since_nuke_pressed;
	u8 speed_up;
	u8 nonprio_lem;
	u8 skill;
	struct {s16 x; s16 y;} cursor;
	u16 x_pos;
	u8 num_actions; // number of actions in queue
	struct ActionQueue action_queue[MAX_ACTION_QUEUE_SIZE];
};

u64 get_action(u32 kDown, u32 kHeld, circlePosition cpad, circlePosition* params);
void init_io_state(struct InputState* io_state, u16 x_pos);
int add_action(struct InputState* io_state, enum Action action, u8 param, u8 param2, u8 param3);
int read_io(struct Level* level, struct InputState* io_state, u8 player);
int process_action_queue(
		// actions to perform (invalid actions will be replaced by ACTIONQUEUE_NOP)
		struct ActionQueue* action_queue,
		u8 num_actions,
		struct Level* level,
		u8 player_id,
		// if multiplayer is set, some actions will be disabled
		u8 multiplayer);
#endif
