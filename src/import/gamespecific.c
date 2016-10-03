#include "gamespecific.h"
#include "settings.h"
#define LEVEL(file, section, modified) ((file)<<3|(section)|((modified)?0x80:0x00))

extern const u8 position_of_classic_level[];
extern const u8 position_of_ohnomore_level[];
extern const u8 ascending_numbers_0_to_63[];
extern const char* msgs_orig[];
extern const char* msgs_ohno[];
extern const char* msgs_holi93[];
extern const char* msgs_holi94[];
extern const u32 main_palette[];
extern const u32 ingame_palette[];
extern const u32 highperf_palette[];
extern const u32 main_palette_xmas[];
extern const u32 ingame_palette_xmas[];
extern const u32 highperf_palette_xmas[];

const char* difficulties_classic[] = {"Fun", "Tricky", "Taxing", "Mayhem"};
const char* difficulties_ohnomore[] = {"Tame", "Crazy", "Wild", "Wicked", "Havoc"};
const char* difficulties_xmas91[] = {"Xmas91"};
const char* difficulties_xmas92[] = {"Xmas92"};
const char* difficulties_holi93[] = {"Flurry", "Blitz"};
const char* difficulties_holi94[] = {"Frost", "Hail", "Flurry", "Blitz"};
const char* difficulties_holi_demo[] = {"Demo"};

const u8 orig_song_order[] = {6, 7, 8, 10, 11, 4, 12, 13, 14, 15, 5, 16, 17, 18, 19, 20, 21};

const struct SpecialSongs special_songs[] = {{21,2}, {43,9}, {74,1}, {111,3} };

const struct GameSpecific import[LEMMING_GAMES] = {
		// original Lemmings DEMO
		{
			1, 4, 1, PATH_DATA_DEMO, "LEVEL",
			difficulties_classic,  ascending_numbers_0_to_63, msgs_orig, 4, // TODO: level positions?
			main_palette, ingame_palette, highperf_palette,
			17, orig_song_order, 0, 0
		},
		// original Lemmings
		{
			1, 4, 30, PATH_DATA_ORIGINAL, "LEVEL",
			difficulties_classic,  position_of_classic_level, msgs_orig, 4,
			main_palette, ingame_palette, highperf_palette,
			17, orig_song_order, 4, special_songs
		},
		// Oh No! More Lemmings DEMO
		{
			0, 5, 1, PATH_DATA_OHNODEMO, "DLVEL",
			difficulties_ohnomore, ascending_numbers_0_to_63, msgs_ohno, 5,
			main_palette, ingame_palette, highperf_palette,
			6, ascending_numbers_0_to_63+1, 0, 0
		},
		// Oh No! More Lemmings
		{
			0, 5, 20, PATH_DATA_OHNOMORE, "DLVEL",
			difficulties_ohnomore, position_of_ohnomore_level, msgs_ohno, 5,
			main_palette, ingame_palette, highperf_palette,
			6, ascending_numbers_0_to_63+1, 0, 0
		},
		// Xmas Lemmings 1991
		{
			0, 1,  4, PATH_DATA_XMAS91, "LEVEL",
			difficulties_xmas91, ascending_numbers_0_to_63, msgs_ohno, 4,
			main_palette_xmas, ingame_palette_xmas, highperf_palette_xmas,
			3, ascending_numbers_0_to_63+1, 0, 0
		},
		// Xmas Lemmings 1992
		{
			0, 1,  4, PATH_DATA_XMAS92, "LEVEL",
			difficulties_xmas92, ascending_numbers_0_to_63, msgs_ohno, 4,
			main_palette_xmas, ingame_palette_xmas, highperf_palette_xmas,
			3, ascending_numbers_0_to_63+1, 0, 0
		},
		// Holiday Lemmings 1993 DEMO
		{
			0, 1, 4, PATH_DATA_HOLI93DEMO, "LEVEL",
			difficulties_holi_demo, ascending_numbers_0_to_63, msgs_holi93, 4,
			main_palette_xmas, ingame_palette_xmas, highperf_palette_xmas,
			3, ascending_numbers_0_to_63+1, 0, 0
		},
		// Holiday Lemmings 1993
		{
			0, 2, 16, PATH_DATA_HOLIDAY93, "LEVEL",
			difficulties_holi93, ascending_numbers_0_to_63, msgs_holi93, 4,
			main_palette_xmas, ingame_palette_xmas, highperf_palette_xmas,
			3, ascending_numbers_0_to_63+1, 0, 0
		},
		// Holiday Lemmings 1994 DEMO
		{
			0, 1, 4, PATH_DATA_HOLI94DEMO, "LEVEL",
			difficulties_holi_demo, ascending_numbers_0_to_63, msgs_holi94, 4,
			main_palette_xmas, ingame_palette_xmas, highperf_palette_xmas,
			6, ascending_numbers_0_to_63+1, 0, 0
		},
		// Holiday Lemmings 1994
		{
			0, 4, 16, PATH_DATA_HOLIDAY94, "LEVEL",
			difficulties_holi94, ascending_numbers_0_to_63, msgs_holi94, 4,
			main_palette_xmas, ingame_palette_xmas, highperf_palette_xmas,
			6, ascending_numbers_0_to_63+1, 0, 0
		}
};

