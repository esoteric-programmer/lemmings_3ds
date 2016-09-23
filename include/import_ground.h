#ifndef IMPORT_GROUND_H
#define IMPORT_GROUND_H
#include <3ds.h>
#include <stdio.h>

struct ObjectInfo {
	u16 animation_flags;
	u8 start_animation_frame_index;
	u8 end_animation_frame_index;
	u8 width;
	u8 height;
	u16 animation_frame_data_size;
	u16 mask_offset_from_image;
	u16 unknown1;
	u16 unknown2;
	u16 trigger_left;
	u16 trigger_top;
	u8 trigger_width;
	u8 trigger_height;
	u8 trigger_effect_id;
	u16 animation_frames_base_loc;
	u16 preview_image_index;
	u16 unknown3;
	u8 trap_sound_effect_id;
};

struct TerrainInfo {
	u8 width;
	u8 height;
	u16 image_loc;
	u16 mask_loc;
	u16 unknown1;
};

struct VariablePalette {
	u8 ega_custom[8];
	u8 ega_standard[8];
	u8 ega_preview[8];
	u32 vga_custom[8];
	u32 vga_standard[8];
	u32 vga_preview[8];
};

struct GroundInfo {
	struct ObjectInfo object_info[16];
	struct TerrainInfo terrain_info[64];
	struct VariablePalette palette;
};

int read_ground_data(struct GroundInfo* ret, FILE* in);
#endif
