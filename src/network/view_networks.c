#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "draw.h"
#include "view_networks.h"
#include "network_menu.h"
#include "network.h"
#include "ingame.h"
#include "import_level.h"
#include "gamespecific_2p.h"

#define MAX(x,y) ((x)>=(y)?(x):(y))

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
void draw_2p_level_selection(
		const char* game,
		u8 cur_level,
		u8 top_offset,
		u8 num_2p_level,
		const char* level_names,
		u8 progress,
		struct MainMenuData* menu_data) {
	tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);

	char msg[30*(40+1)+1];
	char* msg_ptr = msg;
	DRAW_MENU("\n  Select a 2 player level\n\n\n  Game: ");
	DRAW_MENU(game);
	DRAW_MENU("\n\n");
	u8 i;
	for (i=0;i<24;i++) {
		u8 lvl = i+top_offset;
		u8 level_no = lvl + 1;
		if (lvl >= num_2p_level) {
			break;
		}
		if (i==cur_level-top_offset) {
			DRAW_MENU("->");
		}else{
			DRAW_MENU("  ");
		}
		char num[3] = {
			'0' + (level_no/10)%10,
			'0' + level_no%10,
			0};
		DRAW_MENU(num);
		DRAW_MENU(": ");
		if (progress >= lvl) {
			DRAW_MENU(level_names+33*lvl);
			DRAW_MENU("\n");
		}else{
			DRAW_MENU_LINE("--------------------------------");
		}
	}
	draw_menu_text(BOTTOM_SCREEN_BACK,menu_data,0,0,msg,0,0.5f);
}
#undef DRAW_MENU
#undef DRAW_MENU_LINE

int select_2p_level(
		u8* game,
		u8* lvl,
		u8 num_2p_level[2],
		char* name_2p_level,
		u8 multiplayer_progress[2],
		struct MainMenuData* menu_data,
		struct MainInGameData* main_data) {

	// SHOW LEVEL SELECTION SCREEN!
	if (*game > 1) {
		*game = 0;
		*lvl = 0;
	}
	int redraw_selection = 1;
	int cur_lev = *lvl;
	int top = 0;
	if (num_2p_level[*game]) {
		*lvl = *lvl%num_2p_level[*game];
		if (*lvl>20) {
			top = *lvl-20;
		}
	}else{
		*lvl = 0;
		if (*game) {
			*game = 0;
		}else{
			*game = 1;
		}
		if (!num_2p_level[*game]) {
			// no 2p levels found
			// TODO: show error screen
			return MENU_ACTION_EXIT;
		}
	}
	int dwn = 0;
	int up = 0;

	u16 level_names_offset = 0;
	u8 i;
	for (i=0;i<*game;i++) {
		level_names_offset += num_2p_level[i];
	}

	u32 kDown;
	u32 kHeld;
	while (aptMainLoop()) {

		hidScanInput();
		kDown = hidKeysDown();
		kHeld = hidKeysHeld();
		//kUp = hidKeysUp();
		// hidTouchRead(&stylus);

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

		if (kDown & KEY_B) {
			// exit selection
			return MENU_ACTION_EXIT;
		}

		if ((kDown & KEY_DOWN) || dwn >= 20) {
			if (dwn) {
				dwn = 18;
			}else{
				dwn = 1;
			}

			if (cur_lev+1 < num_2p_level[*game]) {
				cur_lev++;
				if (cur_lev > top+20
						&& cur_lev+3
							< num_2p_level[*game]) {
					top++;
				}
			}else if (kDown & KEY_DOWN){
				cur_lev = 0;
				top = cur_lev;
			}
			redraw_selection = 1;
		}

		if ((kDown & KEY_UP) || up >= 20) {
			if (up) {
				up = 18;
			}else{
				up = 1;
			}
			if (cur_lev) {
				cur_lev--;
				if (cur_lev < top+3) {
					if (top) {
						top--;
					}
				}
			}else if (kDown & KEY_UP){
				top = cur_lev + MAX(num_2p_level[*game],24)-24;
				if (num_2p_level[*game]) {
					cur_lev = num_2p_level[*game]-1;
				}else{
					cur_lev = 0;
				}
			}
			redraw_selection = 1;
		}
		if (kDown & KEY_LEFT) {
			if (*game) {
				if (num_2p_level[(*game)-1]) {
					cur_lev = 0;
					top = cur_lev;
					(*game)--;
					level_names_offset -= num_2p_level[*game];
					redraw_selection = 1;
				}
			}
		}
		if (kDown & KEY_RIGHT) {
			if (*game < 1) {
				if (num_2p_level[(*game)+1]) {
					cur_lev = 0;
					top = cur_lev;
					level_names_offset += num_2p_level[*game];
					(*game)++;
					redraw_selection = 1;
				}
			}
		}
		if (kDown & (KEY_A | KEY_START)) {
			if (multiplayer_progress[*game]>=cur_lev) {
				// start level!!
				*lvl = cur_lev;
				return MENU_ACTION_LEVEL_SELECTED;
			}
		}

		if (redraw_selection) {
			draw_2p_level_selection(
					(*game==0)?"Lemmings":"Oh No! More Lemmings",
					cur_lev,
					top,
					num_2p_level[*game],
					name_2p_level+33*level_names_offset,
					multiplayer_progress[*game],
					menu_data
					);
			redraw_selection = 0;
		}
		begin_frame();
		copy_from_backbuffer(TOP_SCREEN);
		copy_from_backbuffer(BOTTOM_SCREEN);
		end_frame();
	}
	return MENU_EXIT_GAME;
}


