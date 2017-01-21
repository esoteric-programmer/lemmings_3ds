#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "network_menu.h"
#include "view_networks.h"
#include "network.h"

int get_aligned_username(char username[41+2*6], const udsNodeInfo* nodeinfo) {
	if (!nodeinfo || !username) {
		return 0;
	}
	Result ret = udsGetNodeInfoUsername(nodeinfo, username);
	if(R_FAILED(ret)) {
		return 0;
	}
	int i;
	size_t pos = 0;
	for (i=0;i<10;i++) {
		u32 tmp2 = 0;
		s16 diff = 0;
		diff = decode_utf8(&tmp2, (u8*)&username[pos]);
		if (diff < 1 || tmp2 < ' ') {
			break;
		}
		pos += diff;
	}
	username[pos] = 0;
	char* usr_align_left = &username[41];
	char* usr_align_right = &username[41+6];
	memset(usr_align_left,' ',5);
	memset(usr_align_right,' ',5);
	usr_align_left[5-(i+1)/2] = 0;
	usr_align_right[5-i/2] = 0;
	return 1;
}

// draw level selection screen to im_bottom
#define DRAW_MENU(string) \
	{\
		size_t len = strlen(string);\
		memcpy(msg_ptr,(string),len+1);\
		msg_ptr += len;\
	}
#define DRAW_MENU_LINE(string) \
	{\
		size_t len = strlen(string);\
		memcpy(msg_ptr,(string),len);\
		msg_ptr += len;\
		*msg_ptr = '\n';\
		msg_ptr++;\
		*msg_ptr = 0;\
	}
void draw_network_view(
		u8 cur_selection,
		u8 total_networks,
		udsNetworkScanInfo* networks,
		struct MainMenuData* menu_data,
		u8* network_alive) {
	tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);

	char* msg = (char*)malloc(30*(40+1)+1);
	sprintf(msg,"\n Choose a Game\n\n");
	char* msg_ptr = msg + strlen(msg);
	u8 cur_offset = 0;
	for (cur_offset = 0; cur_offset < 4; cur_offset++) {
		if (network_alive) {
			network_alive[cur_offset] = 0;
		}
		if (cur_offset == cur_selection) {
			DRAW_MENU("->")
		}else{
			DRAW_MENU("  ")
		}
		char tmp[41+2*6];
		/*
		char* usr_align_left = &tmp[41];
		char* usr_align_right = &tmp[41+6];
		*/
		sprintf(tmp,"%u. ",cur_offset+1);
		DRAW_MENU(tmp);
		// u8 active = 0;
		if (cur_offset < total_networks) {
			if(udsCheckNodeInfoInitialized(&networks[cur_offset].nodes[0])){
				if (get_aligned_username(tmp, &networks[cur_offset].nodes[0])) {
					u8 appdata[0x02];
					size_t appdata_size = 0;
					Result ret = udsGetNetworkStructApplicationData(
							&networks[cur_offset].network,
							appdata,
							sizeof(appdata),
							&appdata_size);
					if(!R_FAILED(ret) && appdata_size >= 2) {
						if (NETWORK_PROTOCOL_VERSION >= appdata[0] &&
								NETWORK_MIN_PROTOCOL_VERSION <= appdata[1]) {
							DRAW_MENU(tmp);
							/*
							DRAW_MENU(usr_align_left);
							DRAW_MENU(usr_align_right);
							active = 1;
							sprintf(
									tmp,
									" %2u/%2u",
									networks[cur_offset].network.total_nodes,
									networks[cur_offset].network.max_nodes);
							DRAW_MENU(tmp); // number of connected clients
							*/
							network_alive[cur_offset] = 1;
						}
					}
				}
			}
		}
		/*
		if (!active) {
			DRAW_MENU("            0/ 0");
		}
		*/
		DRAW_MENU_LINE("")
	}
	DRAW_MENU_LINE("")
	if (cur_selection == 4) {
		DRAW_MENU("->")
	}else{
		DRAW_MENU("  ")
	}
	DRAW_MENU_LINE("Host a game")
	if (cur_selection == 5) {
		DRAW_MENU("->")
	}else{
		DRAW_MENU("  ")
	}
	DRAW_MENU_LINE("Cancel")
	draw_menu_text(BOTTOM_SCREEN_BACK,menu_data,0,0,msg,0,1.0f);
	free(msg);
}
#undef DRAW_MENU
#undef DRAW_MENU_LINE