const unsigned char position_of_classic_level[] = {
		/* Fun */
		LEVEL(9,1,0),
		LEVEL(9,5,0),
		LEVEL(9,6,0),
		LEVEL(9,2,0),
		LEVEL(9,3,0),
		LEVEL(9,4,0),
		LEVEL(9,7,0),
		LEVEL(0,6,1),
		LEVEL(1,2,1),
		LEVEL(3,2,1),
		LEVEL(4,2,1),
		LEVEL(0,7,1),
		LEVEL(1,6,0),
		LEVEL(1,7,1),
		LEVEL(2,2,1),
		LEVEL(2,4,1),
		LEVEL(2,7,1),
		LEVEL(4,3,1),
		LEVEL(5,1,1),
		LEVEL(6,3,1),
		LEVEL(8,4,1),
		LEVEL(1,3,0),
		LEVEL(4,1,1),
		LEVEL(5,7,1),
		LEVEL(6,0,1),
		LEVEL(7,1,1),
		LEVEL(4,6,1),
		LEVEL(6,1,1),
		LEVEL(6,5,1),
		LEVEL(8,2,1),
		
		/* Tricky */
		LEVEL(0,0,0),
		LEVEL(1,6,1),
		LEVEL(2,1,1),
		LEVEL(3,0,1),
		LEVEL(3,1,1),
		LEVEL(3,3,1),
		LEVEL(3,4,1),
		LEVEL(4,7,1),
		LEVEL(6,2,1),
		LEVEL(7,3,1),
		LEVEL(7,7,1),
		LEVEL(8,0,1),
		LEVEL(8,3,1),
		LEVEL(0,2,0),
		LEVEL(9,1,1),
		LEVEL(9,3,1),
		LEVEL(9,4,1),
		LEVEL(9,5,1),
		LEVEL(9,7,1),
		LEVEL(0,3,0),
		LEVEL(0,5,0),
		LEVEL(0,6,0),
		LEVEL(0,7,0),
		LEVEL(1,0,0),
		LEVEL(1,1,0),
		LEVEL(1,2,0),
		LEVEL(1,4,0),
		LEVEL(1,5,0),
		LEVEL(2,0,0),
		LEVEL(1,7,0),

		/* Taxing */
		LEVEL(2,2,0),
		LEVEL(2,3,0),
		LEVEL(2,4,0),
		LEVEL(2,5,0),
		LEVEL(2,6,0),
		LEVEL(2,7,0),
		LEVEL(3,0,0),
		LEVEL(3,1,0),
		LEVEL(3,2,0),
		LEVEL(3,3,0),
		LEVEL(3,4,0),
		LEVEL(3,5,0),
		LEVEL(3,6,0),
		LEVEL(3,7,0),
		LEVEL(0,1,0),
		LEVEL(4,0,0),
		LEVEL(4,1,0),
		LEVEL(4,2,0),
		LEVEL(4,3,0),
		LEVEL(4,4,0),
		LEVEL(4,5,0),
		LEVEL(4,6,0),
		LEVEL(4,7,0),
		LEVEL(5,0,0),
		LEVEL(5,1,0),
		LEVEL(5,2,0),
		LEVEL(5,3,0),
		LEVEL(5,4,0),
		LEVEL(2,1,0),
		LEVEL(6,7,0),

		/* Mayhem */
		LEVEL(5,5,0),
		LEVEL(5,6,0),
		LEVEL(5,7,0),
		LEVEL(6,0,0),
		LEVEL(6,1,0),
		LEVEL(6,2,0),
		LEVEL(6,3,0),
		LEVEL(6,4,0),
		LEVEL(6,5,0),
		LEVEL(6,6,0),
		LEVEL(6,7,1),
		LEVEL(7,0,0),
		LEVEL(7,1,0),
		LEVEL(7,2,0),
		LEVEL(7,3,0),
		LEVEL(7,4,0),
		LEVEL(7,5,0),
		LEVEL(7,6,0),
		LEVEL(7,7,0),
		LEVEL(9,2,1),
		LEVEL(8,0,0),
		LEVEL(0,4,0),
		LEVEL(8,1,0),
		LEVEL(8,2,0),
		LEVEL(8,3,0),
		LEVEL(8,4,0),
		LEVEL(8,5,0),
		LEVEL(8,6,0),
		LEVEL(8,7,0),
		LEVEL(9,0,0)	
};