void draw_2p_results(
		u8 lemmings[2],
		u16 won[2],
		char* other_players_name,
		u8 is_server,
		u8 cntn,
		struct MainMenuData* menu_data)
{
	char* usr = other_players_name;
	char* usr_align_left = &other_players_name[41];
	char* usr_align_right = &other_players_name[41+6];
	char msg[(20+1)*12+1+60];
	u8 player = (is_server?0:1);
	sprintf(
			msg,
			"     %s     \n"
			"                    \n"
			"  Lemmings saved:   \n"
			"         You: %3u   \n"
			"  %s%s%s: %3u   \n"
			"                    \n"
			"     Games won:     \n"
			"         You: %3u   \n"
			"  %s%s%s: %3u   \n"
			"                    \n"
			"%s",
			lemmings[0]==lemmings[1]
				?"   Tie    "
				:(lemmings[player]>lemmings[1-player]
					?" You won! "
					:"You lost! "),
			lemmings[player],
			usr_align_left,
			usr_align_right,
			usr,
			lemmings[1-player],
			won[player],
			usr_align_left,
			usr_align_right,
			usr,
			won[1-player],
			is_server?(
				!cntn
					?"Press A or B to exit\n"
					:"Press A to continue \n"
					 "   or B to exit.    \n"
			):(
				!cntn
					?"Press A or B to exit\n"
					:" Wait for server or \n"
					 "  press B to exit.  \n"
			));
	tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);
	draw_menu_text(
			BOTTOM_SCREEN_BACK,
			menu_data,
			0,
			24,
			msg,
			0,
			1.0f);
	begin_frame();
	copy_from_backbuffer(TOP_SCREEN);
	copy_from_backbuffer(BOTTOM_SCREEN);
	end_frame();
}

