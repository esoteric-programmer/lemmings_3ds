#ifndef NETWORK_H
#define NETWORK_H
#include <3ds.h>
#include "control.h"

#define WLAN_ID 0x40F91730
#define WLAN_PASS "Lemmings for 3DS rulez"
#define NETWORK_PROTOCOL_VERSION 2
#define NETWORK_MIN_PROTOCOL_VERSION 2

// network messages
#define NW_ERROR              0 // communication error (maybe just cancel connection...; implement handling later)
#define NW_PROTOCOL_VERSION  42
#define NW_INITIALIZE         1 // confirm that the server WILL start the game; params: num of (active) players, num of lemmings per player, player id of receiver
#define NW_LEVEL_INFO         2 // receive level data (first packet: Info. then: Chunks)
#define NW_LEVEL_DATA_CHUNK   3 // send back (with zero length) to confirm receivement
#define NW_LEVEL_SENT         4 // entire level has been sent
#define NW_RUN_FRAME          5 // step a given number of frames without action (info from server)
#define NW_USER_INPUT         6 // sent from server to client: screen positions and skill assignments of all players (to inform them);
#define NW_CLIENT_ACTION      7 // client sends actions to server to ask him to apply them (use NW_User_Input struct, maybe do not use all fields)
#define NW_LVLRESULT          8 // indicate end of level and send results (lemmings saved by each player)
#define NW_READY_TO_PROCEED   9 // client informs server that he is ready to start the next level (user has read the result message)

#define NETWORK_SUCCESS                 0
#define NETWORK_ERROR_WLAN_LOST         1
#define NETWORK_ERROR_CONNECTION_LOST   2
#define NETWORK_ERROR_PROTOCOL_ERROR    3
#define NETWORK_ERROR_READ_LEVEL_ERROR  4
#define NETWORK_ERROR_PARSE_LEVEL_ERROR 5
#define NETWORK_ERROR_OUT_OF_MEM        6
#define NETWORK_ERROR_ASYNCHRONOUS      7
#define NETWORK_ERROR_NO_2P_LEVELS      8
#define NETWORK_ERROR_OTHER             9

// sent by client on connect (until server confirms receivement)
struct NW_ProtocolVersion {
	u8 msg_type;
	u8 version_major;
	u8 version_minor;
};

struct NW_GameInit {
	u8 msg_type;
	u8 num_players;
	u8 lemmings_per_player[2];
	u8 receiver_id;
	u8 lvl_id;
	u8 game_id;
	u8 glitch_direct_drop;
	u8 glitch_shrugger;
	u8 timeout; // 2p time limit settings
	u8 inspect_level; // inspect level before it starts?
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
#endif