const unsigned char position_of_ohnomore_level[] = {
		/* Tame */
		80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
		90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
		
		/* Crazy */
		 1,  8, 12, 16, 17, 24, 25, 29, 41, 44,
		47, 55,  4, 13, 14, 22, 28, 31, 35, 52,

		/* Wild */
		74, 56, 57, 58, 59, 61, 70,  7, 18, 21,
		27, 30, 32, 34, 36, 40, 51, 53, 54, 39,

		/* Wicked */
		78, 38, 72, 73,  5, 76, 49,  6, 10, 71,
		33, 45, 48, 50, 63, 65, 79, 75, 64, 62,
		
		/* Havoc */
		60, 43, 26, 23, 20, 19, 42, 15,  9,  3,
		 2,  0, 37, 69, 11, 66, 67, 46, 68, 77	
};

const unsigned char ascending_numbers_0_to_63[] = {
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
		16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
};

const char* msgs_orig[9] = {
		"Superb! You rescued \n"
		"  every lemming on  \n"
		"that level. Can you \n"
		"  do it again....?  \n",
		"You totally stormed \n"
		" that level! Let's  \n"
		"see if you can storm\n"
		"    the next...     \n",
		" That level seemed  \n"
		" no problem to you  \n"
		"  on that attempt.  \n"
		" Onto the next....  \n",
		"RIGHT ON. You can't \n"
		"  get much closer   \n"
		"  than that. Let's  \n"
		"  try the next....  \n",
		" OH NO, So near and \n"
		"yet so far (teehee) \n"
		"     Maybe this     \n"
		"     time.....      \n",
		"   You got pretty   \n"
		"  close that time.  \n"
		" Now try again for  \n"
		" that few % extra.  \n",
		"   A little more    \n"
		"  practice on this  \n"
		"level is definitely \n"
		"    recommended.    \n",
		"   Better rethink   \n"
		"   your strategy    \n"
		"   before you try   \n"
		" this level again!  \n",
		"    ROCK BOTTOM!    \n"
		"  I hope for your   \n"
		"   sake that you    \n"
		" nuked that level.  \n"
};

const char* msgs_ohno[9] = {
		"   WOW! You saved   \n"
		"   every Lemming.   \n"
		" TOTALLY EXCELLENT! \n"
		"                    \n",
		"What a fine display \n"
		"of Lemming control. \n"
		"Take a bow then car-\n"
		"ry on with the game.\n",
		" More than enough.  \n"
		"    You have the    \n"
		"makings of a master \n"
		"  Lemmings player.  \n",
		"    Just made it    \n"
		"   by the skin of   \n"
		"    your teeth.     \n"
		" Time to progress.. \n",
		"  Shame, You were   \n"
		"  short by a tiny   \n"
		" amount. Go for it  \n"
		"     this time.     \n",
		"   Getting close.   \n"
		"   You are either   \n"
		"  pretty good, or   \n"
		"   simply lucky.    \n",
		"   We are not too   \n"
		"   impressed with   \n"
		"  your attempt at   \n"
		"    that level!     \n",
		"  Yes, well, err,   \n"
		"  erm, maybe that   \n"
		"   is NOT the way   \n"
		" to do this level.  \n",
		" Oh dear, not even  \n"
		"  one poor Lemming  \n"
		"saved. Try a little \n"
		" harder next time.  \n"
};


