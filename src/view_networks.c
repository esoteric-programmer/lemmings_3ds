#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "draw.h"
#include "view_networks.h"
#include "network.h"
#include "ingame.h"
#include "import_level.h"
#include "gamespecific_2p.h"

#define WLAN_ID 0x40F91730
#define WLAN_PASS "Lemmings for 3DS rulez"
#define NETWORK_PROTOCOL_VERSION 0
#define NETWORK_MIN_PROTOCOL_VERSION 0

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
	sprintf(msg,"\n Choose a Network Game\n\n");
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
		char tmp[41];
		sprintf(tmp,"%u. ",cur_offset+1);
		DRAW_MENU(tmp);
		u8 active = 0;
		if (cur_offset < total_networks) {
			if(udsCheckNodeInfoInitialized(&networks[cur_offset].nodes[0])){
				Result ret = udsGetNodeInfoUsername(&networks[cur_offset].nodes[0], tmp);
				if(!R_FAILED(ret)) {
					u8 appdata[0x02];
					size_t appdata_size = 0;
					ret = udsGetNetworkStructApplicationData(
							&networks[cur_offset].network,
							appdata,
							sizeof(appdata),
							&appdata_size);
					if(!R_FAILED(ret) && appdata_size >= 2) {
						if (NETWORK_PROTOCOL_VERSION >= appdata[0] &&
								NETWORK_MIN_PROTOCOL_VERSION <= appdata[1]) {
							int i;
							size_t pos = 0;
							int eos = 0;
							for (i=0;i<10;i++) {
								u32 tmp2 = 0;
								s16 diff = 0;
								if (!eos) {
									diff = decode_utf8(&tmp2, (u8*)&tmp[pos]);
								}
								if (diff < 1 || tmp2 < ' ') {
									tmp[pos] = ' ';
									diff = 1;
									eos = 1;
								}
								pos += diff;
							}
							tmp[pos] = 0;
							DRAW_MENU(tmp);
							active = 1;
							sprintf(
									tmp,
									" %2u/%2u",
									networks[cur_offset].network.total_nodes,
									networks[cur_offset].network.max_nodes);
							DRAW_MENU(tmp); // number of connected clients
							network_alive[cur_offset] = 1;
						}
					}
				}
			}
		}
		if (!active) {
			DRAW_MENU("            0/ 0");
		}
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
	draw_menu_text(BOTTOM_SCREEN_BACK,menu_data,0,0,msg,0,0.75f);
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

