#ifndef MAIN_DATA_H
#define MAIN_DATA_H
#include <3ds.h>

struct Image {
	u16 width;
	u16 height;
	u8 data[0];
};

struct MainInGameData {
	struct Image* lemmings_anim[337];
	struct Image* masks[23];
	u32 high_perf_palette[16];
	u8 high_perf_toolbar[320*40];
	u8 skill_numbers[20*8*8];
	u8 high_perf_font[38*8*16];
	u32 level_base_palette[7];
};

struct MainMenuData {
	u32 palette[16];
	struct Image* static_pictures[16];
	u8 blinking_eyes[7*8*32*12];
	u8 scroller[2*16*48*16];
	u8 menu_font[94*16*16];
};
#endif
