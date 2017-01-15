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
// count number of available custom files in specific folder
u8 count_custom_levels(const char* path);
// import custom level (from uncompressed lvl file)
int read_level_file(
		const char* filename,
		const char* ressource_path,
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
