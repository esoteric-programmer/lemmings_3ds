#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "settings_menu.h"
#include "draw.h"
#include "audio.h"

const char* settings_menu_topics[] = {
	"GAME MECHANICS",
	"AUDIO",
	"NETWORK GAME",
	"KEY BINDINGS",
	0
};

struct SettingsValues {
	const char* name;
	u8 value;
};

struct SettingsValues volume[10] = {
	{" Off",0},
	{" 12%",12},
	{" 25%",25},
	{" 37%",37},
	{" 50%",50},
	{" 62%",62},
	{" 75%",75},
	{" 87%",87},
	{"100%",100},
	{0,0}
};

struct SettingsValues audio_source[5] = {
	{"prefer custom",AUDIO_ORDER_PREFER_CUSTOM},
	{"  only custom",AUDIO_ORDER_ONLY_CUSTOM},
	{"  only adlib ",AUDIO_ORDER_ONLY_ADLIB},
	{"prefer adlib ",AUDIO_ORDER_PREFER_ADLIB},
	{0,0}
};

struct SettingsValues timeout_2p[4] = {
	{"  never   ",TIMEOUT_2P_NEVER},
	{"inactivity",TIMEOUT_2P_INACTIVITY},
	{"count down",TIMEOUT_2P_COUNTDOWN},
	{0,0}
};

struct MenuPoint {
	u8 type; // 0: check box; 1: key
	const char* name;
	void* value;
	struct Selection {
		u8 default_index;
		struct SettingsValues* values;
	} selection;
};

struct MenuPoint gamemechanics_points[] = {
	{0, "Nuke glitch", &settings.glitch_nuke, {0,0}},
	{0, "Entrance pausing glitch", &settings.glitch_entrance_pausing, {0,0}},
	{0, "Mining one-way walls glitch", &settings.glitch_mining_right_oneway, {0,0}},
	{0, "Shrugger glitch", &settings.glitch_shrugger, {0,0}},
	{0, "Direct drop glitch", &settings.glitch_direct_drop, {0,0}},
	{0, "Amiga background color", &settings.amiga_background, {0,0}},
	{0, 0, 0, {0,0}}
};

struct MenuPoint audio_points[] = {
		{2, "Music volume", &settings.music_volume, {8,volume}},
		{2, "Effects volume", &settings.sfx_volume, {8,volume}},
		{2, "Audio source", &settings.audio_order, {0,audio_source}},
		{0, 0, 0, {0,0}}
};

struct MenuPoint network_game_points[] = {
	{0, "Always 40 lemmings", &settings.two_player_always_equal, {0,0}},
	{0, "Inspect level", &settings.two_player_inspect_level, {0,0}},
	{2, "Time out", &settings.two_player_timeout, {1,timeout_2p}},
	{0, 0, 0, {0,0}}
};

struct MenuPoint key_bindings_points[] = {
		{1, "Modifier key 1", &settings.key_bindings[0].modifier, {0,0}},
		{1, "Modifier key 2", &settings.key_bindings[1].modifier, {0,0}},
		{1, "Move cursor up", &settings.key_bindings[0].cursor_up, {0,0}},
		{1, "Move cursor down", &settings.key_bindings[0].cursor_down, {0,0}},
		{1, "Move cursor left", &settings.key_bindings[0].cursor_left, {0,0}},
		{1, "Move cursor right", &settings.key_bindings[0].cursor_right, {0,0}},
		{1, "Cursor click", &settings.key_bindings[0].click, {0,0}},
		{1, "Inc release rate", &settings.key_bindings[0].inc_rate, {0,0}},
		{1, "Dec release rate", &settings.key_bindings[0].dec_rate, {0,0}},
		{1, "Next skill", &settings.key_bindings[0].next_skill, {0,0}},
		{1, "Prev skill", &settings.key_bindings[0].prev_skill, {0,0}},
		{1, "Pause", &settings.key_bindings[0].pause, {0,0}},
		{1, "Nuke", &settings.key_bindings[0].nuke, {0,0}},
		{1, "Exit", &settings.key_bindings[0].exit, {0,0}},
		{1, "Speed up", &settings.key_bindings[0].speed_up, {0,0}},
		{1, "Nonprio lemming", &settings.key_bindings[0].non_prio, {0,0}},
		{1, "Step one frame", &settings.key_bindings[0].step_one_frame, {0,0}},
		{1, "Scroll left", &settings.key_bindings[0].scroll_left, {0,0}},
		{1, "Scroll right", &settings.key_bindings[0].scroll_right, {0,0}},
		{0, 0, 0, {0,0}}
};

