#ifndef IMPORT_LEVEL_H
#define IMPORT_LEVEL_H
#include <3ds.h>
#include "decode.h"
#include "level.h"

int read_level_names(u8 game, char* names);
int read_level(
		u8 game,
		u8 id,
		void* level,
		void* ground_data,
		struct Data** vgagr_s0,
		struct Data** vgagr_s1,
		struct Data** vgaspec);
// import custom level (from uncompressed lvl file)
int read_level_file(
		const char* path,
		const char* filename,
		void* level,
		void* ground_data,
		struct Data** vgagr_s0,
		struct Data** vgagr_s1,
		struct Data** vgaspec);
int parse_level(
		void* level,
		void* ground_data,
		struct Data* vgagr_s0,
		struct Data* vgagr_s1,
		struct Data* vgaspec,
		const u32* ingame_palette,
		u8 ABBA_order,
		u8 players,
		struct Level* output);
struct Level* init_level_from_dat(u8 game, u8 lvl, char* level_id);
void free_objects(struct ObjectType* objects[16]);
#endif
