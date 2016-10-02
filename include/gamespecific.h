#ifndef GAMESPECIFIC_H
#define GAMESPECIFIC_H
#include <3ds.h>

#define LEMMING_GAMES        10 // number of currently supported games...

// game identifiers
#define LEMMINGS_DEMO        0
#define ORIGINAL_LEMMINGS    1
#define OH_NO_DEMO           2
#define OH_NO_MORE_LEMMINGS  3
#define HOLIDAY_LEMMINGS_91  4
#define HOLIDAY_LEMMINGS_92  5
#define HOLIDAY_93_DEMO      6
#define HOLIDAY_LEMMINGS_93  7
#define HOLIDAY_94_DEMO      8
#define HOLIDAY_LEMMINGS_94  9

#define PATH_DATA_DEMO       "orig_demo"
#define PATH_DATA_ORIGINAL   "orig"
#define PATH_DATA_OHNODEMO   "ohno_demo"
#define PATH_DATA_OHNOMORE   "ohno"
#define PATH_DATA_XMAS91     "xmas91"
#define PATH_DATA_XMAS92     "xmas92"
#define PATH_DATA_HOLI93DEMO "holi93_demo"
#define PATH_DATA_HOLIDAY93  "holi93"
#define PATH_DATA_HOLI94DEMO "holi94_demo"
#define PATH_DATA_HOLIDAY94  "holi94"

struct GameSpecific {
	const u8 ABBA_order;
	const u8 num_of_difficulties;
	const u8 num_of_level_per_difficulty;
	const char* path;
	const char* level_dat_prefix;
	const char* const* difficulties;
	const unsigned char* level_position;
	const char* const* messages;
	const unsigned char num_of_difficulty_graphics;
	const u32* main_palette;
	const u32* ingame_palette;
	const u32* highperf_palette;
};
extern const struct GameSpecific import[LEMMING_GAMES];

#endif