struct MenuPoint* settings_menu_points[] = {
	gamemechanics_points,
	audio_points,
	network_game_points,
	key_bindings_points,
	0
};

const char* get_key_name(u32 key_mask) {
	switch (key_mask) {
		case KEY_A:
			return "A";
		case KEY_B:
			return "B";
		case KEY_X:
			return "X";
		case KEY_Y:
			return "Y";
		case KEY_L:
			return "L";
		case KEY_R:
			return "R";
		case KEY_ZL:
			return "ZL";
		case KEY_ZR:
			return "ZR";
		case KEY_SELECT:
			return "SEL";
		case KEY_START:
			return "START";
		case KEY_DUP:
			return "DU";
		case KEY_DDOWN:
			return "DD";
		case KEY_DLEFT:
			return "DL";
		case KEY_DRIGHT:
			return "DR";
		case KEY_CPAD_UP:
			return "CP U";
		case KEY_CPAD_DOWN:
			return "CP D";
		case KEY_CPAD_LEFT:
			return "CP L";
		case KEY_CPAD_RIGHT:
			return "CP R";
		case KEY_CSTICK_UP:
			return "CS U";
		case KEY_CSTICK_DOWN:
			return "CS D";
		case KEY_CSTICK_LEFT:
			return "CS L";
		case KEY_CSTICK_RIGHT:
			return "CS R";
		default:
			return 0;
	}
}

int get_keys_name(char name[20], u32 key_mask) {
	if (!name) {
		return 0;
	}
	char* name_ptr = name;
	int left = 19;
	u32 mask;
	*name = 0;
	for (mask=1;mask;mask<<=1) {
		const char* add = get_key_name(key_mask & mask);
		if (add) {
			size_t add_len = strlen(add);
			if (add_len > left || (*name && add_len+3 > left)) {
				return 0;
			}else{
				if (*name) {
					memcpy(name_ptr, " + ", 3);
					name_ptr += 3;
					left -= 3;
				}
				memcpy(name_ptr, add, add_len+1);
				name_ptr += add_len;
				left -= add_len;
			}
		}
	}
	if (!*name) {
		return 0;
	}
	return 1;
}

// draw level selection screen to im_bottom
#define DRAW_MENU(string) \
	{\
		if (cur_offset >= top_offset && cur_offset < top_offset + 28) {\
			size_t len = strlen(string);\
			memcpy(msg_ptr,(string),len+1);\
			msg_ptr += len;\
		}\
	}
#define DRAW_MENU_LINE(string) \
	{\
		if (cur_offset >= top_offset && cur_offset < top_offset + 28) {\
			size_t len = strlen(string);\
			memcpy(msg_ptr,(string),len+1);\
			msg_ptr += len;\
			*msg_ptr = '\n';\
			msg_ptr++;\
		}\
		cur_offset++;\
	}
