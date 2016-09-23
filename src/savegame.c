#include <stdio.h>
#include <string.h>
#include "savegame.h"
#include "settings.h"
#include "gamespecific.h"

void read_savegame(u8* progress) {
	char savegame_fn[64];
	int i;
	int offset = 0;

	// fill progress with zeros
	for (i=0;i<LEMMING_GAMES;i++) {
		memset(progress+offset,0,import[i].num_of_difficulties);
		offset += import[i].num_of_difficulties;
	}

	sprintf(savegame_fn,"%s/SAVEGAME.DAT", PATH_ROOT);
	FILE* savegame = fopen(savegame_fn,"rb");
	if (!savegame) {
		return;
	}
	u8 version = 0;
	if (!fread(&version,1,1,savegame)) {
		fclose(savegame);
		return;
	}
	if (version < 0x20) {
		// old file; directly starts with progress of FUN rating
		version = 0;
		fseek(savegame,0,SEEK_SET);
	}else{
		version -= 0x1F;
	}

	if (version > SAVEGAME_VERSION) {
		// file is too new and therefore unsupported
		fclose(savegame);
		return;
	}

	offset = 0;
	for (i=0;i<LEMMING_GAMES;i++) {
		if (version < 1 && (i == LEMMINGS_DEMO || i == OH_NO_DEMO || i == HOLIDAY_93_DEMO || i == HOLIDAY_94_DEMO)) {
			// skip unsupported games when reading old version savefile
			offset += import[i].num_of_difficulties;
			continue;
		}
		// read data
		if (fread(progress+offset,1,import[i].num_of_difficulties,savegame)
				!= import[i].num_of_difficulties) {
			memset(progress+offset,0,import[i].num_of_difficulties);
			break;
		}
		offset += import[i].num_of_difficulties;
	}
	fclose(savegame);
}

void write_savegame(u8* progress) {
	char savegame_fn[64];
	sprintf(savegame_fn,"%s/SAVEGAME.DAT", PATH_ROOT);
	FILE* savegame = fopen(savegame_fn,"wb");
	if (!savegame) {
		return;
	}
	u8 version = SAVEGAME_VERSION + 0x1F;
	fwrite(&version,1,1,savegame);
	int i;
	int offset = 0;
	for (i=0;i<LEMMING_GAMES;i++) {
		fwrite(progress+offset,1,import[i].num_of_difficulties,savegame);
		offset += import[i].num_of_difficulties;
	}
	fclose(savegame);
}
