#include <string.h>
#include <malloc.h>
#include "network.h"
#include "gamespecific_2p.h"
#include "import_level.h"
#include "audio.h"
#include "draw.h" // debug
#include "control.h"
#include "ingame.h"
#include "lemming.h"
#include "main.h" // for was_suspended(); TODO: disable sleep mode while network game is active

// server:
// read level file
// send via network
// parse level file
// start game

#define CHUNK_SIZE (UDS_DATAFRAME_MAXSIZE - sizeof(struct NW_LevelData_Chunk))

// TODO: tidy up (introduce subroutines to free memory); TODO: catch all errors

int connection_alive() {
	udsConnectionStatus constatus;
	Result ret = udsGetConnectionStatus(&constatus);
	if(R_FAILED(ret) || constatus.total_nodes != 2) {
		return 0; // lost connection
	}
	return 1;
}

int send_until_confirmed(void* msg, u16 length, udsBindContext* bindctx) {
	if (!msg || !length || !bindctx || length > UDS_DATAFRAME_MAXSIZE) {
		return 0;
	}
	u8 buf[UDS_DATAFRAME_MAXSIZE];
	do {
		Result ret = udsSendTo(
				UDS_BROADCAST_NETWORKNODEID,
				1,
				UDS_SENDFLAG_Default,
				msg,
				length);
		if(UDS_CHECK_SENDTO_FATALERROR(ret)) {
			return 0;
		}
		size_t actual_size = 0;
		u16 src_NetworkNodeID;
		ret = udsPullPacket(
				bindctx,
				buf,
				UDS_DATAFRAME_MAXSIZE,
				&actual_size,
				&src_NetworkNodeID);
		if (R_FAILED(ret)) {
			// lost connection
			return 0;
		}
		if (actual_size) {
			if (buf[0] == ((u8*)msg)[0]) {
				// client received message
				return 1;
			}
			if (buf[0] == NW_ERROR) {
				// client sent error
				return 0;
			}
		}
		if(!connection_alive()) {
			return 0; // lost connection to client
		}
		// wait for client confirmation
		if (!aptMainLoop()) {
			return 0;
		}
	}while(1);
}

