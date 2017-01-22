#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "network_menu.h"
#include "view_networks.h"
#include "network.h"
#include "main.h"

#define STACKSIZE (4 * 1024)

volatile bool run_scanning_thread = true;
volatile int scan_status = 0;
Handle scan_mutex;
volatile size_t total_networks = 0;
udsNetworkScanInfo* volatile networks = 0;
void memcpy_volatile(void* volatile d, void* volatile t, u32 len) {
	while (len) {
		*((char* volatile)d) = *((char* volatile)t);
		d = (char* volatile)d + 1;
		t = (char* volatile)t + 1;
		len--;
	}
}

void scan_thread_main(void *arg) {
	size_t tmpbuf_size = (UDS_DATAFRAME_MAXSIZE>0x4000?UDS_DATAFRAME_MAXSIZE:0x4000);
	void* tmpbuf = malloc(tmpbuf_size);
	size_t num_networks = 0;
	udsNetworkScanInfo* networks_scan = 0;
	if (!tmpbuf) {
		scan_status = -1;
		return;
	}
	while (run_scanning_thread)
	{
		svcSleepThread(1000000ULL * 100); // sleep 100ms
		Result res = svcWaitSynchronization(scan_mutex, 1000000ULL * 250); // wait 250ms
		if (SUCCESS(res)) {
			res = udsScanBeacons(
				tmpbuf,
				tmpbuf_size,
				&networks_scan,
				&num_networks,
				WLAN_ID,
				0,
				NULL,
				false);
			scan_status = res;
			if (R_SUCCEEDED(res)) {
				if (networks_scan) {
					total_networks = num_networks;
					if (total_networks > 4) {
						total_networks = 4;
					}
					if (total_networks) {
						memcpy_volatile(
								networks,
								networks_scan,
								total_networks * sizeof(udsNetworkScanInfo));
						free(networks_scan);
						networks_scan = 0;
					}
				}else{
					total_networks = 0;
				}
			}else{
				run_scanning_thread = false;
			}
			svcReleaseMutex(scan_mutex);
		}
	}
	free(tmpbuf);
}

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
	char* msg_ptr = msg;
	DRAW_MENU("\n Choose a Game\n\n");
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

