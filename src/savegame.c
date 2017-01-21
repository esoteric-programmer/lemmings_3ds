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
	savegame->multiplayer_progress[0] = 0;
	savegame->multiplayer_progress[1] = 0;
	savegame->last_multiplayer_level = 0;
	savegame->last_multiplayer_game = 0;

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
		if (version < 1 &&
				(i == LEMMINGS_DEMO || i == OH_NO_DEMO
				|| i == HOLIDAY_93_DEMO || i == HOLIDAY_94_DEMO)) {
			// skip unsupported games when reading old version savefile
			offset += import[i].num_of_difficulties;
			continue;
		}
		// read data
		if (fread(
						&savegame->progress[offset],
						1,
						import[i].num_of_difficulties,
						savefile)
				!= import[i].num_of_difficulties) {
			memset(savegame->progress+offset,0,import[i].num_of_difficulties);
			break;
		}
		offset += import[i].num_of_difficulties;
	}
	if (version > 3) {
		fread(savegame->multiplayer_progress,1,2,savefile);
	}
	settings.sfx_volume = 100;
	settings.music_volume = 100;
	savegame->last_game = 0;
	savegame->last_level = 0;
	if (version == 2) {
		u8 tmp;
		fread(&tmp,1,1,savefile);
		switch (tmp) {
			case 1:
				settings.sfx_volume = 0;
			case 2:
				settings.music_volume = 0;
			default:
				break;
		}
	}
	if (version > 2) {
		fread(&settings.sfx_volume,1,1,savefile);
		fread(&settings.music_volume,1,1,savefile);
		fread(&savegame->last_game,1,1,savefile);
		fread(&savegame->last_level,1,1,savefile);
		if (version > 5) {
			fread(&savegame->last_multiplayer_game,1,1,savefile);
			fread(&savegame->last_multiplayer_level,1,1,savefile);
		}
		fread(&settings.glitch_nuke,1,1,savefile);
		fread(&settings.glitch_entrance_pausing,1,1,savefile);
		fread(&settings.glitch_mining_right_oneway,1,1,savefile);
		fread(&settings.glitch_shrugger,1,1,savefile);
		fread(&settings.glitch_mayhem12,1,1,savefile);
		fread(&settings.glitch_direct_drop,1,1,savefile);
		fread(&settings.speedup_millis_per_frame,1,1,savefile);
		fread(&settings.audio_order,1,1,savefile);
		fread(&settings.dlbclick_nuke,1,1,savefile);
		fread(&settings.dblclick_exit,1,1,savefile);
		fread(&settings.skip_unavailable_skills,1,1,savefile);
		fread(&settings.zoom_mode_active,1,1,savefile);
		if (version > 3) {
			fread(&settings.amiga_background,1,1,savefile);
		}
		if (version > 4) {
			u8 two_player_add_saved_lemmings;
			fread(&two_player_add_saved_lemmings,1,1,savefile);
			settings.two_player_always_equal = !two_player_add_saved_lemmings;
		}
		for (i=0;i<2;i++) {
			fread(&settings.key_bindings[i].modifier,1,4,savefile);
			fread(&settings.key_bindings[i].click,1,4,savefile);
			fread(&settings.key_bindings[i].inc_rate,1,4,savefile);
			fread(&settings.key_bindings[i].dec_rate,1,4,savefile);
			fread(&settings.key_bindings[i].next_skill,1,4,savefile);
			fread(&settings.key_bindings[i].prev_skill,1,4,savefile);
			fread(&settings.key_bindings[i].pause,1,4,savefile);
			fread(&settings.key_bindings[i].nuke,1,4,savefile);
			fread(&settings.key_bindings[i].exit,1,4,savefile);
			fread(&settings.key_bindings[i].speed_up,1,4,savefile);
			fread(&settings.key_bindings[i].non_prio,1,4,savefile);
			fread(&settings.key_bindings[i].step_one_frame,1,4,savefile);
			fread(&settings.key_bindings[i].step_backwards,1,4,savefile);
			fread(&settings.key_bindings[i].play_backwards,1,4,savefile);
			fread(&settings.key_bindings[i].toggle_zoom_mode,1,4,savefile);
			fread(&settings.key_bindings[i].cursor_up,1,4,savefile);
			fread(&settings.key_bindings[i].cursor_down,1,4,savefile);
			fread(&settings.key_bindings[i].cursor_left,1,4,savefile);
			fread(&settings.key_bindings[i].cursor_right,1,4,savefile);
			fread(&settings.key_bindings[i].scroll_up,1,4,savefile);
			fread(&settings.key_bindings[i].scroll_down,1,4,savefile);
			fread(&settings.key_bindings[i].scroll_left,1,4,savefile);
			fread(&settings.key_bindings[i].scroll_right,1,4,savefile);
		}
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
	fwrite(savegame->multiplayer_progress,1,2,savefile);
	fwrite(&settings.sfx_volume,1,1,savefile);
	fwrite(&settings.music_volume,1,1,savefile);
	fwrite(&savegame->last_game,1,1,savefile);
	fwrite(&savegame->last_level,1,1,savefile);
	fwrite(&savegame->last_multiplayer_game,1,1,savefile);
	fwrite(&savegame->last_multiplayer_level,1,1,savefile);
	fwrite(&settings.glitch_nuke,1,1,savefile);
	fwrite(&settings.glitch_entrance_pausing,1,1,savefile);
	fwrite(&settings.glitch_mining_right_oneway,1,1,savefile);
	fwrite(&settings.glitch_shrugger,1,1,savefile);
	fwrite(&settings.glitch_mayhem12,1,1,savefile);
	fwrite(&settings.glitch_direct_drop,1,1,savefile);
	fwrite(&settings.speedup_millis_per_frame,1,1,savefile);
	fwrite(&settings.audio_order,1,1,savefile);
	fwrite(&settings.dlbclick_nuke,1,1,savefile);
	fwrite(&settings.dblclick_exit,1,1,savefile);
	fwrite(&settings.skip_unavailable_skills,1,1,savefile);
	fwrite(&settings.zoom_mode_active,1,1,savefile);
	fwrite(&settings.amiga_background,1,1,savefile);
	u8 two_player_add_saved_lemmings = !settings.two_player_always_equal;
	fwrite(&two_player_add_saved_lemmings,1,1,savefile);
	for (i=0;i<2;i++) {
		fwrite(&settings.key_bindings[i].modifier,1,4,savefile);
		fwrite(&settings.key_bindings[i].click,1,4,savefile);
		fwrite(&settings.key_bindings[i].inc_rate,1,4,savefile);
		fwrite(&settings.key_bindings[i].dec_rate,1,4,savefile);
		fwrite(&settings.key_bindings[i].next_skill,1,4,savefile);
		fwrite(&settings.key_bindings[i].prev_skill,1,4,savefile);
		fwrite(&settings.key_bindings[i].pause,1,4,savefile);
		fwrite(&settings.key_bindings[i].nuke,1,4,savefile);
		fwrite(&settings.key_bindings[i].exit,1,4,savefile);
		fwrite(&settings.key_bindings[i].speed_up,1,4,savefile);
		fwrite(&settings.key_bindings[i].non_prio,1,4,savefile);
		fwrite(&settings.key_bindings[i].step_one_frame,1,4,savefile);
		fwrite(&settings.key_bindings[i].step_backwards,1,4,savefile);
		fwrite(&settings.key_bindings[i].play_backwards,1,4,savefile);
		fwrite(&settings.key_bindings[i].toggle_zoom_mode,1,4,savefile);
		fwrite(&settings.key_bindings[i].cursor_up,1,4,savefile);
		fwrite(&settings.key_bindings[i].cursor_down,1,4,savefile);
		fwrite(&settings.key_bindings[i].cursor_left,1,4,savefile);
		fwrite(&settings.key_bindings[i].cursor_right,1,4,savefile);
		fwrite(&settings.key_bindings[i].scroll_up,1,4,savefile);
		fwrite(&settings.key_bindings[i].scroll_down,1,4,savefile);
		fwrite(&settings.key_bindings[i].scroll_left,1,4,savefile);
		fwrite(&settings.key_bindings[i].scroll_right,1,4,savefile);
	}
	fclose(savefile);
}