int server_prepare_level(
		udsBindContext* bindctx,
		const u8* lemmings,
		u8 game_id,
		u8 level_id,
		struct Level* output){
	if (!lemmings || !bindctx || !output) {
		return 0;
	}
	struct NW_LevelData_Chunk* chunk = (struct NW_LevelData_Chunk*)malloc(
			sizeof(struct NW_LevelData_Chunk) + CHUNK_SIZE);
	if (!chunk) {
		return 0;
	}
	Result ret;

	struct NW_GameInit gameinit;
	gameinit.msg_type = NW_INITIALIZE;
	gameinit.num_players = 1;
	gameinit.lemmings_per_player[0] = lemmings[0];
	gameinit.lemmings_per_player[1] = lemmings[1];
	gameinit.receiver_id = 1; // receiver is second player
	// send gameinit
	if (!send_until_confirmed(&gameinit, sizeof(gameinit), bindctx)) {
		free(chunk);
		return 0;
	}

	u8 swap_exits = 0;
	if (import_2p[game_id].swap_exit && level_id < import_2p[game_id].size_swap_exit) {
		swap_exits = import_2p[game_id].swap_exit[level_id];
	}

	struct NW_LevelData_Info* leveldata_info =
			(struct NW_LevelData_Info*)
			malloc(sizeof(struct NW_LevelData_Info));
	u8* level = (u8*)malloc(2048);
	leveldata_info->msg_type = NW_LEVEL_INFO;
	struct Data* vgagr_s0 = 0;
	struct Data* vgagr_s1 = 0;
	struct Data* vgaspec = 0;
	leveldata_info->vgagr_s0_size = 0;
	leveldata_info->vgagr_s1_size = 0;
	leveldata_info->vgaspec_size = 0;
	leveldata_info->swap_exits = swap_exits;

	char levelfilename[64];
	sprintf(levelfilename,"%s/%s/%02u.lvl",
				PATH_ROOT,import_2p[game_id].level_path,level_id+1);
	char ressource_path[64];
	sprintf(ressource_path,"%s/%s",
				PATH_ROOT,import_2p[game_id].ressource_path);
	int res = read_level_file(
			levelfilename,
			ressource_path,
			level,
			leveldata_info->ground_data,
			&vgagr_s0,
			&vgagr_s1,
			&vgaspec);
	if (!res) {
		free(chunk);
		free(leveldata_info);
		return 0;
	}
	memcpy(leveldata_info->ingame_basis_palette,import[1].ingame_palette,7*sizeof(u32));
	if (vgagr_s0) {
		leveldata_info->vgagr_s0_size = vgagr_s0->size;
	}
	if (vgagr_s1) {
		leveldata_info->vgagr_s1_size = vgagr_s1->size;
	}
	if (vgaspec) {
		leveldata_info->vgaspec_size = vgaspec->size;
	}
	// send leveldata_info
	if (!send_until_confirmed(leveldata_info, sizeof(struct NW_LevelData_Info), bindctx)) {
		free(chunk);
		if (vgagr_s0) {
			free(vgagr_s0);
		}
		if (vgagr_s1) {
			free(vgagr_s1);
		}
		if (vgaspec) {
			free(vgaspec);
		}
		free(leveldata_info);
		return 0;
	}

	struct Data* chunks[] = {
			vgagr_s0,
			vgagr_s1,
			vgaspec
	};
	u8* chunks_received = 0;
	u32 chunk_received_offsets[4] = {0, 2, 2+((chunks[0]?chunks[0]->size:0) + CHUNK_SIZE - 1)/CHUNK_SIZE, 0};
	chunk_received_offsets[3] = chunk_received_offsets[2] + ((chunks[1]?chunks[1]->size:0) + CHUNK_SIZE - 1)/CHUNK_SIZE;
	u32 chunk_num = chunk_received_offsets[3] + ((chunks[2]?chunks[2]->size:0) + CHUNK_SIZE - 1)/CHUNK_SIZE;
	chunks_received = (u8*)malloc(chunk_num);
	memset(chunks_received, 0, chunk_num);
	while (chunk_num) {
		u8 i;
		for (i=0;i<2;i++) {
			if (chunks_received[i]) {
				continue;
			}
			chunk->msg_type = NW_LEVEL_DATA_CHUNK;
			chunk->type = 0;
			chunk->offset = 1024*i;
			chunk->length = 1024;
			memcpy(chunk->data, &level[1024*i], 1024);
			ret = udsSendTo(
					UDS_BROADCAST_NETWORKNODEID,
					1,
					UDS_SENDFLAG_Default,
					chunk,
					sizeof(struct NW_LevelData_Chunk) + 1024);
			if(UDS_CHECK_SENDTO_FATALERROR(ret)) {
				free(chunk);
				if (vgagr_s0) {
					free(vgagr_s0);
				}
				if (vgagr_s1) {
					free(vgagr_s1);
				}
				if (vgaspec) {
					free(vgaspec);
				}
				free(leveldata_info);
				free(chunks_received);
				return 0;
			}
		}

		u32 offset = 0;
		for (i=0;i<3;i++) {
			if (!chunks[i]) {
				continue;
			}
			for (offset = 0;
					offset < chunks[i]->size && chunk_num && connection_alive();
					offset+=CHUNK_SIZE) {

				u32 idx = chunk_received_offsets[i+1] + offset/CHUNK_SIZE;
				if (chunks_received[idx]) {
					continue;
				}

				// test whether some chunks have been confirmed
				do {
					size_t actual_size = 0;
					u16 src_NetworkNodeID;
					ret = udsPullPacket(
							bindctx,
							chunk,
							sizeof(struct NW_LevelData_Chunk),
							&actual_size,
							&src_NetworkNodeID);
					if (R_FAILED(ret)) {
						// lost connection
						free(chunk);
						return 0;
					}
					if (actual_size == sizeof(struct NW_LevelData_Chunk)) {
						if (chunk->msg_type == NW_LEVEL_DATA_CHUNK && chunk->type <= 3) {
							u8 valid = 0;
							if (chunk->type) {
								if (chunk->offset < chunks[chunk->type - 1]->size) {
									valid = 1;
								}
							}else{
								if (chunk->offset < 2048) {
								 valid = 1;
								}
							}
							if (valid) {
								// some chunk has been confirmed
								idx = chunk_received_offsets[chunk->type] + (chunk->type==0?(chunk->offset/1024):(chunk->offset/CHUNK_SIZE));
								if (!chunks_received[idx]) {
									chunks_received[idx] = 1;
									chunk_num--;
								}
							}
						}
					}else{
						break;
					}
				}while(aptMainLoop() && chunk_num);

				chunk->msg_type = NW_LEVEL_DATA_CHUNK;
				chunk->type = i+1;
				chunk->offset = offset;
				chunk->length = CHUNK_SIZE;
				if (chunks[i]->size < offset + chunk->length) {
					chunk->length = chunks[i]->size - offset;
				}
				memcpy(chunk->data, &chunks[i]->data[offset], chunk->length);
				// send chunk
				ret = udsSendTo(
						UDS_BROADCAST_NETWORKNODEID,
						1,
						UDS_SENDFLAG_Default,
						chunk,
						sizeof(struct NW_LevelData_Chunk) + chunk->length);
				if(UDS_CHECK_SENDTO_FATALERROR(ret)) {
					free(chunk);
					if (vgagr_s0) {
						free(vgagr_s0);
					}
					if (vgagr_s1) {
						free(vgagr_s1);
					}
					if (vgaspec) {
						free(vgaspec);
					}
					free(leveldata_info);
					free(chunks_received);
					return 0;
				}
			}
			if(!connection_alive()) {
				break; // lost connection to client
			}
		}
		if (!aptMainLoop() || !connection_alive()) {
			free(chunk);
			if (vgagr_s0) {
				free(vgagr_s0);
			}
			if (vgagr_s1) {
				free(vgagr_s1);
			}
			if (vgaspec) {
				free(vgaspec);
			}
			free(leveldata_info);
			free(chunks_received);
			return 0;
		}
	}
	free(chunks_received);
	chunks_received = 0;
	// tell client that he has received everything and should start parsing the level,
	// wait until client confirms receivement
	u8 level_sent = NW_LEVEL_SENT;
	if (!send_until_confirmed(&level_sent, 1, bindctx)) {
		free(chunk);
		if (vgagr_s0) {
			free(vgagr_s0);
		}
		if (vgagr_s1) {
			free(vgagr_s1);
		}
		if (vgaspec) {
			free(vgaspec);
		}
		free(leveldata_info);
		return 0;
	}
	free(chunk);
	chunk = 0;
	// parse level
	res = parse_level(
		level,
		leveldata_info->ground_data,
		vgagr_s0,
		vgagr_s1,
		vgaspec,
		leveldata_info->ingame_basis_palette,
		swap_exits,
		2, // TODO: number of players?
		output);
	if (vgagr_s0) {
		free(vgagr_s0);
		vgagr_s0 = 0;
	}
	if (vgagr_s1) {
		free(vgagr_s1);
		vgagr_s1 = 0;
	}
	if (vgaspec) {
		free(vgaspec);
		vgaspec = 0;
	}
	free(leveldata_info);
	if (!res) {
		return 0;
	}
	prepare_music(1, 0);

	if(!connection_alive()) {
		free_objects(output->object_types);
		return 0; // lost connection to client
	}
	output->player[0].max_lemmings = lemmings[0];
	output->player[1].max_lemmings = lemmings[1];
	return 1;
}

