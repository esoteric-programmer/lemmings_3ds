#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "data_cache.h"
#include "gamespecific.h"
#include "gamespecific_2p.h"
#include "import_level.h"
#include "settings.h"

void update_data_cache_old(u8 game, u8 multiplayer, const char* level_names) {
	u8 version = CACHE_FILE_VERSION;
	char cachefilename[64];
	u16 num_of_levels = 0;
	if (!level_names) {
		return;
	}
	if (!multiplayer) {
		num_of_levels = (u16)import[game].num_of_difficulties
				* (u16)import[game].num_of_level_per_difficulty;
	}else{
		num_of_levels = multiplayer;
	}
	sprintf(cachefilename, "%s/CACHE_%02X.DAT",PATH_ROOT, game + (multiplayer?0x80:0));
	FILE* cachefile = fopen(cachefilename, "wb");
	if (!cachefile) {
		return;
	}
	fwrite(&version, 1, 1, cachefile);
	if (multiplayer) {
		fwrite(&multiplayer, 1, 1, cachefile);
	}
	fwrite(level_names, 1, 33*num_of_levels, cachefile);
	fclose(cachefile);
}

u8 read_data_cache_old(u8 game, u8 multiplayer, char* level_names) {
	u8 version = 0;
	char cachefilename[64];
	u16 num_of_levels = 0;
	if (!level_names) {
		return 0;
	}
	if (!multiplayer) {
		num_of_levels = (u16)import[game].num_of_difficulties
				* (u16)import[game].num_of_level_per_difficulty;
	}else{
		return 0;
	}
	sprintf(cachefilename, "%s/CACHE_%02X.DAT",PATH_ROOT, game + (multiplayer?0x80:0));
	FILE* cachefile = fopen(cachefilename, "rb");
	if (!cachefile) {
		return 0;
	}
	fread(&version, 1, 1, cachefile);
	if (version > CACHE_FILE_VERSION) {
		return 0;
	}
	if (multiplayer) {
		fread(&num_of_levels, 1, 1, cachefile);
		if (num_of_levels > import_2p[game].num_levels) {
			num_of_levels = import_2p[game].num_levels;
		}
	}
	u16 size = fread(level_names, 1, 33*num_of_levels, cachefile);
	fclose(cachefile);
	if (size != 33*num_of_levels) {
		return 0;
	}
	return (multiplayer?num_of_levels:1);
}

void update_data_cache(const u8* games, const char* level_names, u16 overall_num_of_levels) {
	char cachefilename[64];
	sprintf(cachefilename, "%s/CACHE.DAT",PATH_ROOT);
	FILE* cachefile = fopen(cachefilename, "wb");
	if (!cachefile) {
		return;
	}
	fwrite(games, 1, LEMMING_GAMES, cachefile);
	fwrite(level_names, 1, 33*overall_num_of_levels, cachefile);
	fclose(cachefile);

	int i = 0;
	for (i=0;i<LEMMING_GAMES;i++) {
		sprintf(cachefilename, "%s/CACHE_%02X.DAT",PATH_ROOT, i);
		unlink(cachefilename);
	}
}

u8 read_data_cache(u8* games, char* level_names, u16 overall_num_of_levels) {
	char cachefilename[64];
	sprintf(cachefilename, "%s/CACHE.DAT",PATH_ROOT);
	FILE* cachefile = fopen(cachefilename, "rb");
	if (cachefile) {
		size_t c = fread(games, 1, LEMMING_GAMES, cachefile);
		c += fread(level_names, 1, 33*overall_num_of_levels, cachefile);
		fclose(cachefile);
		if (c == LEMMING_GAMES + overall_num_of_levels) {
			return 1;
		}
		memset(games, 0, LEMMING_GAMES);
	}
	u16 offset = 0;
	int i;
	for (i=0;i<LEMMING_GAMES;i++) {
		games[i] = read_data_cache_old(i, 0, level_names + offset);
		if (!games[i]) {
			games[i] = read_level_names(i, level_names + offset);
		}
		offset += 33*
				(u16)import[i].num_of_difficulties *
				(u16)import[i].num_of_level_per_difficulty;
		if (!aptMainLoop()) {
			return 0;
		}
	}
	update_data_cache(games, level_names, overall_num_of_levels);
	return 1;
}