int network_menu(
		struct SaveGame* savegame,
		u8 num_levels[2],
		char* level_names,
		struct MainMenuData* menu_data, struct MainInGameData* main_data) {
	// init
	total_networks = 0;
	networks = (udsNetworkScanInfo*)malloc(4 * sizeof(udsNetworkScanInfo));
	run_scanning_thread = true;
	scan_status = 0;
	if (!networks) {
		return MENU_ERROR;
	}
	if (R_FAILED(svcCreateMutex(&scan_mutex, false))) {
		free(networks);
		networks = 0;
		return MENU_ERROR;
	}
	Result ret = udsInit(0x3000, NULL);
	if(R_FAILED(ret)) {
		// show WLAN error message
		svcCloseHandle(scan_mutex);
		scan_mutex = 0;
		free(networks);
		networks = 0;
		return show_network_error(menu_data);
	}

	size_t num_networks = 0; // local copy
	udsNetworkScanInfo networks_scan[4]; // local copy
	s32 prio = 0;
	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
	Thread thread = threadCreate(scan_thread_main, (void*)(250), STACKSIZE, prio-1, -2, false);

	u8 cur_selection = 0;
	u8 network_alive[4] = {0, 0, 0, 0};
	u32 kDown;
	u32 kHeld;
	int dwn = 0;
	int up = 0;

	while (aptMainLoop()) {
		draw_network_view(
				cur_selection,
				num_networks,
				networks_scan,
				menu_data,
				network_alive);

		begin_frame();
		copy_from_backbuffer(TOP_SCREEN);
		copy_from_backbuffer(BOTTOM_SCREEN);
		end_frame();

		hidScanInput();
		kDown = hidKeysDown();
		kHeld = hidKeysHeld();

		if (!((kDown | kHeld) & KEY_DOWN)) {
			dwn = 0;
		}
		if (!((kDown | kHeld) & KEY_UP)) {
			up = 0;
		}

		if ((kDown & KEY_DOWN) || dwn >= 15) {
			if (dwn) {
				dwn = 10;
			}else{
				dwn = 1;
			}
			if (cur_selection < 5) {
				cur_selection++;
			}else if (kDown & KEY_DOWN) {
				cur_selection = 0;
			}
		}
		if ((kDown & KEY_UP) || up >= 15) {
			if (up) {
				up = 10;
			}else{
				up = 1;
			}
			if (cur_selection) {
				cur_selection--;
			}else if (kDown & KEY_UP) {
				cur_selection = 5;
			}
		}
		if (kHeld & KEY_DOWN) {
			dwn++;
		}
		if (kHeld & KEY_UP) {
			up++;
		}
		if (kDown & KEY_A) {
			if (cur_selection == 4) {
				kDown |= KEY_START;
			}else if (cur_selection == 5){
				kDown |= KEY_B;
			}else if (cur_selection < 4 && network_alive[cur_selection]){
				// stop scanning
				tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);
				draw_menu_text(
						BOTTOM_SCREEN_BACK,
						menu_data,
						0,
						72,
						"   Connecting...    \n",
						0,
						1.0f);
				begin_frame();
				copy_from_backbuffer(TOP_SCREEN);
				copy_from_backbuffer(BOTTOM_SCREEN);
				end_frame();
				Result res = svcWaitSynchronization(scan_mutex, 1000000ULL * 2000); // wait up to 2s
				if (SUCCESS(res) && cur_selection < num_networks) {
					connect_to_network(&networks_scan[cur_selection], menu_data, main_data);
					num_networks = 0;
					total_networks = 0;
					kDown = 0;
					// continue scanning
					svcReleaseMutex(scan_mutex);
				}else{
					// canot stop scanning!
				}
			}
		}
		if (kDown & KEY_B) {
			// exit menu
			run_scanning_thread = false;
			tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);
			draw_menu_text(
					BOTTOM_SCREEN_BACK,
					menu_data,
					0,
					96,
					"   Please wait...   \n",
					0,
					1.0f);
			begin_frame();
			copy_from_backbuffer(TOP_SCREEN);
			copy_from_backbuffer(BOTTOM_SCREEN);
			end_frame();
			threadJoin(thread, U64_MAX);
			threadFree(thread);
			svcCloseHandle(scan_mutex);
			scan_mutex = 0;
			free(networks);
			networks = 0;
			udsExit();
			return MENU_ACTION_EXIT;
		}
		if (kDown & KEY_START) {
			// create own network
			// stop scanning
			tile_menu_background(BOTTOM_SCREEN_BACK, menu_data);
			draw_menu_text(
					BOTTOM_SCREEN_BACK,
					menu_data,
					0,
					96,
					"   Please wait...   \n",
					0,
					1.0f);
			begin_frame();
			copy_from_backbuffer(TOP_SCREEN);
			copy_from_backbuffer(BOTTOM_SCREEN);
			end_frame();
			Result res = svcWaitSynchronization(scan_mutex, 1000000ULL * 2000); // wait up to 2s
			if (SUCCESS(res)) {
				int res = host_game(savegame, num_levels, level_names, menu_data, main_data);
				num_networks = 0;
				total_networks = 0;
				// continue scanning
				svcReleaseMutex(scan_mutex);
				if (res == MENU_EXIT_GAME) {
					break;
				}
				kDown = 0;
			}else{
					// canot stop scanning!
				}
		}

		Result res = svcWaitSynchronization(scan_mutex, 1000000ULL * 5); // wait up to 5ms
		if (SUCCESS(res)) {
			if (R_FAILED(scan_status)) {
				run_scanning_thread = false;
				svcReleaseMutex(scan_mutex);
				threadJoin(thread, U64_MAX);
				threadFree(thread);
				svcCloseHandle(scan_mutex);
				scan_mutex = 0;
				free(networks);
				networks = 0;
				udsExit();
				return show_network_error(menu_data);
			}
			// update local network info
			if (networks) {
				num_networks = total_networks;
				if (num_networks > 4) {
					num_networks = 4;
				}
				if (num_networks) {
					memcpy_volatile(
							networks_scan,
							networks,
							num_networks * sizeof(udsNetworkScanInfo));
				}
			}else{
				num_networks = 0;
			}
			svcReleaseMutex(scan_mutex);
		}
	}
	run_scanning_thread = false;
	threadJoin(thread, U64_MAX);
	threadFree(thread);
	svcCloseHandle(scan_mutex);
	scan_mutex = 0;
	free(networks);
	networks = 0;
	udsExit();
	return MENU_EXIT_GAME;
}