// client:
// retrieve level file
// parse level file
// start game

// TODO: make sure that client does only accept messages from server
// TODO: also in client_run_level method

union RecBuf{
	    u8 buf[UDS_DATAFRAME_MAXSIZE];
    	struct NW_GameInit gi;
    	struct NW_LevelData_Info li;
    	struct NW_LevelData_Chunk lc;
	};

int client_prepare_level(
		udsBindContext* bindctx,
		const u8* lem,
		struct Level* output){
	if (!lem || !bindctx || !output) {
		return 0;
	}
	u8 lemmings[2] = {lem[0], lem[1]};
	u8 parsed = 0;
	u8 swap_exits = 0;
	Result ret;
	union RecBuf* rec_buf = (union RecBuf*)malloc(sizeof(union RecBuf));;
	size_t actual_size;
	u16 src_NetworkNodeID;

	u8* level = (u8*)malloc(2048);
	u8* ground_data = (u8*)malloc(1056);
	struct Data* vgagr_s0 = 0;
	struct Data* vgagr_s1 = 0;
	struct Data* vgaspec = 0;
	struct Data** chunks[] = {
			&vgagr_s0,
			&vgagr_s1,
			&vgaspec
	};
	u32 ingame_basis_palette[7];

	do{
		actual_size = 0;
		ret = udsPullPacket(
				bindctx,
				rec_buf->buf,
				sizeof(union RecBuf),
				&actual_size,
				&src_NetworkNodeID);
		// test for connection to be closed!!! -> break up
		if (R_FAILED(ret)) {
			free(level);
			free(ground_data);
			free(rec_buf);
			return 0; // lost connection
		}
		// test for acceptance message
		if (actual_size) {
			// got some packet. server seems to accept connection. jippieh!
			switch (rec_buf->buf[0]) {
				case NW_INITIALIZE:
					// parse NW_INITIALIZE payload
					lemmings[0] = rec_buf->gi.lemmings_per_player[0];
					lemmings[1] = rec_buf->gi.lemmings_per_player[1];
					// just send back this message type
					udsSendTo(UDS_HOST_NETWORKNODEID,1,UDS_SENDFLAG_Default,rec_buf->buf,1);
					break;
				case NW_LEVEL_INFO:
					// confirm receivement
					udsSendTo(UDS_HOST_NETWORKNODEID,1,UDS_SENDFLAG_Default,rec_buf->buf,1);
					// initialize new level
					if (parsed) {
						free_objects(output->object_types);
					}
					parsed = 0;
					if (vgagr_s0) {
						if (vgagr_s0->size != rec_buf->li.vgagr_s0_size) {
							free(vgagr_s0);
							vgagr_s0 = 0;
						}
					}
					if (vgagr_s1) {
						if (vgagr_s1->size != rec_buf->li.vgagr_s1_size) {
							free(vgagr_s1);
							vgagr_s1 = 0;
						}
					}
					if (vgaspec) {
						if (vgaspec->size != rec_buf->li.vgaspec_size) {
							free(vgaspec);
							vgaspec = 0;
						}
					}
					if (actual_size != sizeof(struct NW_LevelData_Info)) {
						// invalid message: maybe send error message, then break up connection
						free(level);
						free(ground_data);
						free(rec_buf);
						return 0;
					}
					if (rec_buf->li.vgagr_s0_size && !vgagr_s0) {
						vgagr_s0 = (struct Data*)malloc(
								sizeof(struct Data)
								+ rec_buf->li.vgagr_s0_size);
						vgagr_s0->size = rec_buf->li.vgagr_s0_size;
					}
					if (rec_buf->li.vgagr_s1_size && !vgagr_s1) {
						vgagr_s1 = (struct Data*)malloc(
								sizeof(struct Data)
								+ rec_buf->li.vgagr_s1_size);
						vgagr_s1->size = rec_buf->li.vgagr_s1_size;
					}
					if (rec_buf->li.vgaspec_size && !vgaspec) {
						vgaspec = (struct Data*)malloc(
								sizeof(struct Data)
								+ rec_buf->li.vgaspec_size);
						vgaspec->size = rec_buf->li.vgaspec_size;
					}
					memcpy(
							ingame_basis_palette,
							rec_buf->li.ingame_basis_palette,
							7*sizeof(u32));
					memcpy(
							ground_data,
							rec_buf->li.ground_data,
							1056);
					swap_exits = rec_buf->li.swap_exits;
					break;
				case NW_LEVEL_DATA_CHUNK:
					if (parsed) {
						free_objects(output->object_types);
					}
					parsed = 0;
					// get data chunk of level
					if (actual_size < sizeof(struct NW_LevelData_Chunk)) {
						// invalid message: maybe send error message, then break up connection
						// TODO: free vgagr_s0, vgagr_s1, vgaspec
						free(level);
						free(ground_data);
						free(rec_buf);
						return 0;
					}
					if (actual_size !=
								sizeof(struct NW_LevelData_Chunk)
								+ rec_buf->lc.length
							|| rec_buf->lc.type > 3) {
						// invalid message: maybe send error message, then break up connection
						// TODO: free vgagr_s0, vgagr_s1, vgaspec
						free(level);
						free(ground_data);
						free(rec_buf);
						return 0;
					}
					if (rec_buf->lc.type) {
						struct Data* chk = *chunks[rec_buf->lc.type-1];
						if (!chk) {
							// invalid message: chunk received before size of whole field is known
							// TODO: free vgagr_s0, vgagr_s1, vgaspec
							free(level);
							free(ground_data);
							free(rec_buf);
							return 0;
						}
						if (chk->size < rec_buf->lc.offset + rec_buf->lc.length) {
							// invalid message: maybe send error message, then break up connection
							// TODO: free vgagr_s0, vgagr_s1, vgaspec
							free(level);
							free(ground_data);
							free(rec_buf);
							return 0;
						}
						memcpy(&chk->data[rec_buf->lc.offset], rec_buf->lc.data, rec_buf->lc.length);
					}else{
						if (rec_buf->lc.offset + rec_buf->lc.length > 2048) {
							// invalid message: maybe send error message, then break up connection
							// TODO: free vgagr_s0, vgagr_s1, vgaspec
							free(level);
							free(ground_data);
							free(rec_buf);
							return 0;
						}
						memcpy(&level[rec_buf->lc.offset], rec_buf->lc.data, rec_buf->lc.length);
					}
					rec_buf->lc.length = 0;
					udsSendTo(UDS_HOST_NETWORKNODEID,1,UDS_SENDFLAG_Default,rec_buf->buf,sizeof(struct NW_LevelData_Chunk));
					break;
				case NW_LEVEL_SENT:
					{
						if (parsed) {
							udsSendTo(UDS_HOST_NETWORKNODEID,1,UDS_SENDFLAG_Default,rec_buf->buf,1);
							break;
						}
						// parse level
						int res = parse_level(
							level,
							ground_data,
							vgagr_s0,
							vgagr_s1,
							vgaspec,
							ingame_basis_palette,
							swap_exits,
							2, // TODO: number of players?
							output);

						if (vgagr_s0) {
							free(vgagr_s0);
							vgagr_s0 = 0;
						}
						if (vgagr_s1) {
							free(vgagr_s1);
							vgagr_s1 = 0;
						}
						if (vgaspec) {
							free(vgaspec);
							vgaspec = 0;
						}
						if (!res) {
							// error parsing received level
							// maybe send error message, then break up connection
							free(level);
							free(ground_data);
							free(rec_buf);
							return 0;
						}
						prepare_music(1, 0);
						parsed = 1;
						// send NW_LEVEL_SENT message back to server
						// to inform him that the level has been parsed
						udsSendTo(UDS_HOST_NETWORKNODEID,1,UDS_SENDFLAG_Default,rec_buf->buf,1);
						if(UDS_CHECK_SENDTO_FATALERROR(ret)) {
							// error sending message, maybe lost connection
							// TODO: tidy up
							free_objects(output->object_types);
							free(level);
							free(ground_data);
							free(rec_buf);
							return 0;
						}
					}
					break;
				case NW_RUN_FRAME:
					if (!parsed) {
						// invalid message: maybe send error message, then break up connection
						free(level);
						free(ground_data);
						free(rec_buf);
						return 0;
					}
					// game has been started!
					free(level);
					free(ground_data);
					free(rec_buf);
					output->player[0].max_lemmings = lemmings[0];
					output->player[1].max_lemmings = lemmings[1];
					return 1;
					break;
				default:
					// invalid message: maybe send error message, then break up connection
					if (parsed) {
						free_objects(output->object_types);
					}
					free(level);
					free(ground_data);
					free(rec_buf);
					return 0;
			}
		}
	}while(aptMainLoop() && connection_alive());
	if (parsed) {
		free_objects(output->object_types);
	}
	free(level);
	free(ground_data);
	free(rec_buf);
	return 0;
}