int get_aligned_username(char username[41+2*6], udsNodeInfo* nodeinfo) {
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

int on_client_connect(struct MainMenuData* menu_data, struct MainInGameData* main_data, udsNodeInfo* nodeinfo, u16 client_id, udsBindContext* bindctx) {
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
			u8 num_lvl = count_custom_levels(import_2p[0].level_path);
			struct Level* level = 0;
			if (num_lvl) {
				level = (struct Level*)malloc(sizeof(struct Level));
			}
			if (level) {
				u8 lvl = 0;
				u8 lemmings[2] = {0, 0};
				u16 won[2] = {0, 0};
				do {
					// calculate number of lemmings based on lemmings saved
					int i;
					for (i=0;i<2;i++) {
						lemmings[i] = (lemmings[i]>40?80:(40+lemmings[i]));
					}
					int res = server_prepare_level(bindctx, lemmings, 0, lvl, level);
					if (!res) {
						break;
					}
					// start game!
					server_run_level(bindctx, level, 0, lemmings, menu_data, main_data);
					if (!res) {
						free_objects(level->object_types);
						break;
					}
					free_objects(level->object_types);
					if (lemmings[0] > lemmings[1]) {
						won[0]++;
					}else if (lemmings[1] > lemmings[0]) {
						won[1]++;
					}
					int inform_client = server_send_result(bindctx, lemmings, won);
					// show results; ...
					msg = (char*)malloc((20+1)*12+1+60);
					u8 cntn = (lemmings[0] || lemmings[1]) && inform_client;
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
								:(lemmings[0]>lemmings[1]
									?" You won! "
									:"You lost! "),
							lemmings[0],
							usr_align_left,
							usr_align_right,
							usr,
							lemmings[1],
							won[0],
							usr_align_left,
							usr_align_right,
							usr,
							won[1],
							!cntn
								?"Press A or B to exit\n"
								:"Press A to continue \n"
								 "   or B to exit.    \n");
					tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);
					draw_menu_text(
							BOTTOM_SCREEN_BACK,
							menu_data,
							0,
							cntn?12:16,
							msg,
							0,
							1.0f);
					free(msg);
					msg = 0;
					begin_frame();
					copy_from_backbuffer(TOP_SCREEN);
					copy_from_backbuffer(BOTTOM_SCREEN);
					end_frame();
					// TODO: send results to client; 
					while (aptMainLoop()) {
						// wait for A pressed
						hidScanInput();
						if (cntn && ((hidKeysDown() & KEY_A) || (hidKeysDown() & KEY_TOUCH))) {
							break;
						}
						if ((hidKeysDown() & KEY_B) || (!cntn && ((hidKeysDown() & KEY_A) || (hidKeysDown() & KEY_TOUCH)))) {
							free(level);
							return MENU_ACTION_EXIT; // exit by user
						}
					}
					// at the moment: send an invalid packet to the client
					u8 nw_err = NW_ERROR; // at the moment: just crash!
					udsSendTo(
							UDS_BROADCAST_NETWORKNODEID,
							1,
							UDS_SENDFLAG_Default,
							&nw_err,
							1);

					if (!cntn) {
						break; // no player saved any lemming => end match
					}
					lvl++;
					lvl %= num_lvl;
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

int host_game(struct MainMenuData* menu_data, struct MainInGameData* main_data) {
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
		udsExit();
		return show_network_error(menu_data);
	}

	u8 appdata[0x02] = {/*'L', 'e', 'm', 'm', 'i', 'n', 'g', 's', ' ',
			'f', 'o', 'r', ' ', '3', 'D', 'S',*/
			NETWORK_MIN_PROTOCOL_VERSION, NETWORK_PROTOCOL_VERSION};
	ret = udsSetApplicationData(appdata, sizeof(appdata));
	if(R_FAILED(ret)) {
		udsDestroyNetwork();
		udsUnbind(&bindctx);
		udsExit();
		return show_network_error(menu_data);
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
			udsExit();
			return MENU_ACTION_START_MULTI_PLAYER;
		}
		if(udsWaitConnectionStatusEvent(false, false)) {
			// connection status changed. maybe a user tried to connect
			udsConnectionStatus constatus;
			ret = udsGetConnectionStatus(&constatus);
			if(R_FAILED(ret)) {
				udsDestroyNetwork();
				udsUnbind(&bindctx);
				udsExit();
				return show_network_error(menu_data);
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
								int res = on_client_connect(menu_data, main_data, &nodeinfo, i, &bindctx);
								udsSetNewConnectionsBlocked(false, true, false);
								if (res == MENU_EXIT_GAME) {
									udsDestroyNetwork();
									udsUnbind(&bindctx);
									udsExit();
									return MENU_EXIT_GAME;
								}
								if (res == MENU_ACTION_EXIT) {
									udsDestroyNetwork();
									udsUnbind(&bindctx);
									udsExit();
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
					udsExit();
					return show_network_error(menu_data);
				}
			}else if (constatus.total_nodes > 2) {
				udsDestroyNetwork();
				udsUnbind(&bindctx);
				udsExit();
				return show_network_error(menu_data);
			}
		}
		begin_frame();
		copy_from_backbuffer(TOP_SCREEN);
		copy_from_backbuffer(BOTTOM_SCREEN);
		end_frame();
	}
	udsDestroyNetwork();
	udsUnbind(&bindctx);
	udsExit();
	return MENU_EXIT_GAME;
}

