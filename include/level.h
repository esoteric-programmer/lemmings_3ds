#ifndef LEVEL_H
#define LEVEL_H
#include <3ds.h>

struct Object {
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

struct LevelInfo {
	u8 rate;
	u8 lemmings;
	u8 to_rescue;
	u8 percentage_needed;
	u8 minutes;
	u8 skills[8];
	u16 x_pos;
	char name[33];
};

struct Palette {
	u32 vga[16];
	u32 vga_preview[16];
	u32 ega[16];
	u32 ega_preview[16];
};

struct Entrances {
	struct {
		s16 x;
		s16 y;
	} pos[4];
};

struct Level {
	struct LevelInfo info;
	u8 terrain[1584*160];
	struct Object* o[16];
	struct ObjectInstance obj[32];
	u8 object_map[1584/4*160/4];
	struct Palette palette;
	struct Entrances entrances;
	u8 rescued;
};


struct LevelState {
	u16 frames_left; // until time is up
	u8 paused;
	u8 cur_rate;
	u8 selected_skill;
	u8 entrances_open;
	struct {s16 x; s16 y;} cursor;
};
#endif