struct NW_UI_Queue {
	struct NW_User_Input* data;
	struct NW_UI_Queue* next;
};

void nw_ui_queue_add_copy(
		struct NW_User_Input* data,
		struct NW_UI_Queue** first,
		struct NW_UI_Queue** last) {
	if (!first || !last || !data) {
		return; // error
	}
	if (!*first) {
		*first = (struct NW_UI_Queue*)malloc(sizeof(struct NW_UI_Queue));
		*last = *first;
	}else if (!*last){
		return; // error
	}else{
		struct NW_UI_Queue* tmp = *last;
		*last = (struct NW_UI_Queue*)malloc(sizeof(struct NW_UI_Queue));
		tmp->next = *last;
	}
	(*last)->next = 0;
	(*last)->data = (struct NW_User_Input*)malloc(sizeof(struct NW_User_Input));
	memcpy((*last)->data, data, sizeof(struct NW_User_Input));
}

void nw_ui_queue_add(
		u8 player_id,
		struct InputState* io_state,
		u32* input_id,
		u32 current_frame,
		struct NW_UI_Queue** first,
		struct NW_UI_Queue** last) {
	if (!first || !last || !io_state || !input_id) {
		return; // error
	}
	if (!*first) {
		*first = (struct NW_UI_Queue*)malloc(sizeof(struct NW_UI_Queue));
		*last = *first;
	}else if (!*last){
		return; // error
	}else{
		struct NW_UI_Queue* tmp = *last;
		*last = (struct NW_UI_Queue*)malloc(sizeof(struct NW_UI_Queue));
		tmp->next = *last;
	}
	(*last)->next = 0;
	(*last)->data = (struct NW_User_Input*)malloc(sizeof(struct NW_User_Input));
	(*last)->data->msg_type = NW_USER_INPUT;
	(*last)->data->player_id = player_id;
	(*last)->data->x_pos = io_state->x_pos;
	(*last)->data->num_actions = io_state->num_actions;
	(*last)->data->frame_id = current_frame;
	(*last)->data->input_id = (++(*input_id));
	memcpy((*last)->data->action_queue, io_state->action_queue, sizeof(io_state->action_queue));
}