int show_network_error(struct MainMenuData* menu_data) {
	tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);
	draw_menu_text(
			BOTTOM_SCREEN_BACK,
			menu_data,
			0,
			80,
			" Cannot access the  \n"
			"wireless connection.\n"
			"                    \n"
			"   Press any key    \n"
			"    to continue.    \n",
			0,
			1.0f);
	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() && ~(KEY_CPAD_LEFT | KEY_CPAD_RIGHT | KEY_CPAD_UP | KEY_CPAD_DOWN | KEY_CSTICK_LEFT | KEY_CSTICK_RIGHT | KEY_CSTICK_UP | KEY_CSTICK_DOWN)) {
			return MENU_ACTION_EXIT;
		}
		begin_frame();
		copy_from_backbuffer(TOP_SCREEN);
		copy_from_backbuffer(BOTTOM_SCREEN);
		end_frame();
	}
	return MENU_EXIT_GAME;
}

int network_menu(struct SaveGame* savegame, u8 num_levels[2], char* level_names, struct MainMenuData* menu_data, struct MainInGameData* main_data) {
	// init
	Result ret = udsInit(0x3000, NULL);//The sharedmem size only needs to be slightly larger than the total recv_buffer_size for all binds, with page-alignment.
	if(R_FAILED(ret)) {
		// show WLAN error message
		return show_network_error(menu_data);
	}

	//u8 redraw_selection = 1;
	u8 cur_selection = 0;
	u8 network_alive[4] = {0, 0, 0, 0};
	u32 kDown;
	u32 kHeld;
	int dwn = 0;
	int up = 0;
	size_t tmpbuf_size = (UDS_DATAFRAME_MAXSIZE>0x4000?UDS_DATAFRAME_MAXSIZE:0x4000);
	void* tmpbuf = malloc(tmpbuf_size);
	if (!tmpbuf) {
		udsExit();
		return MENU_ERROR;
	}
	size_t total_networks = 0;
	udsNetworkScanInfo* networks = 0;

	while (aptMainLoop()) {
		hidScanInput();
		kDown = hidKeysDown();
		kHeld = hidKeysHeld();

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

		if ((kDown & KEY_DOWN) || dwn >= 20) {
			if (dwn) {
				dwn = 18;
			}else{
				dwn = 1;
			}
			if (cur_selection < 5) {
				cur_selection++;
			}else if (kDown & KEY_DOWN) {
				cur_selection = 0;
			}
			//redraw_selection = 1;
		}
		if ((kDown & KEY_UP) || up >= 20) {
			if (up) {
				up = 18;
			}else{
				up = 1;
			}
			if (cur_selection) {
				cur_selection--;
			}else if (kDown & KEY_UP) {
				cur_selection = 5;
			}
			//redraw_selection = 1;
		}
		if (kDown & KEY_A) {
			if (cur_selection == 4) {
				kDown |= KEY_START;
			}else if (cur_selection == 5){
				kDown |= KEY_B;
			}else if (cur_selection < 4 && network_alive[cur_selection]){
				connect_to_network(&networks[cur_selection], menu_data, main_data);
				kDown = 0;
			}
		}
		if (kDown & KEY_B) {
			// exit menu without saving
			udsExit();
			free(tmpbuf);
			return MENU_ACTION_EXIT;
		}
		if (kDown & KEY_START) {
			// create own network
			int res = host_game(savegame, num_levels, level_names, menu_data, main_data);
			if (res == MENU_EXIT_GAME) {
				break;
			}
			kDown = 0;
		}
		// TODO: scanning blocks a long time, so do this in a seperate thread
		if (networks) {
			free(networks);
			networks = 0;
		}
		ret = udsScanBeacons(
				tmpbuf,
				tmpbuf_size,
				&networks,
				&total_networks,
				WLAN_ID,
				0,
				NULL,
				false);
		if (R_FAILED(ret)) {
			udsExit();
			free(tmpbuf);
			return show_network_error(menu_data);
		}
		//if (redraw_selection) {
			draw_network_view(
					cur_selection,
					total_networks,
					networks,
					menu_data,
					network_alive);
			//redraw_selection = 0;
		//}
		begin_frame();
		copy_from_backbuffer(TOP_SCREEN);
		copy_from_backbuffer(BOTTOM_SCREEN);
		end_frame();
	}
	if (networks) {
		free(networks);
		networks = 0;
	}
	udsExit();
	free(tmpbuf);
	return MENU_EXIT_GAME;
}
