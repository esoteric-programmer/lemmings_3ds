#include "gamespecific_2p.h"

extern const u32 ingame_palette[];

// 17..20 need to be tested
const u8 orig_swap_exit[] =
		{0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0};

// needs to be tested
const u8 ohno_swap_exit[] =
		{1, 0, 0, 0, 0, 0, 1, 1, 1, 1};

const struct GameSpecific2P import_2p[2] = {
		// original Lemmings
		{
			"2p/orig", PATH_DATA_ORIGINAL,
			ingame_palette,
			20, orig_swap_exit
		},
		// Oh No! More Lemmings
		{
			"2p/ohno", PATH_DATA_OHNOMORE,
			ingame_palette,
			10, ohno_swap_exit
		}
};