void nw_ui_queue_remove(
		u32 remove_until_input_id,
		struct NW_UI_Queue** first,
		struct NW_UI_Queue** last) {
	if (!first || !last) {
		return;
	}
	while (*first) {
		if ((*first)->data) {
			if ((*first)->data->input_id <= remove_until_input_id) {
				free((*first)->data);
				(*first)->data = 0;
			}else{
				break;
			}
		}
		struct NW_UI_Queue* tmp = *first;
		*first = (*first)->next;
		free(tmp);
	}
	if (!*first) {
		*last = 0;
	}
}

void nw_ui_queue_send(
		udsBindContext* bindctx,
		struct NW_UI_Queue* first) {
	while (first) {
		Result ret = udsSendTo(
				UDS_BROADCAST_NETWORKNODEID,
				1,
				UDS_SENDFLAG_Default,
				first->data,
				sizeof(struct NW_User_Input));
		if(UDS_CHECK_SENDTO_FATALERROR(ret)) {
				return;
		}
		first = first->next;
	}
}

void nw_ui_queue_clear(
		struct NW_UI_Queue** first,
		struct NW_UI_Queue** last) {
	if (!first || !last) {
		return; // error
	}
	if (*last) {
		nw_ui_queue_remove((*last)->data->input_id, first, last);
	}
}