int show_2p_results(
		u8 lemmings[2],
		u16 won[2],
		char* other_players_name,
		u8 is_server,
		u8 is_connected,
		udsBindContext* bindctx, // only necessary if is_client and is_connected, otherwise NULL
		struct MainMenuData* menu_data)
{
	u8 cntn = (lemmings[0] || lemmings[1]) && is_connected;
	draw_2p_results(lemmings, won, other_players_name, is_server, cntn, menu_data);
	while (aptMainLoop()) {
		// wait for A pressed
		hidScanInput();
		if (is_server && cntn && ((hidKeysDown() & KEY_A) || (hidKeysDown() & KEY_TOUCH))) {
			return 1;
		}
		if ((hidKeysDown() & KEY_B) || (!cntn && ((hidKeysDown() & KEY_A) || (hidKeysDown() & KEY_TOUCH)))) {
			return 0;
		}
		if (!is_server && cntn) {
			union {
				u8 msg;
				struct NW_Level_Result res;
				struct NW_GameInit init;
			} tmp;
			size_t actual_size = 0;
			u16 src_NetworkNodeID;
			Result ret = udsPullPacket(
					bindctx,
					&tmp,
					sizeof(tmp),
					&actual_size,
					&src_NetworkNodeID);
			if (R_FAILED(ret)) {
				// lost connection
				cntn = 0;
				draw_2p_results(lemmings, won, other_players_name, is_server, cntn, menu_data);
			}
			if (actual_size == sizeof(struct NW_Level_Result)
					&& tmp.msg == NW_LVLRESULT) {
				udsSendTo(
						UDS_BROADCAST_NETWORKNODEID,
						1,
						UDS_SENDFLAG_Default,
						&tmp,
						1);
			} else if (actual_size == sizeof(struct NW_GameInit)
					&& tmp.msg == NW_INITIALIZE) {
				// run next level!
				return 1;
			}
		}
		if (cntn) {
			if (!connection_alive()) {
				cntn = 0;
				draw_2p_results(lemmings, won, other_players_name, is_server, cntn, menu_data);
			}
		}
	}
	return 0;
}


int on_client_connect(struct SaveGame* savegame, u8 game, u8 lvl, struct MainMenuData* menu_data, struct MainInGameData* main_data, udsNodeInfo* nodeinfo, u16 client_id, udsBindContext* bindctx) {
	// ask user whether he wants to play with the new client
	char usr[41+2*6];
	if (!get_aligned_username(usr, nodeinfo)) {
		return 0;
	}
	char* usr_align_left = &usr[41];
	char* usr_align_right = &usr[41+6];

	char* msg = (char*)malloc((20+1)*5+1+30);
	sprintf(msg,
			"  %s%s wants%s  \n"
			" to play with you.  \n"
			"                    \n"
			" Press A to accept  \n"
			" or B to decline.   \n",
			usr_align_left,
			usr,
			usr_align_right);
	tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);
	draw_menu_text(
			BOTTOM_SCREEN_BACK,
			menu_data,
			0,
			72,
			msg,
			0,
			1.0f);
	free(msg);
	msg = 0;
	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_B) {
			return MENU_HOST_REJECT_CLIENT; // eject client
		}
		if (hidKeysDown() & KEY_A) {
			// start network game with said client: send level to client
			u8 num_lvl = count_custom_levels(import_2p[game].level_path);
			struct Level* level = 0;
			if (lvl <= num_lvl) {
				level = (struct Level*)malloc(sizeof(struct Level));
			}
			if (level) {
				u8 lemmings[2] = {0, 0};
				u16 won[2] = {0, 0};
				do {
					tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);
					draw_menu_text(
							BOTTOM_SCREEN_BACK,
							menu_data,
							0,
							96,
							"   Connecting...    \n",
							0,
							1.0f);
					begin_frame();
					copy_from_backbuffer(TOP_SCREEN);
					copy_from_backbuffer(BOTTOM_SCREEN);
					end_frame();

					// calculate number of lemmings based on lemmings saved
					int i;
					for (i=0;i<2;i++) {
						if (!settings.two_player_always_equal) {
							lemmings[i] = (lemmings[i]>40?80:(40+lemmings[i]));
						}else{
							lemmings[i] = 40;
						}
					}
					int res = server_prepare_level(bindctx, lemmings, game, lvl, level);
					if (res != NETWORK_SUCCESS) {
						free(level);
						return show_network_error(res, menu_data);
					}
					// start game!
					char lvlnum[4];
					sprintf(lvlnum,"%02u:",(lvl+1)%100);
					res = server_run_level(bindctx, level, lvlnum, lemmings, menu_data, main_data);
					if (res != NETWORK_SUCCESS) {
						free_objects(level->object_types);
						free(level);
						return show_network_error(res, menu_data);
					}
					free_objects(level->object_types);
					if (lemmings[0] > lemmings[1]) {
						won[0]++;
					}else if (lemmings[1] > lemmings[0]) {
						won[1]++;
					}
					int inform_client = server_send_result(bindctx, lemmings, won);
					// show results; ...
					if (lemmings[0] || lemmings[1]) {
						if (lvl >= savegame->multiplayer_progress[game]) {
							savegame->multiplayer_progress[game] = lvl+1;
						}
						lvl++;
						lvl %= num_lvl;
						savegame->last_multiplayer_game = game;
						savegame->last_multiplayer_level = lvl;
						write_savegame(savegame);
					}
					if (!show_2p_results(lemmings, won, usr, 1, inform_client, 0, menu_data)) {
						break; // end match
					}
				}while(aptMainLoop());
				free(level);
				level = 0;
				// TODO: clean up?
			}
			return MENU_ACTION_EXIT; // done
		}
		// test if client quit
		if (udsWaitConnectionStatusEvent(false, false)) {
			udsConnectionStatus constatus;
			Result ret = udsGetConnectionStatus(&constatus);
			if(R_FAILED(ret)) {
				return MENU_ERROR;
			}
			if (!(constatus.node_bitmask & (1 << client_id))) {
				// lost client
				return MENU_CLIENT_QUIT;
			}
			if (constatus.total_nodes > 2) {
				return MENU_ERROR;
			}
		}
		begin_frame();
		copy_from_backbuffer(TOP_SCREEN);
		copy_from_backbuffer(BOTTOM_SCREEN);
		end_frame();
	}
	return MENU_EXIT_GAME;
}

