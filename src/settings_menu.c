#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "settings_menu.h"
#include "draw.h"

//#define MAX(x,y) ((x)>=(y)?(x):(y))

const char* settings_menu_topics[] = {
	"GAME MECHANICS",
	"AUDIO",
	"KEY BINDINGS",
	0
};

struct MenuPoint {
	u8 type; // 0: check box; 1: key
	const char* name;
	void* value;
};

struct MenuPoint gamemechanics_points[] = {
	{0, "Nuke glitch", &settings.glitch_nuke},
	{0, "Entrance pausing glitch", &settings.glitch_entrance_pausing},
	{0, "Mining one-way walls glitch", &settings.glitch_mining_right_oneway},
	{0, "Shrugger glitch", &settings.glitch_shrugger},
	{0, 0, 0}
};

struct MenuPoint audio_points[] = {
		{0, "Music", &settings.music_volume},
		{0, "Effects", &settings.sfx_volume},
		{0, 0, 0}
};

struct MenuPoint key_bindings_points[] = {
		{1, "Modifier key 1", &settings.key_bindings[0].modifier},
		{1, "Modifier key 2", &settings.key_bindings[1].modifier},
		{1, "Move cursor up", &settings.key_bindings[0].cursor_up},
		{1, "Move cursor down", &settings.key_bindings[0].cursor_down},
		{1, "Move cursor left", &settings.key_bindings[0].cursor_left},
		{1, "Move cursor right", &settings.key_bindings[0].cursor_right},
		{1, "Cursor click", &settings.key_bindings[0].click},
		{1, "Inc release rate", &settings.key_bindings[0].inc_rate},
		{1, "Dec release rate", &settings.key_bindings[0].dec_rate},
		{1, "Next skill", &settings.key_bindings[0].next_skill},
		{1, "Prev skill", &settings.key_bindings[0].prev_skill},
		{1, "Pause", &settings.key_bindings[0].pause},
		{1, "Nuke", &settings.key_bindings[0].nuke},
		{1, "Exit", &settings.key_bindings[0].exit},
		{1, "Speed up", &settings.key_bindings[0].speed_up},
		{1, "Nonprio lemming", &settings.key_bindings[0].non_prio},
		{1, "Step one frame", &settings.key_bindings[0].step_one_frame},
		{1, "Scroll left", &settings.key_bindings[0].scroll_left},
		{1, "Scroll right", &settings.key_bindings[0].scroll_right},
		{0, 0, 0}
};

struct MenuPoint* settings_menu_points[] = {
	gamemechanics_points,
	audio_points,
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
			if (settings_menu_points[draw_topic][draw_line].type == 1) {
				char spaces[21];
				memset(spaces,' ',20);
				spaces[20-strlen(settings_menu_points[draw_topic][draw_line].name)] = 0;
				DRAW_MENU(spaces)
				char name[20];
				if (get_keys_name(name, menu_values[draw_topic][draw_line])) {
					DRAW_MENU(name)
				}else{
					DRAW_MENU("---")
				}
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

	// TODO: init
	u8 redraw_selection = 1;
	u32 gamemechanics_values[4];
	u32 audio_values[2];
	u32 key_bindings_values[19];
	u32* menu_values[3] = {
		gamemechanics_values,
		audio_values,
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
	u32 kDown;
	//u32 kHeld;
	while (aptMainLoop()) {

		hidScanInput();
		kDown = hidKeysDown();
		//kHeld = hidKeysHeld();
		//kUp = hidKeysUp();
		// hidTouchRead(&stylus);
		if (redraw_selection) {
			draw_settings_menu(
					top_offset,
					cur_topic,
					cur_point,
					menu_values,
					menu_data);
			redraw_selection = 0;
		}

		if (kDown & KEY_DOWN) {
			if (cur_topic == topic) {
				if (cur_point < 1) {
					cur_point++;
					cur_line++;
					if (lines-1 > 26 && top_offset < lines-1-26 && top_offset < cur_line-24) {
						top_offset = cur_line-24;
						if (top_offset > lines-1-26) {
							top_offset = lines-1-26;
						}
					}
				}else{
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
				if (lines-1 > 26 && top_offset < lines-1-26 && top_offset < cur_line-24) {
					top_offset = cur_line-24;
					if (top_offset > lines-1-26) {
						top_offset = lines-1-26;
					}
				}
			}
			redraw_selection = 1;
		}
		if (kDown & KEY_UP) {
			if (cur_point) {
				cur_point--;
				cur_line--;
				if (top_offset > 0 && top_offset+2 > cur_line) {
					top_offset = cur_line-2;
				}
			}else if (cur_topic){
				cur_line-=(cur_topic==topic?2:3);
				cur_topic--;
				cur_point = 0;
				while (settings_menu_points[cur_topic][cur_point+1].value) {
					cur_point++;
				}
				if (top_offset > 0 && top_offset+2 > cur_line) {
					top_offset = cur_line-2;
				}
			}else{
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
								menu_values[cur_topic][cur_point]?0:100;
						redraw_selection = 1;
						break;
					case 1:
						// TODO
						break;
					default:
						break;
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
			// save settings
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
						default:
							// error
							break;
					}
				}
			}
			// save
			write_savegame(savegame);
			// exit menu
			return MENU_ACTION_EXIT;
		}
		begin_frame();
		copy_from_backbuffer(TOP_SCREEN);
		copy_from_backbuffer(BOTTOM_SCREEN);
		end_frame();
	}
	return MENU_EXIT_GAME;
}
