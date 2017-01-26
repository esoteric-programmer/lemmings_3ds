#include <malloc.h>
#include <string.h>
#include "import_main.h"
#include "decode.h"
#include "gamespecific.h"
#include "settings.h"
#include "audio.h"

const struct {
	int w;
	int h;
	int d;
} format[337] = {
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 14, 3},
	{16, 14, 3},
	{16, 14, 3},
	{16, 14, 3},
	{16, 14, 3},
	{16, 14, 3},
	{16, 14, 3},
	{16, 14, 3},
	{16, 14, 3},
	{16, 14, 3},
	{16, 14, 3},
	{16, 14, 3},
	{16, 14, 3},
	{16, 14, 3},
	{16, 14, 3},
	{16, 14, 3},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 12, 2},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 10, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 13, 3},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 16, 3},
	{16, 16, 3},
	{16, 16, 3},
	{16, 16, 3},
	{16, 16, 3},
	{16, 16, 3},
	{16, 16, 3},
	{16, 16, 3},
	{16, 16, 3},
	{16, 16, 3},
	{16, 16, 3},
	{16, 16, 3},
	{16, 16, 3},
	{16, 16, 3},
	{16, 16, 3},
	{16, 16, 3},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 13, 2},
	{16, 13, 2},
	{16, 13, 2},
	{16, 13, 2},
	{16, 13, 2},
	{16, 13, 2},
	{16, 13, 2},
	{16, 13, 2},
	{16, 14, 4},
	{16, 14, 4},
	{16, 14, 4},
	{16, 14, 4},
	{16, 14, 4},
	{16, 14, 4},
	{16, 14, 4},
	{16, 14, 4},
	{16, 14, 4},
	{16, 14, 4},
	{16, 14, 4},
	{16, 14, 4},
	{16, 14, 4},
	{16, 14, 4},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{16, 10, 2},
	{32, 32, 3}
};

const struct {
	int w;
	int h;
} mask_sizes[23] = {
	{16, 10},
	{16, 10},
	{16, 10},
	{16, 10},
	{16, 10},
	{16, 10},
	{16, 10},
	{16, 10},
	{16, 13},
	{16, 13},
	{16, 13},
	{16, 13},
	{16, 22},
	{8, 8},
	{8, 8},
	{8, 8},
	{8, 8},
	{8, 8},
	{8, 8},
	{8, 8},
	{8, 8},
	{8, 8},
	{8, 8}
};