int host_game(struct SaveGame* savegame, u8 num_levels[2], char* level_names, struct MainMenuData* menu_data, struct MainInGameData* main_data) {

	// level selection
	u8 game = savegame->last_multiplayer_game;
	u8 lvl = savegame->last_multiplayer_level;
	int res = select_2p_level(
		&game,
		&lvl,
		num_levels,
		level_names,
		savegame->multiplayer_progress,
		menu_data,
		main_data);
	if (res != MENU_ACTION_LEVEL_SELECTED) {
		return res;
	}
	savegame->last_multiplayer_game = game;
	savegame->last_multiplayer_level = lvl;
	write_savegame(savegame);

	udsNetworkStruct networkstruct;
	udsBindContext bindctx;
	udsGenerateDefaultNetworkStruct(&networkstruct, WLAN_ID, 0, 2);
	Result ret = udsCreateNetwork(
			&networkstruct,
			WLAN_PASS,
			strlen(WLAN_PASS),
			&bindctx,
			1,
			UDS_DEFAULT_RECVBUFSIZE);
	if(R_FAILED(ret)) {
		return show_network_error(NETWORK_ERROR_WLAN_LOST, menu_data);
	}

	u8 appdata[0x02] = {
			NETWORK_MIN_PROTOCOL_VERSION,
			NETWORK_PROTOCOL_VERSION};
	ret = udsSetApplicationData(appdata, sizeof(appdata));
	if(R_FAILED(ret)) {
		udsDestroyNetwork();
		udsUnbind(&bindctx);
		return show_network_error(NETWORK_ERROR_WLAN_LOST, menu_data);
	}
	udsEjectSpectator();
	const char* msg =
			"    Waiting for     \n"
			"    opponent to     \n"
			"    connect...      \n"
			"                    \n"
			"     Press B to     \n"
			"       abort.       \n";
	tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);
	draw_menu_text(
			BOTTOM_SCREEN_BACK,
			menu_data,
			0,
			72,
			msg,
			0,
			1.0f);
	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_B) {
			// stop hosting network game, return to network game view
			udsDestroyNetwork();
			udsUnbind(&bindctx);
			return MENU_ACTION_START_MULTI_PLAYER;
		}
		// check connection status.
		udsConnectionStatus constatus;
		ret = udsGetConnectionStatus(&constatus);
		if(R_FAILED(ret) || constatus.status != 0x06) {
			udsDestroyNetwork();
			udsUnbind(&bindctx);
			if (show_network_error(NETWORK_ERROR_WLAN_LOST, menu_data) != MENU_EXIT_GAME) {
				return MENU_EXIT_NETWORK;
			}else{
				return MENU_EXIT_GAME;
			}
		}
		if (constatus.total_nodes == 2) {
			u16 other_mask = (constatus.node_bitmask & ~(1 << (constatus.cur_NetworkNodeID-1)));
			u8 error = 1;
			if (other_mask) {
				u16 i;
				for (i=1; i<16; i++) {
					if (other_mask == (1 << (i-1))) {
						udsNodeInfo nodeinfo;
						ret = udsGetNodeInformation(i, &nodeinfo);
						if(!R_FAILED(ret)) {
							error = 0;
							udsSetNewConnectionsBlocked(true, true, false);
							appdata[0] = 255;
							appdata[1] = 0;
							udsSetApplicationData(appdata, sizeof(appdata));
							int res = on_client_connect(
									savegame,
									game,
									lvl,
									menu_data,
									main_data,
									&nodeinfo,
									i,
									&bindctx);
							appdata[0] = NETWORK_MIN_PROTOCOL_VERSION;
							appdata[1] = NETWORK_PROTOCOL_VERSION;
							udsSetApplicationData(appdata, sizeof(appdata));
							udsSetNewConnectionsBlocked(false, true, false);
							if (res == MENU_EXIT_GAME) {
								udsDestroyNetwork();
								udsUnbind(&bindctx);
								return MENU_EXIT_GAME;
							}
							if (res == MENU_ACTION_EXIT) {
								udsDestroyNetwork();
								udsUnbind(&bindctx);
								return MENU_ACTION_EXIT;
							}
							if (res == MENU_HOST_REJECT_CLIENT) {
								// kick client
								udsEjectClient(i);
							}
							tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);
							draw_menu_text(
									BOTTOM_SCREEN_BACK,
									menu_data,
									0,
									72,
									msg,
									0,
									1.0f);
							if (res == MENU_ERROR) {
								error = 1;
							}
						}
						break;
					}
				}
			}
			if (error) {
				udsDestroyNetwork();
				udsUnbind(&bindctx);
				return show_network_error(NETWORK_ERROR_OTHER, menu_data);
			}
		}else if (constatus.total_nodes > 2) {
			udsDestroyNetwork();
			udsUnbind(&bindctx);
			return show_network_error(NETWORK_ERROR_OTHER, menu_data);
		}
		begin_frame();
		copy_from_backbuffer(TOP_SCREEN);
		copy_from_backbuffer(BOTTOM_SCREEN);
		end_frame();
	}
	udsDestroyNetwork();
	udsUnbind(&bindctx);
	return MENU_EXIT_GAME;
}

