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

#define orig_victory_message \
		"  Congratulations!  \n"\
		"Not many people will\n"\
		"complete the Mayhem \n"\
		"  levels, you are   \n"\
		" definitely one of  \n"\
		"     the elite.     \n"

#define ohno_victory_message \
		"  Congratulations!  \n"\
		"  You are truly an  \n"\
		" Excellent Lemmings \n"\
		"       player       \n"

#define demo_victory_message \
		"  CONGRATULATIONS!  \n"\
		" You have completed \n"\
		"all the levels, now \n"\
		" back to the start. \n"

#define holi94_victory_message \
		"  Congratulations!  \n"\
		" You have completed \n"\
		"   all of the new   \n"\
		"1994 Holiday Levels.\n"\
		"   Now, enjoy the   \n"\
		" bonus levels from  \n"\
		"the 1993 edition of \n"\
		" Holiday Lemmings!  \n"

const struct SpecialMessages orig_messages =
		{119, 6, orig_victory_message};

const struct SpecialMessages ohno_messages =
		{99, 4, ohno_victory_message};

const struct SpecialMessages demo_messages =
		{3, 4, demo_victory_message};

const struct SpecialMessages ohno_demo_messages =
		{4, 4, demo_victory_message};

const struct SpecialMessages holi93_messages =
		{31, 4, demo_victory_message};

const struct SpecialMessages holi94_messages[] =
		{
			{31, 8, holi94_victory_message},
			{63, 4, demo_victory_message},
		};

const u8 orig_song_order[] =
		{6, 7, 8, 10, 11, 4, 12, 13, 14, 15, 5, 16, 17, 18, 19, 20, 21};

const struct SpecialSongs special_songs[] =
		{
			{21,2},
			{43,9},
			{74,1},
			{111,3}
		};