void draw_settings_menu(
		u8 top_offset,
		u8 cur_topic,
		u8 cur_point,
		u8 blink,
		u32** menu_values,
		struct MainMenuData* menu_data) {
	tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);

	char msg[30*(40+1)+1];
	sprintf(msg,"\n SETTINGS\n\n");
	char* msg_ptr = msg + strlen(msg);
	u8 cur_offset = 0;
	u8 draw_topic;
	for (draw_topic = 0; settings_menu_points[draw_topic]; draw_topic++) {
		DRAW_MENU(" ")
		DRAW_MENU_LINE(settings_menu_topics[draw_topic])
		u8 draw_line;
		for (draw_line = 0;settings_menu_points[draw_topic][draw_line].value;draw_line++) {
			if (cur_topic == draw_topic && cur_point == draw_line) {
				DRAW_MENU("->")
			}else{
				DRAW_MENU("  ")
			}
			if (settings_menu_points[draw_topic][draw_line].type == 0) {
				DRAW_MENU("[")
				if (menu_values[draw_topic][draw_line]) {
					DRAW_MENU("X")
				}else{
					DRAW_MENU(" ")
				}
				DRAW_MENU("] ")
			}
			DRAW_MENU(settings_menu_points[draw_topic][draw_line].name)
			if (settings_menu_points[draw_topic][draw_line].type == 1
					|| settings_menu_points[draw_topic][draw_line].type == 2) {
				char spaces[20];
				memset(spaces,' ',19);
				spaces[19-strlen(
						settings_menu_points[draw_topic][draw_line]
							.name)] = 0;
				DRAW_MENU(spaces)
			}
			if (settings_menu_points[draw_topic][draw_line].type == 1) {
				if (!blink || draw_topic != cur_topic || draw_line != cur_point) {
					char name[20];
					if (get_keys_name(name, menu_values[draw_topic][draw_line])) {
						DRAW_MENU(name)
					}else{
						DRAW_MENU("---")
					}
				}
			}
			if (settings_menu_points[draw_topic][draw_line].type == 2) {
				u8 idx = settings_menu_points[draw_topic][draw_line].selection.default_index;
				u8 i;
				for (i=0;settings_menu_points[draw_topic][draw_line].selection.values[i].name;i++) {
					if (settings_menu_points[draw_topic][draw_line].selection.values[i].value
							== menu_values[draw_topic][draw_line]) {
						idx = i;
					}
				}
				if (idx) {
					DRAW_MENU("< ")
				}else{
					DRAW_MENU("  ")
				}
				DRAW_MENU(settings_menu_points[draw_topic][draw_line].selection.values[idx].name)
				if (idx+1 < i) {
					DRAW_MENU(" >")
				}/*else{
					DRAW_MENU("  ")
				}*/
			}
			DRAW_MENU_LINE("")
		}
		DRAW_MENU_LINE("")
	}
	if (cur_topic == draw_topic && cur_point == 0) {
		DRAW_MENU("->")
	}else{
		DRAW_MENU("  ")
	}
	DRAW_MENU_LINE("OK")
	if (cur_topic == draw_topic && cur_point == 1) {
		DRAW_MENU("->")
	}else{
		DRAW_MENU("  ")
	}
	DRAW_MENU_LINE("CANCEL")
	*msg_ptr = 0;
	draw_menu_text(BOTTOM_SCREEN_BACK,menu_data,0,0,msg,0,0.5f);
}