int read_main_ingame(u8 game, struct MainInGameData* data){
	if (!data) {
		return 0;
	}
	memset(data,0,sizeof(struct MainInGameData));
	char mainfile[64];
	sprintf(mainfile,"%s/%s/MAIN.DAT", PATH_ROOT, import[game].path);
	FILE* main_dat = fopen(mainfile, "rb");
	if (!main_dat) {
		return 0; // error
	}

	struct Data* dec = decompress_cur_section(main_dat);
	if (!dec) {
		fclose(main_dat);
		return 0; // error
	}
	if (dec->size != 0x5270) {
		free(dec);
		fclose(main_dat);
		return 0; // error
	}

	// temporary variables
	char tmp[512];
	//unsigned char img[1024];

	int i;
	int frame;
	int offset = 0;
	for (frame=0;frame<337;frame++) {
		// copy from main.dat, fill missing bitplanes
		int size = format[frame].w*format[frame].h*format[frame].d/8;
		memcpy(tmp, dec->data+offset, size);
		memset(tmp+size, 0, format[frame].w*format[frame].h/2 - size);
		// convert to normal bitmap
		data->lemmings_anim[frame] = (struct Image*)malloc(sizeof(struct Image)+format[frame].w*format[frame].h);
		if (!data->lemmings_anim[frame]) {
			// error: out of memory
			free(dec);
			fclose(main_dat);
			free_ingame_data_arrays(data);
			return 0;
		}
		data->lemmings_anim[frame]->width = format[frame].w;
		data->lemmings_anim[frame]->height = format[frame].h;
		planar_image_to_pixmap(
				data->lemmings_anim[frame]->data,
				tmp,
				format[frame].w*format[frame].h,
				0);
		offset += size;
	}
	free(dec);



	// read section 1
	dec = decompress_cur_section(main_dat);
	offset = 0;
	for (frame=0;frame<23;frame++) {
		// copy from main.dat, fill missing bitplanes
		int size = mask_sizes[frame].w*mask_sizes[frame].h/8;
		memcpy(tmp, dec->data+offset, size);
		memset(tmp+size, 0, mask_sizes[frame].w*mask_sizes[frame].h/2 - size);
		// convert to normal bitmap
		data->masks[frame] = (struct Image*)malloc(sizeof(struct Image)+mask_sizes[frame].w*mask_sizes[frame].h);
		if (!data->masks[frame]) {
			// error: out of memory
			free(dec);
			fclose(main_dat);
			free_ingame_data_arrays(data);
			return 0;
		}
		data->masks[frame]->width = mask_sizes[frame].w;
		data->masks[frame]->height = mask_sizes[frame].h;
		planar_image_to_pixmap(
				data->masks[frame]->data,
				tmp,
				mask_sizes[frame].w*mask_sizes[frame].h,
				0);
		offset += size;
	}
	free(dec);



	// read section 2
	dec = decompress_cur_section(main_dat);
	if (!dec) {
		fclose(main_dat);
		return 0; // error
	}
	for (i=0;i<16;i++) {
		data->high_perf_palette[i] = import[game].highperf_palette[i];
	}
	for (i=0;i<7;i++) {
		data->level_base_palette[i] = import[game].ingame_palette[i];
	}

	planar_image_to_pixmap(data->high_perf_toolbar, dec->data, 320*40, 0);
	for (i=0;i<20;i++) {
		memcpy(tmp,dec->data+0x1900+8*i,8);
		memset(tmp+8,0,8*3);
		planar_image_to_pixmap(data->skill_numbers+i*8*8, tmp, 8*8, 0);
	}
	for (i=0;i<38;i++) {
		memcpy(tmp,dec->data+0x19A0+48*i,48);
		//0->0, 2->2, 3->3, 5->9; other colors not tested yet
		memcpy(tmp+48,tmp+32,16);
		memset(tmp+32,0,16);
		planar_image_to_pixmap(data->high_perf_font+i*8*16, tmp, 8*16, 0);
	}
	free(dec);
	fclose(main_dat);

	import_audio(game);

	return 1; // success
}

void free_ingame_data_arrays(struct MainInGameData* data) {
	if (!data) {
		return;
	}
	int i;
	for (i=0;i<337;i++) {
		if (data->lemmings_anim[i]) {
			free(data->lemmings_anim[i]);
			data->lemmings_anim[i] = 0;
		}
	}
	for (i=0;i<23;i++) {
		if (data->masks[i]) {
			free(data->masks[i]);
			data->masks[i] = 0;
		}
	}
}