const struct GameSpecific import[LEMMING_GAMES] = {
		// original Lemmings DEMO
		{
			1, 24, 4, 1, PATH_DATA_DEMO, "orig", "LEVEL",
			difficulties_classic,  ascending_numbers_0_to_63, msgs_orig, 4,
			main_palette, ingame_palette, highperf_palette,
			17, orig_song_order, 0, 0,
			1, &demo_messages
		},
		// original Lemmings
		{
			1, 24, 4, 30, PATH_DATA_ORIGINAL, "orig", "LEVEL",
			difficulties_classic,  position_of_classic_level, msgs_orig, 4,
			main_palette, ingame_palette, highperf_palette,
			17, orig_song_order, 4, special_songs,
			1, &orig_messages
		},
		// Oh No! More Lemmings DEMO
		{
			0, 25, 5, 1, PATH_DATA_OHNODEMO, "ohno", "DLVEL",
			difficulties_ohnomore, ascending_numbers_0_to_63, msgs_ohno, 5,
			main_palette, ingame_palette, highperf_palette,
			6, ascending_numbers_0_to_63+1, 0, 0,
			1, &ohno_demo_messages
		},
		// Oh No! More Lemmings
		{
			0, 25, 5, 20, PATH_DATA_OHNOMORE, "ohno", "DLVEL",
			difficulties_ohnomore, position_of_ohnomore_level, msgs_ohno, 5,
			main_palette, ingame_palette, highperf_palette,
			6, ascending_numbers_0_to_63+1, 0, 0,
			1, &ohno_messages
		},
		// Xmas Lemmings 1991
		{
			0, 25, 1,  4, PATH_DATA_XMAS91, "xmas", "LEVEL",
			difficulties_xmas91, ascending_numbers_0_to_63, msgs_ohno, 4,
			main_palette_xmas, ingame_palette_xmas, highperf_palette_xmas,
			3, ascending_numbers_0_to_63+1, 0, 0,
			1, &demo_messages
		},
		// Xmas Lemmings 1992
		{
			0, 25, 1,  4, PATH_DATA_XMAS92, "xmas", "LEVEL",
			difficulties_xmas92, ascending_numbers_0_to_63, msgs_ohno, 4,
			main_palette_xmas, ingame_palette_xmas, highperf_palette_xmas,
			3, ascending_numbers_0_to_63+1, 0, 0,
			1, &demo_messages
		},
		// Holiday Lemmings 1993 DEMO
		{
			0, 25, 1, 4, PATH_DATA_HOLI93DEMO, "xmas", "LEVEL",
			difficulties_holi_demo, ascending_numbers_0_to_63, msgs_holi93, 4,
			main_palette_xmas, ingame_palette_xmas, highperf_palette_xmas,
			3, ascending_numbers_0_to_63+1, 0, 0,
			1, &demo_messages
		},
		// Holiday Lemmings 1993
		{
			0, 25, 2, 16, PATH_DATA_HOLIDAY93, "xmas", "LEVEL",
			difficulties_holi93, ascending_numbers_0_to_63, msgs_holi93, 4,
			main_palette_xmas, ingame_palette_xmas, highperf_palette_xmas,
			3, ascending_numbers_0_to_63+1, 0, 0,
			1, &holi93_messages
		},
		// Holiday Lemmings 1994 DEMO
		{
			0, 25, 1, 4, PATH_DATA_HOLI94DEMO, "ohno", "LEVEL",
			difficulties_holi_demo, ascending_numbers_0_to_63, msgs_holi94, 4,
			main_palette_xmas, ingame_palette_xmas, highperf_palette_xmas,
			6, ascending_numbers_0_to_63+1, 0, 0,
			1, &demo_messages
		},
		// Holiday Lemmings 1994
		{
			0, 25, 4, 16, PATH_DATA_HOLIDAY94, "ohno", "LEVEL",
			difficulties_holi94, ascending_numbers_0_to_63, msgs_holi94, 4,
			main_palette_xmas, ingame_palette_xmas, highperf_palette_xmas,
			6, ascending_numbers_0_to_63+1, 0, 0,
			2, holi94_messages
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

#ifdef ABGR
const u32 main_palette[] = {
		0x000000FF,
		0x804020FF,
		0x603020FF,
		0x300010FF,
		0x20087CFF,
		0x402C90FF,
		0x6858A4FF,
		0x988CBCFF,
		0x005000FF,
		0x006010FF,
		0x007020FF,
		0x008040FF,
		0xD0D0D0FF,
		0xB0B000FF,
		0x4050B0FF,
		0xE08090FF
};

const u32 ingame_palette[] = {
		0x000000FF,
 		0x4040E2FF,
		0x00B200FF,
		0xF2D2D2FF,
		0xB2B200FF,
		0xF22020FF,
		0x818181FF,
		0
};

const u32 highperf_palette[] = {
		0x000000FF,
		0x4040E0FF,
		0x00B000FF,
		0xF0D0D0FF,
		0xB0B000FF,
		0xF02020FF,
		0x808080FF,
		0, // TO BE OVERWRITTEN BY LEVEL-PALETTE-VALUE 8
		0x00A800FF,
		0x50FC50FF,
		0x505450FF,
		0xA80000FF,
		0xA85400FF,
		0x00A8A8FF,
		0xF854F8FF,
		0xA800A8FF
};

const u32 main_palette_xmas[] = {
		0x000000FF,
		0x804020FF,
		0x603020FF,
		0x300010FF,
		0x20087CFF,
		0x402C90FF,
		0x6858A4FF,
		0x988CBCFF,
		0x005000FF,
		0x006010FF,
		0x007020FF,
		0x008040FF,
		0xD0D0D0FF,
		0xB0B000FF,
		0xC81010FF, // xmas: red
		0xE08090FF
};

const u32 ingame_palette_xmas[] = {
		0x000000FF,
 		0xF22020FF,
		0x00B200FF,
		0xF2D2D2FF,
		0xB2B200FF,
		0x4040E2FF,
		0x818181FF,
		0
};

const u32 highperf_palette_xmas[] = {
		0x000000FF,
		0xD02020FF,
		0x00B000FF,
		0xF0D0D0FF,
		0xF0F000FF,
		0x4040E0FF,
		0x808080FF, // TODO
		0, // TO BE OVERWRITTEN BY LEVEL-PALETTE-VALUE 8
		0x00A800FF, // TODO
		0x50F050FF,
		0x505450FF, // TODO
		0xA80000FF, // TODO
		0x603030FF,
		0xA05000FF,
		0xF854F8FF, // TODO
		0x904040FF
};
#else
const u32 main_palette[] = {
		0xFF000000,
		0xFF204080,
		0xFF203060,
		0xFF100030,
		0xFF7C0820,
		0xFF902C40,
		0xFFA45868,
		0xFFBC8C98,
		0xFF005000,
		0xFF106000,
		0xFF207000,
		0xFF408000,
		0xFFD0D0D0,
		0xFF00B0B0,
		0xFFB05040,
		0xFF9080E0
};

const u32 ingame_palette[] = {
		0xFF000000,
		0xFFE24040,
		0xFF00B200,
		0xFFD2D2F2,
		0xFF00B2B2,
		0xFF2020F2,
		0xFF818181,
		0
};

const u32 highperf_palette[] = {
		0xFF000000,
		0xFFE04040,
		0xFF00B000,
		0xFFD0D0F0,
		0xFF00B0B0,
		0xFF2020F0,
		0xFF808080,
		0, // TO BE OVERWRITTEN BY LEVELPALETTEVALUE 8
		0xFF00A800,
		0xFF50FC50,
		0xFF505450,
		0xFF0000A8,
		0xFF0054A8,
		0xFFA8A800,
		0xFFF854F8,
		0xFFA800A8
};

const u32 main_palette_xmas[] = {
		0xFF000000,
		0xFF204080,
		0xFF203060,
		0xFF100030,
		0xFF7C0820,
		0xFF902C40,
		0xFFA45868,
		0xFFBC8C98,
		0xFF005000,
		0xFF106000,
		0xFF207000,
		0xFF408000,
		0xFFD0D0D0,
		0xFF00B0B0,
		0xFF1010C8, // xmas: red
		0xFF9080E0
};

const u32 ingame_palette_xmas[] = {
		0xFF000000,
		0xFF2020F2,
		0xFF00B200,
		0xFFD2D2F2,
		0xFF00B2B2,
		0xFFE24040,
		0xFF818181,
		0
};

const u32 highperf_palette_xmas[] = {
		0xFF000000,
		0xFF2020D0,
		0xFF00B000,
		0xFFD0D0F0,
		0xFF00F0F0,
		0xFFE04040,
		0xFF808080, // TODO
		0, // TO BE OVERWRITTEN BY LEVELPALETTEVALUE 8
		0xFF00A800, // TODO
		0xFF50F050,
		0xFF505450, // TODO
		0xFF0000A8, // TODO
		0xFF303060,
		0xFF0050A0,
		0xFFF854F8, // TODO
		0xFF404090
};
#endif
