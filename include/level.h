#ifndef LEVEL_H
#define LEVEL_H
#include <3ds.h>
#include "settings.h"
#include "lemming_data.h"

// object effects
#define OBJECT_EXIT 1
#define OBJECT_FORCE_LEFT 2
#define OBJECT_FORCE_RIGHT 3
#define OBJECT_TRAP 4
#define OBJECT_WATER 5
#define OBJECT_FIRE 6
#define OBJECT_ONEWAY_LEFT 7
#define OBJECT_ONEWAY_RIGHT 8
#define OBJECT_STEEL 9
#define OBJECT_BLOCKER 10

struct ObjectType {
	u16 flags;
	u16 width;
	u16 height;
	u8 start_frame; // used?
	u8 end_frame; // or: frames
	s16 trigger_x;
	s16 trigger_y;
	u8 trigger_width;
	u8 trigger_height;
	u8 trigger; // effect
	u8 sound;
	u8 preview_frame;
	u8 data[0];
};

#define OBJECT_USED 1 // is object slot used / object present?
#define OBJECT_DONT_OVERWRITE 2 // dont overwrite terrain, but overwrite other objects?
#define OBJECT_REQUIRE_TERRAIN 4 // dont accept other objects?
#define OBJECT_UPSIDE_DOWN 8 // draw object upside down (TODO: flip trigger area as well?)
struct ObjectInstance {
	s16 x;
	s16 y;
	u8 type;
	u8 modifier;
	u8 current_frame; // STATE;
};

struct LevelPlayer {
	u8 max_lemmings;
	u8 skills[8];
	u16 x_pos;
	u8 nuking;
	u8 timer_assign;
	u8 next_lemming_id;
	// player one's lemmings and player two's lemmings that used the exit
	// of that player this LevelPlayer struct corresponds to.
	u8 rescued[2];
	u16 request_common_nuke; // multiplayer only. set when player wants to nuke. count down each frame, so the other player must request nuking in the same time slot
	u8 ready_to_start; // multiplayer only. set when player finished inspection of the level. to start the game, all players must have set this value (if level inspection is enabled).
	struct Lemming lemmings[MAX_NUM_OF_LEMMINGS];
};

struct Level {
	u8 terrain[1584*160];
	struct ObjectType* object_types[16];
	struct ObjectInstance object_instances[32];
	u8 object_map[1584/4*160/4];
	u32 palette[16];
	struct {s16 x; s16 y;} entrances[4];
	u8 rate;
	u8 percentage_needed;
	u8 speed_up;
	u8 num_players;

	u8 fade_in;
	u8 fade_out;
	u8 opening_counter; // count frames until entrances open
	u8 entrances_open;
	u16 frames_left; // until time is up

	u8 paused;
	u8 inspect;
	u8 frame_step_forward;

	u8 next_lemming_countdown;
	// which player got the last lemming?
	// in multiplayer mode, the next lemming should belong to the other player
	u8 lemming_last_player;
	u8 cur_rate;

	struct LevelPlayer player[2];

	char name[33];
};
#endif