int read_main_menu(u8 game, struct MainMenuData* data) {
	if (!data) {
		return 0;
	}
	memset(data,0,sizeof(struct MainMenuData));
	char mainfile[64];
	sprintf(mainfile,"%s/%s/MAIN.DAT", PATH_ROOT, import[game].path);
	FILE* main_dat = fopen(mainfile, "rb");
	if (!main_dat) {
		return 0; // error
	}
	if (!goto_section(main_dat,3)) {
		fclose(main_dat);
		return 0; // error
	}
	struct Data* dec = decompress_cur_section(main_dat);
	if (!dec) {
		fclose(main_dat);
		return 0; // error
	}
	// temporary variable
	char* tmp = (char*)malloc(60000);
	if (!tmp) {
		free(dec);
		fclose(main_dat);
		return 0; // error
	}

	int i;
	for (i=0;i<16;i++) {
		data->palette[i] = import[game].main_palette[i];
	}

	const struct {
		int w;
		int h;
		int d;
	} format[16] = {
		{320, 104, 2},
		{632,  94, 4},
		{120,  61, 4},
		{120,  61, 4},
		{120,  61, 4},
		{120,  61, 4},
		{120,  61, 4},
		{120,  61, 4},
		{ 64,  31, 4},
		{ 64,  31, 4},
		{ 16,  16, 4},
		{ 72,  27, 4},
		{ 72,  27, 4},
		{ 72,  27, 4},
		{ 72,  27, 4},
		{ 72,  27, 4}
	};

	int offset = 0;
	for (i=0;i<10;i++) {
		// copy from main.dat, fill missing bitplanes
		int size = format[i].w*format[i].h*format[i].d/8;
		memcpy(tmp, dec->data+offset, size);
		memset(tmp+size, 0, format[i].w*format[i].h/2 - size);
		// convert to normal bitmap
		data->static_pictures[i] = (struct Image*)malloc(sizeof(struct Image)+format[i].w*format[i].h);
		if (!data->static_pictures[i]) {
			// error: out of memory
			free(dec);
			free(tmp);
			fclose(main_dat);
			free_menu_data_arrays(data);
			return 0;
		}
		data->static_pictures[i]->width = format[i].w;
		data->static_pictures[i]->height = format[i].h;
		planar_image_to_pixmap(
				data->static_pictures[i]->data,
				tmp,
				format[i].w*format[i].h, 0);
		if (i>=8) {
			int j;
			for (j=0;j<format[i].w*format[i].h;j++) {
				data->static_pictures[i]->data[j] |= 0xF0;
			}
		}
		offset += size;
	}
	free(dec);
	free(tmp);
	tmp = (char*)malloc(2048);
	if (!tmp) {
		free(dec);
		fclose(main_dat);
		return 0; // error
	}
	dec = decompress_cur_section(main_dat);
	if (!dec) {
		free(tmp);
		fclose(main_dat);
		return 0; // error
	}

	// read blinking eyes
	for (i=0;i<7*8;i++) {
		planar_image_to_pixmap(
				data->blinking_eyes+i*32*12,
				dec->data+192*i,
				32*12,
				0);
	}

	for (i=0;i<2*16;i++) {
		planar_image_to_pixmap(
				data->scroller+i*48*16,
				dec->data+0x2A00+384*i,
				48*16,
				0);
	}

	// read reel and difficulty signs
	offset = 0x5A00;
	for (i=10;i<11+import[game].num_of_difficulty_graphics;i++) {
		// copy from main.dat, fill missing bitplanes
		int size = format[i].w*format[i].h*format[i].d/8;
		memcpy(tmp, dec->data+offset, size);
		memset(tmp+size, 0, format[i].w*format[i].h/2 - size);
		// convert to normal bitmap
		data->static_pictures[i] = (struct Image*)malloc(sizeof(struct Image)+format[i].w*format[i].h);
		if (!data->static_pictures[i]) {
			// error: out of memory
			free(dec);
			free(tmp);
			fclose(main_dat);
			free_menu_data_arrays(data);
			return 0;
		}
		data->static_pictures[i]->width = format[i].w;
		data->static_pictures[i]->height = format[i].h;
		planar_image_to_pixmap(
				data->static_pictures[i]->data,
				tmp,
				format[i].w*format[i].h,
				0);
		if (i>10) {
			s16 p;
			for (p=0;p<format[i].w*format[i].h;p++) {
				data->static_pictures[i]->data[p] |= 0xF0;
			}
		}
		offset += size;
	}

	// read menu-font
	for (i=0;i<94;i++) {
		memcpy(
				tmp,
				dec->data+0x5A80+0x3CC*(int)import[game].num_of_difficulty_graphics+96*i,
				96);
		memset(tmp+96,0,32);
		planar_image_to_pixmap(
				data->menu_font+i*16*16,
				tmp,
				16*16,
				0);
	}

	free(dec);
	free(tmp);
	fclose(main_dat);
	return 1; // success
}

void free_menu_data_arrays(struct MainMenuData* data) {
	if (!data) {
		return;
	}
	int i;
	for (i=0;i<10;i++) {
		if (data->static_pictures[i]) {
			free(data->static_pictures[i]);
			data->static_pictures[i] = 0;
		}
	}
}

// (re)initialize gamespecific data (top screen, palettes, menu screen, toolbar, ...)
int read_gamespecific_data(u8 game, struct MainMenuData* menu, struct MainInGameData* ingame) {
	clean_gamedata(menu, ingame);
	if (!read_main_menu(game, menu)) {
		memset(menu,0,sizeof(struct MainMenuData));
		return 0;
	}
	if (!read_main_ingame(game, ingame)) {
		free_menu_data_arrays(menu);
		memset(ingame,0,sizeof(struct MainInGameData));
		memset(menu,0,sizeof(struct MainMenuData));
		return 0;
	}
	return 1;
}

void clean_gamedata(struct MainMenuData* menu_data, struct MainInGameData* main_data) {
	if (main_data) {
		free_ingame_data_arrays(main_data);
		memset(main_data,0,sizeof(struct MainInGameData));
	}
	if (menu_data) {
		free_menu_data_arrays(menu_data);
		memset(menu_data,0,sizeof(struct MainMenuData));
	}
}
