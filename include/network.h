#ifndef NETWORK_H
#define NETWORK_H
#include <3ds.h>
#include "level.h"
#include "main_data.h"
#include "control.h"

// network messages
#define NW_ERROR              0 // communication error (maybe just cancel connection...; implement handling later)
#define NW_INITIALIZE         1 // confirm that the server WILL start the game; params: num of (active) players, num of lemmings per player, player id of receiver
#define NW_LEVEL_INFO         2 // receive level data (first packet: Info. then: Chunks)
#define NW_LEVEL_DATA_CHUNK   3 // send back (with zero length) to confirm receivement
#define NW_LEVEL_SENT         4 // entire level has been sent
#define NW_RUN_FRAME          5 // step a given number of frames without action (info from server)
#define NW_USER_INPUT         6 // screen positions and mouse positions of all players (to inform other); own position should be sent by clients with this message, too
#define NW_LVLRESULT          7 // indicate end of level and send results (lemmings saved by each player)
#define NW_READY_TO_PROCEED   8 // client informs server that he is ready to start the next level (user has read the result message)

struct NW_GameInit {
	u8 msg_type;
	u8 num_players;
	u8 lemmings_per_player[2];
	u8 receiver_id;
};

struct NW_LevelData_Info {
	u8 msg_type;
	u8 swap_exits;
	u32 vgagr_s0_size;
	u32 vgagr_s1_size;
	u32 vgaspec_size;
	u32 ingame_basis_palette[7];
	u8 ground_data[1056];
};

struct NW_LevelData_Chunk {
	u8 msg_type;
	u8 type; // 0: level; 1: vgagr_s0; 2: vgagr_s1; 3: vgaspec
	u32 offset;
	u16 length;
	u8 data[0];
};

struct NW_User_Input {
	u8 msg_type;
	u8 player_id;
	u16 x_pos;
	u32 frame_id; // frame id the user made the input (only set when sent by server)
	u32 input_id; // id of this operation; starting with 1; no id must be missed
	u8 num_actions;
	struct ActionQueue action_queue[MAX_ACTION_QUEUE_SIZE];
};

struct NW_Run_Frame {
	u8 msg_type;
	u32 frame_id; // current frame id
	u32 required_input_id; // this frame must only be rendered if the required_input_id has been received already
};

struct NW_Level_Result {
	u8 msg_type;
	u8 lemmings_saved[2];
	u16 won[2];
};

int connection_alive();

int server_prepare_level(
		udsBindContext* bindctx,
		const u8* lemmings, // number of lemmings the players start with
		u8 game_id,
		u8 level_id,
		struct Level* output);
int client_prepare_level(
		udsBindContext* bindctx,
		const u8* lemmings, // number of lemmings the players start with
		struct Level* output);

int server_run_level(
		udsBindContext* bindctx,
		struct Level* level,
		const char* level_id,
		u8* lemmings, // number of lemmings the players have rescued
		struct MainMenuData* menu_data,
		struct MainInGameData* main_data);
int client_run_level(
		udsBindContext* bindctx,
		struct Level* level,
		const char* level_id,
		u8* lemmings,
		u16* won,
		struct MainMenuData* menu_data,
		struct MainInGameData* main_data);

int server_send_result(
		udsBindContext* bindctx,
		u8 lemmings[2],
		u16 won[2]);
#endif
