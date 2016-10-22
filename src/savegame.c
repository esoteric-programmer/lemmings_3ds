#include <stdio.h>
#include <string.h>
#include "savegame.h"
#include "settings.h"
#include "gamespecific.h"

void read_savegame(struct SaveGame* savegame) {
	char savefile_fn[64];
	int i;
	int offset = 0;
	if (!savegame) {
		return;
	}
	if (!savegame->progress) {
		return;
	}

	// fill progress with zeros
	for (i=0;i<LEMMING_GAMES;i++) {
		memset(savegame->progress+offset,0,import[i].num_of_difficulties);
		offset += import[i].num_of_difficulties;
	}

	sprintf(savefile_fn,"%s/SAVEGAME.DAT", PATH_ROOT);
	FILE* savefile = fopen(savefile_fn,"rb");
	if (!savefile) {
		return;
	}
	u8 version = 0;
	if (!fread(&version,1,1,savefile)) {
		fclose(savefile);
		return;
	}
	if (version < 0x20) {
		// old file; directly starts with progress of FUN rating
		version = 0;
		fseek(savefile,0,SEEK_SET);
	}else{
		version -= 0x1F;
	}

	if (version > SAVEGAME_VERSION) {
		// file is too new and therefore unsupported
		fclose(savefile);
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
		if (fread(
						savegame->progress+offset,
						1,
						import[i].num_of_difficulties,
						savefile)
				!= import[i].num_of_difficulties) {
			memset(savegame->progress+offset,0,import[i].num_of_difficulties);
			break;
		}
		offset += import[i].num_of_difficulties;
	}
	if (version > 1) {
		fread(&savegame->audio_settings,1,1,savefile);
	}
	fclose(savefile);
}

void write_savegame(struct SaveGame* savegame) {
	if (!savegame) {
		return;
	}
	if (!savegame->progress) {
		return;
	}
	char savefile_fn[64];
	sprintf(savefile_fn,"%s/SAVEGAME.DAT", PATH_ROOT);
	FILE* savefile = fopen(savefile_fn,"wb");
	if (!savefile) {
		return;
	}
	u8 version = SAVEGAME_VERSION + 0x1F;
	fwrite(&version,1,1,savefile);
	int i;
	int offset = 0;
	for (i=0;i<LEMMING_GAMES;i++) {
		fwrite(
				savegame->progress+offset,
				1,
				import[i].num_of_difficulties,
				savefile);
		offset += import[i].num_of_difficulties;
	}
	fwrite(&savegame->audio_settings,1,1,savefile);
	fclose(savefile);
}
