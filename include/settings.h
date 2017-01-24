#ifndef SETTINGS_H
#define SETTINGS_H
#include <3ds.h>

#define NO_SF2D

#define LEMMINGS_DIR "/lemmings" // if it does not exist, "." will be used instead
extern const char* PATH_ROOT;

#define MAX_NUM_OF_LEMMINGS       80
#define FPS                       17 // dosframes per second (for time counter in level)
#define SCREEN_WIDTH             320
#define MS_PER_FRAME              71 // duration of frame in milliseconds
#define MS_PER_FRAME_SUPERLEM     28 // frame duration in "introducing superlemming"
#define INPUT_SAMPLING_MILLIS     25 // affects cursor speed and so on...
#define MS_PER_FRAME_SPEED_UP     20
#define FADE_IN_DOSFRAMES         18
#define FADE_OUT_DOSFRAMES        32
#define AMIGA_BACKGROUND  0x000030FF

#define COMMON_NUKE_FRAME_INTERVAL (FPS * 8 - 1) // player must respond to nuke request within 15 seconds

#define AUDIO_ORDER_PREFER_CUSTOM  0
#define AUDIO_ORDER_PREFER_ADLIB   1
#define AUDIO_ORDER_ONLY_CUSTOM    2
#define AUDIO_ORDER_ONLY_ADLIB     3

#define TIMEOUT_2P_NEVER           0
#define TIMEOUT_2P_INACTIVITY      1
#define TIMEOUT_2P_COUNTDOWN       2

extern struct Settings {
	u8 glitch_nuke;
	u8 glitch_entrance_pausing;
	u8 glitch_mining_right_oneway;
	u8 glitch_shrugger;
	u8 glitch_mayhem12; // not implemented yet
	u8 glitch_direct_drop;
	u8 speedup_millis_per_frame;
	u8 music_volume; // 0 = off; 100 = max
	u8 sfx_volume; // 0 = off; 100 = max
	u8 audio_order; // 0 = prefer custom sound; 1 = prefer ADLIB; 2 = only custom; 3 = only ADLIB
	u8 dlbclick_nuke; // not implemented yet
	u8 dblclick_exit; // not implemented yet
	u8 skip_unavailable_skills; // not implemented yet
	u8 zoom_mode_active; // not implemented yet
	u8 amiga_background; // 0 = no (black); 1 = yes (dark blue)
	u8 two_player_always_equal; // 0 = yes, always start with 40 lemmings; 1 = no, start wth 40 + rescued lemmings
	u8 two_player_timeout;
	u8 two_player_inspect_level;
	u8 reserved_1; // maybe 1p recording settings (for playback)
	u8 reserved_2; // maybe 2p recording settings (for playback)
	struct KeyBindings {
		u32 modifier;
		u32 click;
		u32 inc_rate;
		u32 dec_rate;
		u32 next_skill;
		u32 prev_skill;
		u32 pause;
		u32 nuke;
		u32 exit;
		u32 speed_up;
		u32 non_prio;
		u32 step_one_frame;
		u32 step_backwards; // not implemented yet
		u32 play_backwards; // not implemented yet
		u32 toggle_zoom_mode; // not implemented yet
		u32 cursor_up; // d-pad, xyab, c-pad, c-stick
		u32 cursor_down; // d-pad, xyab, c-pad, c-stick
		u32 cursor_left; // d-pad, xyab, c-pad, c-stick
		u32 cursor_right; // d-pad, xyab, c-pad, c-stick
		u32 scroll_up; // d-pad, xyab, c-pad, c-stick (for zoom-mode, not implemented yet)
		u32 scroll_down; // d-pad, xyab, c-pad, c-stick (for zoom-mode, not implemented yet)
		u32 scroll_left; // d-pad, xyab, c-pad, c-stick
		u32 scroll_right; // d-pad, xyab, c-pad, c-stick
	} key_bindings[2];
} settings;

extern u8 settings_icon[];

#ifdef NO_SF2D
#define ABGR
#endif
#endif
