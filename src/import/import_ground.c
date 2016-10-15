#include <malloc.h>
#include <string.h>
#include "settings.h"
#include "import_ground.h"

static inline void read_object_info(struct ObjectInfo* ret, FILE* in) {
	fread(&(ret->animation_flags),2,1,in);
	fread(&(ret->start_animation_frame_index),1,1,in);
	fread(&(ret->end_animation_frame_index),1,1,in);
	fread(&(ret->width),1,1,in);
	fread(&(ret->height),1,1,in);
	fread(&(ret->animation_frame_data_size),2,1,in);
	fread(&(ret->mask_offset_from_image),2,1,in);
	fread(&(ret->unknown1),2,1,in);
	fread(&(ret->unknown2),2,1,in);
	fread(&(ret->trigger_left),2,1,in);
	fread(&(ret->trigger_top),2,1,in);
	fread(&(ret->trigger_width),1,1,in);
	fread(&(ret->trigger_height),1,1,in);
	fread(&(ret->trigger_effect_id),1,1,in);
	fread(&(ret->animation_frames_base_loc),2,1,in);
	fread(&(ret->preview_image_index),2,1,in);
	fread(&(ret->unknown3),2,1,in);
	fread(&(ret->trap_sound_effect_id),1,1,in);
}

static inline void read_terrain_info(struct TerrainInfo* ret, FILE* in) {
	fread(&(ret->width),1,1,in);
	fread(&(ret->height),1,1,in);
	fread(&(ret->image_loc),2,1,in);
	fread(&(ret->mask_loc),2,1,in);
	fread(&(ret->unknown1),2,1,in);
}

static inline void read_palette(struct VariablePalette* ret, FILE* in) {
	int i;
	fread(ret->ega_custom,1,8,in);
	fread(ret->ega_standard,1,8,in);
	fread(ret->ega_preview,1,8,in);
	for (i=0;i<8;i++) {
		ret->vga_custom[i] = 0;
		fread(ret->vga_custom+i,1,3,in);
		#ifdef ABGR
		ret->vga_custom[i]=(((ret->vga_custom[i] >> 16) * 255 / 63) << 8) | ((((ret->vga_custom[i] & 0xFF00) >> 8) * 255 / 63) << 16) | (((ret->vga_custom[i] & 0xFF) * 255 / 63) << 24) | 0x000000FF;
		#else
		ret->vga_custom[i]=(((ret->vga_custom[i] >> 16) * 255 / 63) << 16) | ((((ret->vga_custom[i] & 0xFF00) >> 8) * 255 / 63) << 8) | ((ret->vga_custom[i] & 0xFF) * 255 / 63) | 0xFF000000;
		#endif
	}
	for (i=0;i<8;i++) {
		ret->vga_standard[i] = 0;
		fread(ret->vga_standard+i,1,3,in);
		#ifdef ABGR
		ret->vga_standard[i]=(((ret->vga_standard[i] >> 16) * 255 / 63) << 8) | ((((ret->vga_standard[i] & 0xFF00) >> 8) * 255 / 63) << 16) | (((ret->vga_standard[i] & 0xFF) * 255 / 63) << 24) | 0x000000FF;
		#else
		ret->vga_standard[i]=(((ret->vga_standard[i] >> 16) * 255 / 63) << 16) | ((((ret->vga_standard[i] & 0xFF00) >> 8) * 255 / 63) << 8) | ((ret->vga_standard[i] & 0xFF) * 255 / 63) | 0xFF000000;
		#endif
	}
	for (i=0;i<8;i++) {
		ret->vga_preview[i] = 0;
		fread(ret->vga_preview+i,1,3,in);
		#ifdef ABGR
		ret->vga_preview[i]=(((ret->vga_preview[i] >> 16) * 255 / 63) << 8) | ((((ret->vga_preview[i] & 0xFF00) >> 8) * 255 / 63) << 16) | (((ret->vga_preview[i] & 0xFF) * 255 / 63) << 24) | 0x000000FF;
		#else
		ret->vga_preview[i]=(((ret->vga_preview[i] >> 16) * 255 / 63) << 16) | ((((ret->vga_preview[i] & 0xFF00) >> 8) * 255 / 63) << 8) | ((ret->vga_preview[i] & 0xFF) * 255 / 63) | 0xFF000000;
		#endif
	}
}

// read all
int read_ground_data(struct GroundInfo* ret, FILE* in) {
	int i;
	if (ret == 0 || in == 0) {
		return 0;
	}
	for (i=0;i<16;i++) {
		read_object_info(ret->object_info+i,in);
	}
	for (i=0;i<64;i++) {
		read_terrain_info(ret->terrain_info+i,in);
	}
	read_palette(&(ret->palette),in);

	if (feof(in)) {
		return 0; // TODO: add better checks for success (also in subroutines)
	}
	return 1;
}