// TODO: save all IO-Messages in Queue and resend them until the receivement is confirmed (higher confirm-id auto-confirms lower ids)
int server_run_level(
		udsBindContext* bindctx,
		struct Level* level,
		const char* level_id,
		u8* lemmings,
		struct MainMenuData* menu_data,
		struct MainInGameData* main_data) {
	// send run frame message to client, so he can also start loading the level!
	struct NW_Run_Frame nw_rf;
	struct NW_UI_Queue* nw_ui_queue_first = 0;
	struct NW_UI_Queue* nw_ui_queue_last = 0;
	nw_rf.msg_type = NW_RUN_FRAME;
	nw_rf.frame_id = 0;
	nw_rf.required_input_id = 0;
	udsSendTo(
			UDS_BROADCAST_NETWORKNODEID,
			1,
			UDS_SENDFLAG_Default,
			&nw_rf,
			sizeof(struct NW_Run_Frame));

	// show black screen while loading
	begin_frame();
	clear(BOTTOM_SCREEN);
	end_frame();

	struct InputState io_state;
	init_io_state(&io_state, level->player[0].x_pos);

	main_data->high_perf_palette[7] = level->palette[8];
	init_lemmings(level);

	u64 next_frame = osGetTime();
	u64 next_input = next_frame;
	u32 current_frame = 0;
	u32 input_id = 0;
	u16 max_level_time = level->frames_left;
	u8 error = 0;
	// main loop
	while (aptMainLoop()) {

		if (!next_frame) {
			break;
		}

		if (!connection_alive()) {
			error = 1;
			break;
		}

		// read user input
		do {
			// timing...
			u64 time = osGetTime();
			if (next_input > time) {
				break;
			}
			if (was_suspended()) {
				next_input = time;
				next_frame = time;
			}
			next_input += INPUT_SAMPLING_MILLIS;
			if (!read_io(level, &io_state, 0)) {
				stop_audio();
				nw_ui_queue_clear(&nw_ui_queue_first, &nw_ui_queue_last);
				return 0;
			}
		}while(1);

		// apply actions from action_queue
		if (process_action_queue(io_state.action_queue, io_state.num_actions, level, 0, 1)) {
			level->frames_left = max_level_time;
		}

		// send action queue to client!
		if (io_state.num_actions || level->player[0].x_pos != io_state.x_pos) {
			nw_ui_queue_add(0, &io_state, &input_id, current_frame, &nw_ui_queue_first, &nw_ui_queue_last);
		}
		nw_ui_queue_send(bindctx, nw_ui_queue_first);

		// apply user input
		level->player[0].x_pos = io_state.x_pos;

		// receive actions from client
		do {
			union {
				u8 msg_type;
				struct NW_User_Input ui;
				struct NW_Run_Frame rf;
			} nw;
			size_t actual_size = 0;
			u16 src_NetworkNodeID;
			Result ret = udsPullPacket(
					bindctx,
					&nw,
					sizeof(struct NW_User_Input),
					&actual_size,
					&src_NetworkNodeID);
			if (R_FAILED(ret)) {
				// TODO: clean up!
				stop_audio();
				nw_ui_queue_clear(&nw_ui_queue_first, &nw_ui_queue_last);
				return 0;
			}
			if (!actual_size) {
				break; // message receivement complete, preceed level
			}
			if (nw.msg_type == NW_USER_INPUT) {
				if (actual_size == sizeof(struct NW_User_Input)) {
				
					// TODO: test whether client ID match player_id... (only for multiple clients)
					if (nw.ui.player_id == 1) {
						if (process_action_queue(nw.ui.action_queue, nw.ui.num_actions, level, 1, 1)) {
							level->frames_left = max_level_time;
						}
						level->player[1].x_pos = nw.ui.x_pos;
						if (nw.ui.num_actions) {
							nw.ui.frame_id = current_frame;
							nw.ui.input_id = (++input_id);
							nw_ui_queue_add_copy(&nw.ui, &nw_ui_queue_first, &nw_ui_queue_last);
							nw_ui_queue_send(bindctx, nw_ui_queue_first);
						}
					}
				}
			}else if (nw.msg_type == NW_RUN_FRAME) {
				if (actual_size == sizeof(struct NW_Run_Frame)) {
					nw_ui_queue_remove(nw.rf.required_input_id,&nw_ui_queue_first, &nw_ui_queue_last);
				}
			}
			update_audio();
		}while(aptMainLoop());

		io_state.num_actions = 0;

		// update lemmings
		do {
			update_audio();

			// timing...
			u64 time = osGetTime();
			if (next_frame > time) {
				break;
			}
			if (was_suspended()) {
				// TODO: prevent suspension
				next_input = time;
				next_frame = time;
			}
			if (level->speed_up){
				next_frame += MS_PER_FRAME_SUPERLEM;
			}else{
				next_frame += MS_PER_FRAME;
			}

			// send NW_RUN_FRAME to client
			current_frame++;
			nw_rf.msg_type = NW_RUN_FRAME;
			nw_rf.frame_id = current_frame;
			nw_rf.required_input_id = input_id;
			Result ret = udsSendTo(
					UDS_BROADCAST_NETWORKNODEID,
					1,
					UDS_SENDFLAG_Default,
					&nw_rf,
					sizeof(struct NW_Run_Frame));
			if(UDS_CHECK_SENDTO_FATALERROR(ret)) {
				// TODO: clean up!
				stop_audio();
				nw_ui_queue_clear(&nw_ui_queue_first, &nw_ui_queue_last);
				return 0;
			}
			u8 changes = 0;
			if (!level_step(
					main_data,
					level,
					&changes)) { // simulate one DOS frame (one step)
				next_frame = 0;
				break;
			}
			if (changes) {
				level->frames_left = max_level_time;
			}
		}while(1);

		// draw frame
		render_level_frame(
					level_id,
					main_data,
					level,
					&io_state,
					0);
	}
	stop_audio();
	nw_ui_queue_clear(&nw_ui_queue_first, &nw_ui_queue_last);
	if (error) {
		return 0;
	}
	if (lemmings) {
		// set lemmings saved...
		lemmings[0] = level->player[0].rescued[0] + level->player[0].rescued[1];
		lemmings[1] = level->player[1].rescued[0] + level->player[1].rescued[1];
	}
	return 1;
}