int view_networks(struct MainMenuData* menu_data, struct MainInGameData* main_data) {
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
				for(attempt=0; attempt<10; attempt++) {
					ret = udsConnectNetwork(
							&networks[cur_selection].network,
							WLAN_PASS,
							strlen(WLAN_PASS),
							&bindctx,
							UDS_BROADCAST_NETWORKNODEID,
							UDSCONTYPE_Client,
							1,
							UDS_DEFAULT_RECVBUFSIZE);
					if(!R_FAILED(ret)) {
						kDown = 0;
						break;
					}
					hidScanInput();
					kDown = hidKeysDown();
					if (kDown & KEY_B) {
						attempt = 10; // give up
						kDown = 0;
						break;
					}
				}
				if(attempt<10) {
					// connected!
					// now wait for server to accept
					while (aptMainLoop()) {
						// get data
						size_t actual_size;
						u16 src_NetworkNodeID;
						ret = udsPullPacket(
								&bindctx,
								tmpbuf,
								UDS_DATAFRAME_MAXSIZE,
								&actual_size,
								&src_NetworkNodeID);
						hidScanInput();
						kDown = hidKeysDown();
						// test for connection to be closed!!! -> break up
						if (R_FAILED(ret) || (kDown & KEY_B)) {
							udsDisconnectNetwork();
							udsUnbind(&bindctx);
							kDown = 0;
							break; // lost connection
						}
						// test for acceptance message
						if (actual_size) {
							// got some packet. server seems to accept connection. jippieh!
							// parse NW_INITIALIZE packet
							if (((u8*)tmpbuf)[0] == NW_INITIALIZE
									&& actual_size == sizeof(struct NW_GameInit)) {
								// start game
								// confirm receivement of NW_INITIALIZE packet
								udsSendTo(UDS_HOST_NETWORKNODEID,1,UDS_SENDFLAG_Default,tmpbuf,1);
								// TODO: parse NW_INITIALIZE payload

								struct NW_GameInit* init = (struct NW_GameInit*)tmpbuf;
								u8 lemmings[2] = {
										init->lemmings_per_player[0],
										init->lemmings_per_player[1]
								};
								/* u8 num_players = init->num_players;
								u8 client_id = init->receiver_id; */
								// receive level
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
								struct Level* level = (struct Level*)malloc(sizeof(struct Level));
								if (level) {
									char usr[41+2*6];
									if (!get_aligned_username(usr, &networks[cur_selection].nodes[0])) {
										return 0;
									}
									char* usr_align_left = &usr[41];
									char* usr_align_right = &usr[41+6];
									do {
										int res = client_prepare_level(&bindctx, lemmings, level);
										if (res) {
											// start game; ...
											u8 lemmings[2];
											u16 won[2];
											if (!client_run_level(
													&bindctx,
													level,
													0,
													lemmings,
													won,
													menu_data,
													main_data)) {
												free_objects(level->object_types);
												break;
											}
											free_objects(level->object_types);
					char* msg = (char*)malloc((20+1)*12+1+60);
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
							" Wait for server or \n"
							"  press B to exit.  \n",
							lemmings[0]==lemmings[1]
								?"   Tie    "
								:(lemmings[1]>lemmings[0]
									?" You won! "
									:"You lost! "),
							lemmings[1],
							usr_align_left,
							usr_align_right,
							usr,
							lemmings[0],
							won[1],
							usr_align_left,
							usr_align_right,
							usr,
							won[0]);
					tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);
					draw_menu_text(
							BOTTOM_SCREEN_BACK,
							menu_data,
							0,
							12,
							msg,
							0,
							1.0f);
					free(msg);
					msg = 0;
					begin_frame();
					copy_from_backbuffer(TOP_SCREEN);
					copy_from_backbuffer(BOTTOM_SCREEN);
					end_frame();
					// TODO: send results to client; 
					while (aptMainLoop()) {
						// wait for A pressed
						hidScanInput();
						if ((hidKeysDown() & KEY_B)) {
							free(level);
							level = 0;
							break;
						}
						union {
							u8 msg;
							struct NW_Level_Result res;
							struct NW_GameInit init;
						} tmp;
						size_t actual_size = 0;
						u16 src_NetworkNodeID;
						ret = udsPullPacket(
								&bindctx,
								&tmp,
								sizeof(struct NW_Level_Result),
								&actual_size,
								&src_NetworkNodeID);
						if (R_FAILED(ret)) {
							// lost connection
								free(level);
								level = 0;
								break;
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
							break;
						}
					}
										}
									}while(aptMainLoop() && level);
									if (level) {
										// TODO: clean up;
										free(level);
									}
								}
							}
							// done. -> exit connection
							udsDisconnectNetwork();
							udsUnbind(&bindctx);
							kDown = 0;
							break;
							
						}
						begin_frame();
						copy_from_backbuffer(TOP_SCREEN);
						copy_from_backbuffer(BOTTOM_SCREEN);
						end_frame();
					}
				}
			}
		}
		if (kDown & KEY_B) {
			// exit menu without saving
			udsExit();
			free(tmpbuf);
			return MENU_ACTION_EXIT;
		}
		if (kDown & KEY_START) {
			// TODO: create own network
			// exit menu
			// udsExit();
			free(tmpbuf);
			return MENU_HOST_GAME;
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
