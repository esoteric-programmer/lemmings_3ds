#include <malloc.h>
#include <string.h>
#include "settings.h"
#include "import_ground.h"

inline u8 read_u8(void** in) {
	u16 ret = *(u8*)*in;
	*(u8**)in += 1;
	return ret;
}

inline u16 read_u16(void** in) {
	u16 ret = *(u16*)*in;
	*(u16**)in += 1;
	return ret;
}

inline void read_object_info(struct ObjectInfo* ret, void** in) {
	ret->animation_flags = read_u16(in);
	ret->start_animation_frame_index = read_u8(in);
	ret->end_animation_frame_index = read_u8(in);
	ret->width = read_u8(in);
	ret->height = read_u8(in);
	ret->animation_frame_data_size = read_u16(in);
	ret->mask_offset_from_image = read_u16(in);
	ret->unknown1 = read_u16(in);
	ret->unknown2 = read_u16(in);
	ret->trigger_left = read_u16(in);
	ret->trigger_top = read_u16(in);
	ret->trigger_width = read_u8(in);
	ret->trigger_height = read_u8(in);
	ret->trigger_effect_id = read_u8(in);
	ret->animation_frames_base_loc = read_u16(in);
	ret->preview_image_index = read_u16(in);
	ret->unknown3 = read_u16(in);
	ret->trap_sound_effect_id = read_u8(in);
}

inline void read_terrain_info(struct TerrainInfo* ret, void** in) {
	ret->width = read_u8(in);
	ret->height = read_u8(in);
	ret->image_loc = read_u16(in);
	ret->mask_loc = read_u16(in);
	ret->unknown1 = read_u16(in);
}

inline void read_palette(struct LevelPalette* ret, void** in) {
	int i;
	for (i=0;i<8;i++) {
		ret->ega_custom[i] = read_u8(in);
	}
	for (i=0;i<8;i++) {
		ret->ega_standard[i] = read_u8(in);
	}
	for (i=0;i<8;i++) {
		ret->ega_preview[i] = read_u8(in);
	}
	for (i=0;i<8;i++) {
		ret->vga_custom[i] = 0;
		memcpy(&ret->vga_custom[i],*in,3);
		*(u8**)in += 3;
		ret->vga_custom[i] =
				  (((ret->vga_custom[i] >> 16) * 255 / 63) << 8)
				| ((((ret->vga_custom[i] & 0xFF00) >> 8) * 255 / 63) << 16)
				| (((ret->vga_custom[i] & 0xFF) * 255 / 63) << 24) | 0x000000FF;
	}
	for (i=0;i<8;i++) {
		ret->vga_standard[i] = 0;
		memcpy(&ret->vga_standard[i],*in,3);
		*(u8**)in += 3;
		ret->vga_standard[i] =
				  (((ret->vga_standard[i] >> 16) * 255 / 63) << 8)
				| ((((ret->vga_standard[i] & 0xFF00) >> 8) * 255 / 63) << 16)
				| (((ret->vga_standard[i] & 0xFF) * 255 / 63) << 24) | 0x000000FF;
	}
	for (i=0;i<8;i++) {
		ret->vga_preview[i] = 0;
		memcpy(&ret->vga_preview[i],*in,3);
		*(u8**)in += 3;
		ret->vga_preview[i] =
				  (((ret->vga_preview[i] >> 16) * 255 / 63) << 8)
				| ((((ret->vga_preview[i] & 0xFF00) >> 8) * 255 / 63) << 16)
				| (((ret->vga_preview[i] & 0xFF) * 255 / 63) << 24) | 0x000000FF;
	}
}

// read all
int read_ground_data(struct GroundInfo* ret, void* ground_data) {
	int i;
	if (ret == 0 || ground_data == 0) {
		return 0;
	}
	void* in = ground_data;
	for (i=0;i<16;i++) {
		read_object_info(&ret->object_info[i],&in);
	}
	for (i=0;i<64;i++) {
		read_terrain_info(&ret->terrain_info[i],&in);
	}
	read_palette(&ret->palette,&in);
	return 1;
}
