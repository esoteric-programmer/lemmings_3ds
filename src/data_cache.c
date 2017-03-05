#include <stdio.h>
#include "data_cache.h"
#include "gamespecific.h"
#include "settings.h"

void update_data_cache(u8 game, u8 multiplayer, const char* level_names) {
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
		return;
	}
	sprintf(cachefilename, "%s/CACHE_%02X.DAT",PATH_ROOT, game + (multiplayer?0x80:0));
	FILE* cachefile = fopen(cachefilename, "wb");
	if (!cachefile) {
		return;
	}
	fwrite(&version, 1, 1, cachefile);
	fwrite(level_names, 1, 33*num_of_levels, cachefile);
	fclose(cachefile);
}

u8 read_data_cache(u8 game, u8 multiplayer, char* level_names) {
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
	u16 size = fread(level_names, 1, 33*num_of_levels, cachefile);
	fclose(cachefile);
	if (size != 33*num_of_levels) {
		return 0;
	}
	return 1;
}