const char* msgs_holi93[9] = {
		" Excellent! You've  \n"
		"  saved all of the  \n" // original message is too long -> shortened
		"  little rodents!   \n"
		"  Now do it again!  \n",
		"    An exemplary    \n"
		" instance of rodent \n"
		" control. Now, onto \n"
		" the next challenge.\n",
		" Well done.  You've \n"
		"    made it with    \n"
		"  plenty to spare.  \n"
		"Now onto the next...\n",
		"  That was close,   \n"
		"  but you made it!  \n"
		"    Onwards and     \n"
		"     upwards...     \n",
		"  Just a tiny bit   \n"
		"   more effort on   \n"
		" your part will get \n"
		" the lemmings home! \n",
		"  Not bad, but you  \n"
		"  can certainly do  \n"
		"   a bit better!    \n"
		"                    \n",
		"     Try a bit      \n"
		"     harder...      \n"
		"  the lemmings are  \n"
		" depending on you!  \n",
		"     Umm, maybe     \n"
		"    you'd better    \n"
		"    rethink your    \n"
		"  strategy a bit!   \n",
		" At this rate, the  \n"
		"  lemmings are not  \n"
		"  going to have a   \n"
		"very happy holiday! \n"
};


const char* msgs_holi94[9] = {
		" Excellent! You've  \n"
		"managed to save them\n"
		" all! Can you do as \n"
		"  well next time?   \n",
		"  Very impressive.  \n"
		"You're well on your \n"
		" way to becoming a  \n"
		"  Lemmings Master!  \n",
		" Well done.  You've \n"
		"    made it with    \n"
		"  plenty to spare.  \n"
		"Now onto the next...\n",
		"   Whew! That was   \n"
		"   close, but you   \n"
		" made it! On to the \n"
		"  next challenge!   \n",
		"  Just a tiny bit   \n"
		"more effort will get\n"
		" the lemmings home  \n"
		" for the holidays!  \n",
		"  Not bad, but you  \n"
		"  can certainly do  \n"
		"   a bit better!    \n"
		"                    \n",
		"     Try a bit      \n"
		"     harder...      \n"
		"  the lemmings are  \n"
		" depending on you!  \n",
		"     Umm, maybe     \n"
		"    you'd better    \n"
		"    rethink your    \n"
		"  strategy a bit!   \n",
		"Uh-oh! Not a single \n"
		"   lemming saved!   \n"
		"  Try harder, the   \n"
		" lemmings need you! \n"
};

const u32 main_palette[] = {
		0x000000,
		0x204080,
		0x203060,
		0x100030,
		0x7C0820,
		0x902C40,
		0xA45868,
		0xBC8C98,
		0x005000,
		0x106000,
		0x207000,
		0x408000,
		0xD0D0D0,
		0x00B0B0,
		0xB05040,
		0x9080E0
};

const u32 ingame_palette[] = {
		0x000000,
 		0xE24040,
		0x00B200,
		0xD2D2F2,
		0x00B2B2,
		0x2020F2,
		0x818181,
		0
};

const u32 highperf_palette[] = {
		0x000000,
		0xE04040,
		0x00B000,
		0xD0D0F0,
		0x00B0B0,
		0x2020F0,
		0x808080,
		0, // TO BE OVERWRITTEN BY LEVEL-PALETTE-VALUE 8
		0x00A800,
		0x50FC50,
		0x505450,
		0x0000A8,
		0x0054A8,
		0xA8A800,
		0xF854F8,
		0xA800A8
};

const u32 main_palette_xmas[] = {
		0x000000,
		0x204080,
		0x203060,
		0x100030,
		0x7C0820,
		0x902C40,
		0xA45868,
		0xBC8C98,
		0x005000,
		0x106000,
		0x207000,
		0x408000,
		0xD0D0D0,
		0x00B0B0,
		0x1010C8, // xmas: red
		0x9080E0
};

const u32 ingame_palette_xmas[] = {
		0x000000,
 		0x2020F2,
		0x00B200,
		0xD2D2F2,
		0x00B2B2,
		0xE24040,
		0x818181,
		0
};

const u32 highperf_palette_xmas[] = {
		0x000000,
		0x2020D0,
		0x00B000,
		0xD0D0F0,
		0x00F0F0,
		0xE04040,
		0x808080, // TODO
		0, // TO BE OVERWRITTEN BY LEVEL-PALETTE-VALUE 8
		0x00A800, // TODO
		0x50F050,
		0x505450, // TODO
		0x0000A8, // TODO
		0x303060,
		0x0050A0,
		0xF854F8, // TODO
		0x404090
};