int connect_to_network(const udsNetworkScanInfo* scan_info, struct MainMenuData* menu_data, struct MainInGameData* main_data) {
	// try to connect
	const char* msg =
			"   Connecting...    \n"
			"                    \n"
			"     Press B to     \n"
			"       abort.       \n";
	tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);
	draw_menu_text(
			BOTTOM_SCREEN_BACK,
			menu_data,
			0,
			72,
			msg,
			0,
			1.0f);
	begin_frame();
	copy_from_backbuffer(TOP_SCREEN);
	copy_from_backbuffer(BOTTOM_SCREEN);
	end_frame();
	udsBindContext bindctx;
	int attempt;
	Result ret;
	union {
		u8 msg_type;
		struct NW_GameInit gi;
	} tmpbuf;
	for(attempt=0; attempt<10; attempt++) {
		ret = udsConnectNetwork(
				&scan_info->network,
				WLAN_PASS,
				strlen(WLAN_PASS),
				&bindctx,
				UDS_BROADCAST_NETWORKNODEID,
				UDSCONTYPE_Client,
				1,
				UDS_DEFAULT_RECVBUFSIZE);
		if(!R_FAILED(ret)) {
			break;
		}
		hidScanInput();
		u32 kDown = hidKeysDown();
		if (kDown & KEY_B) {
			return 0; // give up
		}
	}
	if(attempt>=10) {
		show_network_error(NETWORK_ERROR_CONNECTION_LOST, menu_data);
		return 0;
	}
	// connected!
	// now wait for server to accept
	while (aptMainLoop()) {
		// get data
		size_t actual_size;
		u16 src_NetworkNodeID;
		ret = udsPullPacket(
				&bindctx,
				&tmpbuf,
				sizeof(struct NW_GameInit),
				&actual_size,
				&src_NetworkNodeID);
		hidScanInput();
		u32 kDown = hidKeysDown();
		// test for connection to be closed!!! -> break up
		if (R_FAILED(ret) || (kDown & KEY_B)) {
			udsDisconnectNetwork();
			udsUnbind(&bindctx);
			if (R_FAILED(ret)) {
				show_network_error(NETWORK_ERROR_CONNECTION_LOST, menu_data);
			}
			return 0; // lost connection
		}
		// test for acceptance message
		if (actual_size) {
			// got some packet. server seems to accept connection. jippieh!
			// parse NW_INITIALIZE packet
			if (tmpbuf.msg_type == NW_INITIALIZE
					&& actual_size == sizeof(struct NW_GameInit)) {
				// start game
				// confirm receivement of NW_INITIALIZE packet
				udsSendTo(UDS_HOST_NETWORKNODEID,1,UDS_SENDFLAG_Default,&tmpbuf,1);
				// parse NW_INITIALIZE payload
				u8 lemmings[2] = {
						tmpbuf.gi.lemmings_per_player[0],
						tmpbuf.gi.lemmings_per_player[1]
				};
				/* u8 num_players = init->num_players;
				u8 client_id = init->receiver_id; */
				// receive level
				struct Level* level = (struct Level*)malloc(sizeof(struct Level));
				if (level) {
					char usr[41+2*6];
					if (!get_aligned_username(usr, &scan_info->nodes[0])) {
						free(level);
						udsDisconnectNetwork();
						udsUnbind(&bindctx);
						show_network_error(NETWORK_ERROR_OTHER, menu_data);
						return 0;
					}
					// save local settings
					u8 local_settings[] = {
							settings.glitch_direct_drop,
							settings.two_player_timeout,
							settings.two_player_inspect_level
					};
					// apply server settings
					settings.glitch_direct_drop = tmpbuf.gi.glitch_direct_drop;
					settings.two_player_timeout = tmpbuf.gi.timeout;
					settings.two_player_inspect_level = tmpbuf.gi.inspect_level;
					do {
						tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);
						draw_menu_text(
								BOTTOM_SCREEN_BACK,
								menu_data,
								0,
								96,
								"   Connecting...    \n",
								0,
								1.0f);
						begin_frame();
						copy_from_backbuffer(TOP_SCREEN);
						copy_from_backbuffer(BOTTOM_SCREEN);
						end_frame();
						u8 lvl = 0;
						int res = client_prepare_level(
								&bindctx,
								lemmings,
								&lvl,
								level);
						if (res != NETWORK_SUCCESS) {
							show_network_error(res, menu_data);
							break;
						}
						// start game; ...
						u8 lemmings[2];
						u16 won[2];
						char lvlname[4];
						sprintf(lvlname,"%02u:",(lvl+1)%100);
						res = client_run_level(
								&bindctx,
								level,
								lvlname,
								lemmings,
								won,
								menu_data,
								main_data);
							if (res != NETWORK_SUCCESS) {
							free_objects(level->object_types);
							show_network_error(res, menu_data);
							break;
						}
						free_objects(level->object_types);
						if (!show_2p_results(
								lemmings,
								won,
								usr,
								0,
								1,
								&bindctx,
								menu_data)) {
							break;
						}
					}while(aptMainLoop() && level);
					// restore local settings
					settings.glitch_direct_drop = local_settings[0];
					settings.two_player_timeout = local_settings[1];
					settings.two_player_inspect_level = local_settings[2];
					if (level) {
						// TODO: clean up;
						free(level);
						level = 0;
					}
				}
			}
			// done. -> exit connection
			udsDisconnectNetwork();
			udsUnbind(&bindctx);
			return 1;
		}
		begin_frame();
		copy_from_backbuffer(TOP_SCREEN);
		copy_from_backbuffer(BOTTOM_SCREEN);
		end_frame();
	}
	udsDisconnectNetwork();
	udsUnbind(&bindctx);
	return 0;
}
