#ifndef DAtA_CACHE_H
#define DAtA_CACHE_H
#define CACHE_FILE_VERSION 0
#include <3ds.h>

void update_data_cache_old(u8 game, u8 multiplayer, const char* level_names);
u8 read_data_cache_old(u8 game, u8 multiplayer, char* level_names);
u8 read_data_cache(u8* games, char* level_names, u16 overall_num_of_levels);

#endif