int settings_menu(struct SaveGame* savegame, struct MainMenuData* menu_data) {
	// init
	u8 redraw_selection = 1;
	// TODO: don't use hard coded size of menu entries
	//       malloc dynamically instaed
	u32 gamemechanics_values[6];
	u32 audio_values[3];
	u32 network_game_values[3];
	u32 key_bindings_values[19];
	u32* menu_values[4] = {
		gamemechanics_values,
		audio_values,
		network_game_values,
		key_bindings_values
	};
	u8 topic;
	u8 lines = 0;
	for (topic = 0; settings_menu_points[topic]; topic++) {
		u8 option;
		lines += 2;
		for (option = 0;settings_menu_points[topic][option].value;option++) {
			lines++;
			switch (settings_menu_points[topic][option].type) {
				case 0:
					menu_values[topic][option] =
							*(u8*)settings_menu_points[topic][option].value;
					break;
				case 1:
					menu_values[topic][option] =
							*(u32*)settings_menu_points[topic][option].value;
					break;
				case 2:
					menu_values[topic][option] =
							*(u8*)settings_menu_points[topic][option].value;
					break;
				default:
					menu_values[topic][option] = 0;
					break;
			}
		}
	}
	lines += 2;
	u8 top_offset = 0;
	u8 cur_topic = 0;
	u8 cur_point = 0;
	u8 cur_line = 1;
	u8 key_assignment = 0;
	u32 modifier_mask = 0;
	u32 kDown;
	u32 kHeld;
	int dwn = 0;
	int up = 0;
	int lft = 0;
	int rght = 0;

	while (aptMainLoop()) {
		hidScanInput();
		kDown = hidKeysDown();
		kHeld = hidKeysHeld();
		if (key_assignment) {
			dwn = 0;
			up = 0;
			lft = 0;
			rght = 0;
			if (kDown & KEY_TOUCH) {
				// cancel assignment
				key_assignment = 0;
				redraw_selection = 1;
				continue;
			}
			if (kDown & ~modifier_mask) {
				// update other key settings (remove key)
				// TODO: don't use hard coded positions of modifier keys...
				u32 key_code = kDown | (kHeld & modifier_mask);
				u32 remove_modifier = 0;
				if (cur_point < 2 && menu_values[cur_topic][cur_point] != key_code) {
					remove_modifier |= key_code;
				}
				u8 i;
				for (i=0;settings_menu_points[cur_topic][i].value;i++) {
					if (i<2) {
						if (i == cur_point && key_code != menu_values[cur_topic][i]) {
							remove_modifier |= menu_values[cur_topic][i];
						}else if (i != cur_point && key_code == menu_values[cur_topic][i]){
							remove_modifier |= menu_values[cur_topic][i];
						}
					}else{
						if (menu_values[cur_topic][i] & remove_modifier) {
							menu_values[cur_topic][i] = 0;
						}
					}
					if (menu_values[cur_topic][i] == key_code) {
						menu_values[cur_topic][i] = 0;
					}
				}
				// assign key!
				menu_values[cur_topic][cur_point] =
						kDown | (kHeld & modifier_mask);
				key_assignment = 0;
				redraw_selection = 1;
				continue;
			}
			draw_settings_menu(
					top_offset,
					cur_topic,
					cur_point,
					osGetTime() % 1000 >= 500,
					menu_values,
					menu_data);
			redraw_selection = 0;
			begin_frame();
			copy_from_backbuffer(TOP_SCREEN);
			copy_from_backbuffer(BOTTOM_SCREEN);
			end_frame();
			continue;
		}

		if (kHeld & KEY_DOWN) {
			dwn++;
		} else if (!((kDown | kHeld) & KEY_DOWN)) {
			dwn = 0;
		}
		if (kHeld & KEY_UP) {
			up++;
		} else if (!((kDown | kHeld) & KEY_UP)) {
			up = 0;
		}
		if (kHeld & KEY_LEFT) {
			lft++;
		} else if (!((kDown | kHeld) & KEY_LEFT)) {
			lft = 0;
		}
		if (kHeld & KEY_RIGHT) {
			rght++;
		} else if (!((kDown | kHeld) & KEY_RIGHT)) {
			rght = 0;
		}

		if ((kDown & KEY_DOWN) || dwn >= 20) {
			if (dwn) {
				dwn = 18;
			}else{
				dwn = 1;
			}
			if (cur_topic == topic) {
				if (cur_point < 1) {
					cur_point++;
					cur_line++;
					if (lines-1 > 26
							&& top_offset < lines-1-26
							&& top_offset < cur_line-24) {
						top_offset = cur_line-24;
						if (top_offset > lines-1-26) {
							top_offset = lines-1-26;
						}
					}
				}else if (kDown & KEY_DOWN) {
					cur_topic = 0;
					cur_point = 0;
					cur_line = 1;
					top_offset = 0;
				}
			}else{
				cur_point++;
				cur_line++;
				if (!settings_menu_points[cur_topic][cur_point].value) {
					cur_point = 0;
					cur_topic++;
					cur_line+=(cur_topic==topic?1:2);
				}
				if (lines-1 > 26
						&& top_offset < lines-1-26
						&& top_offset < cur_line-24) {
					top_offset = cur_line-24;
					if (top_offset > lines-1-26) {
						top_offset = lines-1-26;
					}
				}
			}
			redraw_selection = 1;
		}
		if ((kDown & KEY_UP) || up >= 20) {
			if (up) {
				up = 18;
			}else{
				up = 1;
			}
			if (cur_point) {
				cur_point--;
				cur_line--;
				if (top_offset > 0 && top_offset+2 > cur_line) {
					top_offset = cur_line-2;
				}
			}else if (cur_topic) {
				cur_line-=(cur_topic==topic?2:3);
				cur_topic--;
				cur_point = 0;
				while (settings_menu_points[cur_topic][cur_point+1].value) {
					cur_point++;
				}
				if (top_offset > 0 && top_offset+2 > cur_line) {
					top_offset = cur_line-2;
				}
			}else if (kDown & KEY_UP) {
				cur_topic = topic;
				cur_point = 1;
				cur_line = lines-1;
				if (cur_line > 26) {
					top_offset = cur_line-26;
				}else{
					top_offset = 0;
				}
			}
			redraw_selection = 1;
		}
		if (kDown & KEY_A) {
			if (cur_topic == topic) {
				if (cur_point == 0) {
					kDown |= KEY_START;
				}else{
					kDown |= KEY_B;
				}
			}else{
				switch (settings_menu_points[cur_topic][cur_point].type) {
					case 0:
						menu_values[cur_topic][cur_point] =
								!menu_values[cur_topic][cur_point];
						redraw_selection = 1;
						break;
					case 1:
						key_assignment = 1;
						if (settings_menu_points[cur_topic][cur_point].value
								== &settings.key_bindings[0].modifier
								|| settings_menu_points[cur_topic][cur_point].value
								== &settings.key_bindings[1].modifier) {
							modifier_mask = 0;
						}else{
							// TODO: don't use hard coded positions of modifier keys...
							//       find them instead...
							modifier_mask = menu_values[2][0] | menu_values[2][1];
						}
						break;
					case 2:
						{
							u8 idx = settings_menu_points[cur_topic][cur_point].selection.default_index;
							u8 i;
							for (i=0;settings_menu_points[cur_topic][cur_point].selection.values[i].name;i++) {
								if (settings_menu_points[cur_topic][cur_point].selection.values[i].value
										== menu_values[cur_topic][cur_point]) {
									idx = i;
								}
							}
							if (idx) {
								idx = 0;
							}else if (i>0){
								idx = i-1;
							}
							menu_values[cur_topic][cur_point] =
									settings_menu_points[cur_topic][cur_point].selection.values[idx].value;
						}
						redraw_selection = 1;
						break;
					default:
						break;
				}
			}
		}
		if ((kDown & KEY_LEFT) || lft >= 20) {
			if (lft) {
				lft = 18;
			}else{
				lft = 1;
			}
			if (settings_menu_points[cur_topic][cur_point].type == 2) {
				u8 idx = settings_menu_points[cur_topic][cur_point].selection.default_index;
				u8 i;
				for (i=0;settings_menu_points[cur_topic][cur_point].selection.values[i].name;i++) {
					if (settings_menu_points[cur_topic][cur_point].selection.values[i].value
							== menu_values[cur_topic][cur_point]) {
						idx = i;
					}
				}
				if (idx) {
					idx--;
					menu_values[cur_topic][cur_point] =
							settings_menu_points[cur_topic][cur_point].selection.values[idx].value;
					redraw_selection = 1;
				}
			}
		}
		if ((kDown & KEY_RIGHT) || rght >= 20) {
			if (rght) {
				rght = 18;
			}else{
				rght = 1;
			}
			if (settings_menu_points[cur_topic][cur_point].type == 2) {
				u8 idx = settings_menu_points[cur_topic][cur_point].selection.default_index;
				u8 i;
				for (i=0;settings_menu_points[cur_topic][cur_point].selection.values[i].name;i++) {
					if (settings_menu_points[cur_topic][cur_point].selection.values[i].value
							== menu_values[cur_topic][cur_point]) {
						idx = i;
					}
				}
				if (idx+1 < i) {
					idx++;
					menu_values[cur_topic][cur_point] =
							settings_menu_points[cur_topic][cur_point].selection.values[idx].value;
					redraw_selection = 1;
				}
			}
		}
		if (kDown & KEY_X) {
			// unset current key
			if (cur_topic < topic) {
				if (settings_menu_points[cur_topic][cur_point].type == 1) {
					menu_values[cur_topic][cur_point] = 0;
					redraw_selection = 1;
				}
			}
		}
		if (kDown & KEY_B) {
			// exit menu without saving
			return MENU_ACTION_EXIT;
		}
		if (kDown & KEY_START) {
			// update settings
			for (topic = 0; settings_menu_points[topic]; topic++) {
				u8 option;
				for (option = 0;settings_menu_points[topic][option].value;option++) {
					switch (settings_menu_points[topic][option].type) {
						case 0:
							*(u8*)settings_menu_points[topic][option].value =
									menu_values[topic][option];
							break;
						case 1:
							*(u32*)settings_menu_points[topic][option].value =
									menu_values[topic][option];
							break;
						case 2:
							*(u8*)settings_menu_points[topic][option].value =
									menu_values[topic][option];
							break;
						default:
							// error
							break;
					}
				}
			}
			update_volume();
			// save
			write_savegame(savegame);
			// exit menu
			return MENU_ACTION_EXIT;
		}
		if (redraw_selection) {
			draw_settings_menu(
					top_offset,
					cur_topic,
					cur_point,
					0,
					menu_values,
					menu_data);
			redraw_selection = 0;
		}
		begin_frame();
		copy_from_backbuffer(TOP_SCREEN);
		copy_from_backbuffer(BOTTOM_SCREEN);
		end_frame();
	}
	return MENU_EXIT_GAME;
}