int client_run_level(
		udsBindContext* bindctx,
		struct Level* level,
		const char* level_id,
		u8* lemmings,
		u16* won,
		struct MainMenuData* menu_data,
		struct MainInGameData* main_data) {
	// show black screen while loading
	begin_frame();
	clear(BOTTOM_SCREEN);
	end_frame();

	struct InputState io_state;
	init_io_state(&io_state, level->player[1].x_pos); // TODO: player's id?

	main_data->high_perf_palette[7] = level->palette[8];
	init_lemmings(level);

	u64 next_frame = osGetTime();
	u64 next_input = next_frame;
	u32 last_frame = 0;
	u32 last_input_id = 0;
	u8 game_ended = 0;
	u16 max_level_time = level->frames_left;
	// main loop
	while (aptMainLoop() && connection_alive()) {

		if (!next_frame) {
			break;
		}

		// read user input
		do {
			// timing...
			u64 time = osGetTime();
			if (next_input > time) {
				break;
			}
			if (was_suspended()) {
				next_input = time;
				next_frame = time;
			}
			next_input += INPUT_SAMPLING_MILLIS;
			if (!read_io(level, &io_state, 1)) {
				stop_audio();
				return 0;
			}
		}while(1);

		// send action queue to server!
		union {
			u8 type;
			struct NW_User_Input io;
			struct NW_Run_Frame frame;
			struct NW_Level_Result result;
			// other expected messages do not use payload yet
		} msg;
		if (io_state.num_actions || level->player[1].x_pos != io_state.x_pos) {
			msg.io.msg_type = NW_USER_INPUT;
			msg.io.player_id = 1; // TODO
			msg.io.x_pos = io_state.x_pos;
			msg.io.num_actions = io_state.num_actions;
			memcpy(
					msg.io.action_queue,
					io_state.action_queue,
					sizeof(io_state.action_queue));
			Result ret = udsSendTo(
					UDS_HOST_NETWORKNODEID,
					1,
					UDS_SENDFLAG_Default,
					&msg,
					sizeof(struct NW_User_Input));
			if(UDS_CHECK_SENDTO_FATALERROR(ret)) {
				// TODO: clean up!
				stop_audio();
				return 0;
			}
		}
		io_state.num_actions = 0;
		// apply user input
		level->player[1].x_pos = io_state.x_pos;

		// receive packets from server
		do {
			// TODO: leave this while loop after some time
			// to ensure that the screen does not freeze when a lot of packets are received...
			update_audio();
			size_t actual_size = 0;
			u16 src_NetworkNodeID;
			Result ret = udsPullPacket(
					bindctx,
					&msg,
					sizeof(msg),
					&actual_size,
					&src_NetworkNodeID);
			if (R_FAILED(ret)) {
				// TODO: clean up!
				stop_audio();
				return 0;
			}
			// TODO: ignore messages not from server (src_NetworkNodeID)
			if (!actual_size) {
				break; // message receivement complete, preceed level
			}
			switch (msg.type) {
				case NW_USER_INPUT:
					// process user input!
					if (actual_size == sizeof(struct NW_User_Input)) {
						if (msg.io.input_id <= last_input_id+1) {
							// inform server about receivement
							struct NW_Run_Frame info;
							info.msg_type = NW_RUN_FRAME;
							info.frame_id = msg.io.frame_id;
							info.required_input_id = msg.io.input_id;
							udsSendTo(
									UDS_HOST_NETWORKNODEID,
									1,
									UDS_SENDFLAG_Default,
									&info,
									sizeof(struct NW_Run_Frame));
						}
						if (msg.io.input_id != last_input_id+1) {
							break; // ignore message
						}
						last_input_id++;
						if (msg.io.frame_id < last_frame) {
							// FATAL ERROR
							// TODO: tidy up...
							stop_audio();
							return 0;
						}
						// run frames if necessary until input is done...
						u32 steps = msg.io.frame_id - last_frame;
						last_frame = msg.io.frame_id;
						while (steps) {
							u8 changes = 0;
							if (!level_step(
									main_data,
									level,
									&changes)) { // simulate one DOS frame (one step)
								// fatal error:
								// no action must be triggered after end of game
								// TODO: tidy up...
								stop_audio();
								return 0;
							}
							if (changes) {
								level->frames_left = max_level_time;
							}
							steps--;
						}
						// parse input...
						if (process_action_queue(msg.io.action_queue, msg.io.num_actions, level, msg.io.player_id, 1)) {
							level->frames_left = max_level_time;
						}
						if (msg.io.player_id != 1) {
							// don't overwrite own position
							level->player[msg.io.player_id].x_pos = msg.io.x_pos;
						}
					}
					break;
				case NW_RUN_FRAME:
					if (actual_size == sizeof(struct NW_Run_Frame)) {
						if (msg.frame.required_input_id <= last_input_id) {
							u32 steps = msg.frame.frame_id - last_frame;
							last_frame = msg.frame.frame_id;
							while (steps) {
								if (game_ended) {
									// fatal error:
									// no frame should occur after end of game
									// TODO: tidy up...
									return 0;
								}
								u8 changes = 0;
								if (!level_step(
										main_data,
										level,
										&changes)) { // simulate one DOS frame (one step)
									stop_audio();
									game_ended = 1;
								}
								if (changes) {
									level->frames_left = max_level_time;
								}
								steps--;
							}
						}
					}
					break;
				case NW_LVLRESULT:
					// confirm receivement
					udsSendTo(
							UDS_HOST_NETWORKNODEID,
							1,
							UDS_SENDFLAG_Default,
							&msg,
							1);
					stop_audio();
					if (lemmings) {
						lemmings[0] = msg.result.lemmings_saved[0];
						lemmings[1] = msg.result.lemmings_saved[1];
					}
					if (won) {
						won[0] = msg.result.won[0];
						won[1] = msg.result.won[1];
					}
					return 1;
				default:
					stop_audio();
					return 0; // error
			}
		}while(aptMainLoop());

		update_audio();
		// draw frame
		render_level_frame(
					level_id,
					main_data,
					level,
					&io_state,
					1);
	}
	stop_audio();
	// error: aptMainLoop() exit
	// TODO: tidy up
	return 0;
}

int server_send_result(
		udsBindContext* bindctx,
		u8 lemmings[2],
		u16 won[2]) {
	struct NW_Level_Result nw_lr;
	nw_lr.msg_type = NW_LVLRESULT;
	nw_lr.lemmings_saved[0] = lemmings[0];
	nw_lr.lemmings_saved[1] = lemmings[1];
	nw_lr.won[0] = won[0];
	nw_lr.won[1] = won[1];
	if (!send_until_confirmed(&nw_lr, sizeof(struct NW_Level_Result), bindctx)) {
		return 0;
	}
	return 1;
}
