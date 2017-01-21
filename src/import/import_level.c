#include <malloc.h>
#include <string.h>
#include <3ds.h>
#include "settings.h"
#include "import_level.h"
#include "gamespecific.h"
#include "decode.h"
#include "import_ground.h"
#include "audio.h"

struct Image {
	u16 width;
	u16 height;
	u8 data[0];
};

int decompress_rle(s8* in, s8* out, u16 out_length, u16 max_in_length) {
	u16 out_pos = 0;
	u16 in_pos = 0;
	u16 i;
	while ((u8)*in != 0x80) {
		if ((*in) & 0x80) {
			if (in_pos+1>=max_in_length) {
				break;
			}
			for (i=0;i<257-((u8)*in);i++) {
				if (out_pos>=out_length) {
					// error
					printf("out: %d, i: %d of %d\n",out_pos,i,257-((u8)*in));
					return in_pos;
				}
				*out = in[1];
				out++;
				out_pos++;
			}
			in+=2;
			in_pos+=2;
		} else {
			for (i=0;i<*in+1;i++) {
				if (out_pos>=out_length) {
					// error
					return in_pos;
				}
				if (in_pos+1+i>=max_in_length) {
					// error
					while (out_pos<out_length) {
						*out = 0;
						out++;
						out_pos++;
					}
					return in_pos;
				}
				*out = in[i+1];
				out++;
				out_pos++;
			}
			in_pos+=*in+2;
			in+=*in+2;
		}
		if (in_pos>=max_in_length) {
			// error
			while (out_pos<out_length) {
				*out = 0;
				out++;
				out_pos++;
			}
			return in_pos;
		}
	}
	out++;
	out_pos++;
	while (out_pos<out_length) {
		*out = 0;
		out++;
		out_pos++;
	}
	return in_pos; // success
}

int parse_vgagr(
		void* ground_data,
		struct Data* vgagr_s0,
		struct Data* vgagr_s1,
		u32 palette[16],
		struct Image* terrain_img[64],
		struct ObjectType* objects[16]) {
	struct GroundInfo info;
	int ter;
	int obj;
	int i;

	if (!ground_data || !vgagr_s0 || !vgagr_s1 || !palette || ! terrain_img || !objects) {
		return 0; // error
	}

	memset(terrain_img,0,64*sizeof(void*));
	memset(objects, 0,16*sizeof(void*));

	read_ground_data(&info, ground_data);
	struct Data* dec = vgagr_s0;

	for (i=0;i<8;i++) {
		palette[i+8] = info.palette.vga_custom[i];
	}
	palette[7] = palette[8];

	for (ter=0;ter<64;ter++){

		u16 img_length = ((u16)info.terrain_info[ter].width)*((u16)info.terrain_info[ter].height);
		if (img_length == 0) {
			continue;
		}
		terrain_img[ter] = (struct Image*)malloc(img_length*sizeof(u32)+sizeof(struct Image));
		if (!terrain_img[ter]) {
			// ERROR!
			// TODO
			return 0;
		}
		terrain_img[ter]->width = info.terrain_info[ter].width;
		terrain_img[ter]->height = info.terrain_info[ter].height;

		// TODO: test whether data->size is large enough
		planar_image_to_pixmap(
				terrain_img[ter]->data,
				dec->data + info.terrain_info[ter].image_loc,
				img_length,
				info.terrain_info[ter].mask_loc - info.terrain_info[ter].image_loc);
	}

	dec = vgagr_s1;
	if (!dec) {
		return 0; // error
	}

	// read object
	for (obj=0;obj<16;obj++){
		//objects
		u16 frames = ((u16)info.object_info[obj].end_animation_frame_index);
		u16 data_size = info.object_info[obj].animation_frame_data_size;
		u16 img_length = ((u16)info.object_info[obj].width)*((u16)info.object_info[obj].height);
		if (!data_size || !img_length) {
			continue;
		}
		objects[obj] = (struct ObjectType*)malloc(
				sizeof(struct ObjectType)+frames*img_length*sizeof(u32));
		if (!objects[obj]) {
			// ERROR!
			// TODO
			return 0;
		}
		objects[obj]->flags = info.object_info[obj].animation_flags;
		objects[obj]->width = info.object_info[obj].width;
		objects[obj]->height = info.object_info[obj].height;
		objects[obj]->start_frame = info.object_info[obj].start_animation_frame_index;
		objects[obj]->end_frame = info.object_info[obj].end_animation_frame_index;
		objects[obj]->trigger_x = info.object_info[obj].trigger_left;
		objects[obj]->trigger_y = info.object_info[obj].trigger_top;
		objects[obj]->trigger_width = info.object_info[obj].trigger_width;
		objects[obj]->trigger_height = info.object_info[obj].trigger_height;
		objects[obj]->trigger = info.object_info[obj].trigger_effect_id;
		objects[obj]->sound = info.object_info[obj].trap_sound_effect_id;
		objects[obj]->preview_frame = (info.object_info[obj].preview_image_index
				- info.object_info[obj].animation_frames_base_loc)
					/ data_size;
		if (objects[obj]->preview_frame > objects[obj]->end_frame) {
			objects[obj]->preview_frame = 0;
		}
		for (i=0;i<frames;i++) {
			// TODO: test whether data->size is large enough
			planar_image_to_pixmap(
					objects[obj]->data + i*img_length,
					dec->data + info.object_info[obj].animation_frames_base_loc + i*data_size,
					img_length,
					info.object_info[obj].mask_offset_from_image);
		}
	}

	return 1; // success
}

void free_terrain(struct Image* terrain_img[64]) {
	int i;
	for (i=0;i<64;i++) {
		if (terrain_img[i] != 0) {
			free(terrain_img[i]);
			terrain_img[i] = 0;
		}
	}
}

void free_objects(struct ObjectType* objects[16]) {
	int i;
	for (i=0;i<16;i++) {
		if (objects[i] != 0) {
			free(objects[i]);
			objects[i] = 0;
		}
	}
}

// read at most num_of_level many names; on success: set num_of_levels to the number of names that have been read
int read_level_names_from_path(const char* levelpath, u8* num_of_levels, char* names) {
	if (!levelpath || !num_of_levels || !names) {
		return 0;
	}
	if (!*num_of_levels) {
		return 0;
	}
	u8 level;
	for (level = 0; level < *num_of_levels && level < 99; level++) {
		char levelfilename[64];
		sprintf(levelfilename, "%s/%s/%02u.lvl",PATH_ROOT, levelpath, level+1);
		FILE* levelfile = fopen(levelfilename, "rb");
		if (!levelfile) {
			break;
		}
		fseek(levelfile,2016,SEEK_SET);
		if (!fread(names+33*(u16)level, 1, 32, levelfile)) {
			fclose(levelfile);
			break;
		}
		fclose(levelfile);
		names[33*(u16)level+32] = 0;
		char* end = &names[33*(u16)level+31];
		while (end >= &names[33*(u16)level] && (*end == ' ' || *end == 0)) {
			*end = 0;
			end--;
		}
		char* start = &names[33*(u16)level];
		while (*start == ' ') {
			start++;
		}
		char* tmp = &names[33*(u16)level];
		while (*start != 0) {
			*tmp = *start;
			tmp++;
			start++;
		}
		*tmp = 0;
	}
	*num_of_levels = level;
	return 1;
}


// names: 33 bytes per levelname (at most 32 bytes followed by \0)
int read_level_names(u8 game, char* names) {
	// TODO: check game parameter?
	if (!names) {
		return 0; // error
	}

	char* unordered_names = (char*)malloc(33*256);
	if (!unordered_names) {
		return 0;
	}
	memset(unordered_names,0,33*256);
	memset(names,0,33*(int)import[game].num_of_difficulties * (int)import[game].num_of_level_per_difficulty);
	int levelfile_id = 0;

	while(levelfile_id < 16) {
		char levelfilename[64];
		FILE* levelfile = 0;
		struct Data* dec = 0;
		sprintf(levelfilename,"%s/%s/%s%03d.DAT",
				PATH_ROOT,import[game].path,import[game].level_dat_prefix,levelfile_id);

		levelfile = fopen(levelfilename,"rb");
		if (!levelfile) {
			break;
		}
		int section = 0;
		while (section < 8) {
			dec = decompress_cur_section(levelfile);
			if (!dec) {
				break;
			}
			if (dec->size != 2048) {
				free(dec);
				continue;
			}
			memcpy(unordered_names+33*(8*levelfile_id+section),dec->data+2016,32);
			free(dec);
			section++;
		}
		fclose(levelfile);
		levelfile_id++;
	}

	if (!levelfile_id) {
		free(unordered_names);
		return 0;
	}

	char oddtable_fn[64];
	sprintf(oddtable_fn,"%s/%s/ODDTABLE.DAT",PATH_ROOT,import[game].path);
	FILE* oddtable = fopen(oddtable_fn,"rb");
	if (oddtable) {
		int i;
		for (i=0;i<128;i++) {
			fseek(oddtable,56*i+24,SEEK_SET);
			if (!fread(unordered_names+33*(128+i),1,32,oddtable)) {
				break;
			}
		}
		fclose(oddtable);
	}

	// now copy from unordered_names to names
	int j;
	for (
			j=0;
			j < (int)import[game].num_of_difficulties
					* (int)import[game].num_of_level_per_difficulty;
			j++) {
		char* name = names+33*j;
		memcpy(name, unordered_names+33*(import[game].level_position[j]),33);
		int i = 32;
		while (i > 0 && name[i-1] == ' ') {
			i--;
		}
		name[i] = 0;
		char* start = name;
		while(*start == ' ') {
			start++;
		}
		while(*start) {
			*name = *start;
			name++;
			start++;
		}
		*name = 0;
	}
	free(unordered_names);
	return 1;
}

int read_graphic_set(
		const char* graphic_set_path, // PATH_ROOT / import[game]
		struct Data** vgagr_s0,
		struct Data** vgagr_s1,
		struct Data** vgaspec,
		void* ground_data,
		void* level) {
	if (!vgagr_s0 || !vgagr_s1 || !vgaspec) {
		return 0;
	}
	*vgagr_s0 = 0;
	*vgagr_s1 = 0;
	*vgaspec = 0;
	char groundname[64];
	char vgrgrname[64];
	char vgaspecname[64];
	FILE* ground_file = 0;
	FILE* vgagr_file = 0;
	if (((u8*)level)[26] || ((u8*)level)[28]) {
		// invalid graphic set or invalid extended set
		return 0;
	}

	/* read graphic set */
	sprintf(groundname,"%s/GROUND%uO.DAT",graphic_set_path,((u8*)level)[27]);
	sprintf(vgrgrname,"%s/VGAGR%u.DAT",graphic_set_path,((u8*)level)[27]);
	ground_file = fopen(groundname,"rb");
	if (!ground_file) {
		return 0;
	}
	if (fread(ground_data,1,1056,ground_file) != 1056) {
		fclose(ground_file);
		return 0;
	}
	fclose(ground_file);
	vgagr_file = fopen(vgrgrname,"rb");
	if (!vgagr_file) {
		return 0;
	}
	/* read vgagr sections */
	*vgagr_s0 = decompress_cur_section(vgagr_file);
	if (!*vgagr_s0) {
		return 0; // error
	}
	*vgagr_s1 = decompress_cur_section(vgagr_file);
	if (!*vgagr_s1) {
		free(*vgagr_s0);
		*vgagr_s0 = 0;
		return 0; // error
	}
	fclose(vgagr_file);
	/* vgaspec? */
	if (((u8*)level)[29]) {
		sprintf(vgaspecname,"%s/VGASPEC%u.DAT",
				graphic_set_path,((u8*)level)[29]-1);
		FILE* extended_file = fopen(vgaspecname,"rb");
		*vgaspec = decompress_cur_section(extended_file);
		fclose(extended_file);
		if (!*vgaspec) {
			free(*vgagr_s0);
			free(*vgagr_s1);
			*vgagr_s0 = 0;
			*vgagr_s1 = 0;
			return 0;
		}
		if ((*vgaspec)->size < 40) {
			free(*vgagr_s0);
			free(*vgagr_s1);
			free(*vgaspec);
			*vgagr_s0 = 0;
			*vgagr_s1 = 0;
			*vgaspec = 0;
			return 0;
		}
	}
	return 1;
}

int read_level_from_compressed_file(
		u8 game,
		u8 id,
		void* level) {
	u8 levelposition = import[game].level_position[id];
	u8 level_file = (levelposition & 0x78) >> 3;
	u8 level_nr = (levelposition & 0x07);
	u8 modified = ((levelposition & 0x80)?1:0);
	char levelfilename[64];
	FILE* levelfile = 0;
	struct Data* dec = 0;
	sprintf(levelfilename,"%s/%s/%s%03d.DAT",
			PATH_ROOT,import[game].path,import[game].level_dat_prefix,level_file);
	levelfile = fopen(levelfilename,"rb");
	goto_section(levelfile, level_nr);
	dec = decompress_cur_section(levelfile);
	fclose(levelfile);
	if (!dec) {
		return 0;
	}
	if (dec->size != 2048) {
		free(dec);
		return 0;
	}
	if (modified) {
		/* apply modifier */
		char oddtable_fn[64];
		sprintf(oddtable_fn,"%s/%s/ODDTABLE.DAT",PATH_ROOT,import[game].path);
		FILE* oddtable = fopen(oddtable_fn,"rb");
		if (oddtable) {
			fseek(oddtable,56*(u16)(level_file*8+level_nr),SEEK_SET);
			fread(dec->data,1,24,oddtable);
			fread(dec->data+2016,1,32,oddtable);
			fclose(oddtable);
		}else{
			// error
			free(dec);
			return 0;
		}
	}
	memcpy(level,dec->data,2048);
	free(dec);
	return 1;
}


int parse_level(
		void* level,
		void* ground_data,
		struct Data* vgagr_s0,
		struct Data* vgagr_s1,
		struct Data* vgaspec,
		const u32* ingame_palette,
		u8 ABBA_order,
		u8 players,
		struct Level* output) {
	int i;
	if (!vgagr_s0 || !vgagr_s1 || !ground_data || !level || !output) {
		return 0;
	}
	u8 swap_exits = 0;
	if (players > 1) {
		swap_exits = (ABBA_order?1:0);
		ABBA_order = 0;
	}
	memset(output,0,sizeof(struct Level));
	output->num_players = players;

	u8* terrain = output->terrain;
	struct ObjectType** object_types = output->object_types;
	struct ObjectInstance* objects = output->object_instances;
	u8* object_map = output->object_map;
	u32* palette = output->palette;
	for (i=0;i<7;i++) {
		palette[i] = ingame_palette[i];
	}
	if (settings.amiga_background) {
		palette[0] = AMIGA_BACKGROUND;
	}
	struct Image* terrain_img[64];
	if (!parse_vgagr(
			ground_data,
			vgagr_s0,
			vgagr_s1,
			palette,
			terrain_img,
			object_types)) {
		return 0;
	}

	for (i=0; i<output->num_players; i++) {
		output->player[i].x_pos = ((u8*)level)[24];
		output->player[i].x_pos <<= 8;
		output->player[i].x_pos |= ((u8*)level)[25];
		if (output->num_players > 1) {
			if (output->player[i].x_pos > 96) {
				output->player[i].x_pos -= 96;
			}else{
				output->player[i].x_pos = 0;
			}
		}
		if (output->player[i].x_pos > 1264) {
			output->player[i].x_pos = 1264;
		}
		if (output->player[i].x_pos % 8 >= 4) {
			output->player[i].x_pos -= output->player[i].x_pos%8;
			output->player[i].x_pos+=8;
		}else{
			output->player[i].x_pos -= output->player[i].x_pos%8;
		}
		output->player[i].max_lemmings = ((u8*)level)[3];
		int j;
		for (j=0;j<8;j++) {
			output->player[i].skills[j] = ((u8*)level)[9+2*j];
		}
	}

	output->cur_rate = (output->rate = ((u8*)level)[1]);
	output->fade_in = FADE_IN_DOSFRAMES;
	output->next_lemming_countdown = 20;
	output->percentage_needed = ((u16)((u8*)level)[5])*100 / (u16)output->player[0].max_lemmings;
	output->frames_left = ((u8*)level)[7] * 60 * FPS;
	if (((u8*)level)[30] == 0xFF && ((u8*)level)[31] == 0xFF) {
		output->speed_up = 1;
	}
	memcpy(output->name,((u8*)level)+2016,32);
	// cut off level name at the end (not at the beginning, because we want to keep original alignment of level name)
	i = 32;
	while (i > 0 && output->name[i-1] == ' ') {
		i--;
	}
	output->name[i] = 0;


	if (vgaspec) {
		// parse extended graphic set
		// read palette... ignore first entry (which should be 0x00)
		// TODO: overwrite palette[0] with first entry??
		palette[8] = 0x007C78;
		for (i=1;i<8;i++) {
			palette[i+8] = (u32)((u8)vgaspec->data[3*i+2])*255/63;
			palette[i+8]<<=8;
			palette[i+8] |= ((u32)((u8)vgaspec->data[3*i+1]))*255/63;
			palette[i+8]<<=8;
			palette[i+8] |= ((u32)((u8)vgaspec->data[3*i+0]))*255/63;
		}
		palette[7] = palette[8];

		s8* chunk = (s8*)malloc(19200);
		if (!chunk) {
			return 0;
		}
		memset(chunk,0,19200);
		u8* chunk_img = (u8*)malloc(38400*sizeof(u8));
		if (!chunk_img) {
			free(chunk);
			return 0;
		}
		u16 chunk_offset = 40;
		for (i=0;i<4;i++) { //iterate over chunks
			int j;
			if (vgaspec->size <= chunk_offset ){
				break;
			}
			chunk_offset += decompress_rle(vgaspec->data+chunk_offset, chunk, 14400, vgaspec->size-chunk_offset);
			for (j=0;j<4800;j++) {
				// 0->0, 1->9, 2->10, ..., 7->15
				chunk[14400+j] = (chunk[j] | chunk[4800+j] | chunk[9600+j]);
			}
			while ((u8)vgaspec->data[chunk_offset] != 0x80
					&& chunk_offset < vgaspec->size) {
				chunk_offset++;
			}
			chunk_offset++;

			if (planar_image_to_pixmap(chunk_img, chunk, 38400, 0)) {
				// copy image
				u16 x,y;
				for (y=0;y<40;y++) {
					for (x=0;x<960;x++) {
						// position at 304
						terrain[x+304+1584*(y+40*i)] = chunk_img[x+960*y];
					}
				}
			}
		}
		free(chunk_img);
		free(chunk);
	} else {
		/* draw graphic set terrain objects */
		for (i=0;i<400;i++) {
			u16 x_pos_tmp = ((u8*)level)[0x120+4*i];
			x_pos_tmp <<= 8;
			x_pos_tmp |= ((u8*)level)[0x120+4*i+1];
			if (x_pos_tmp == 0xFFFF) {
				//continue;
				break;
			}
			u8 flags = (u8)((s16)(x_pos_tmp >> 12));
			s16 x_pos = (u16)(x_pos_tmp & 0x7FF);
			x_pos -= 16;
			s16 y_pos = ((u8*)level)[0x120+4*i+2];
			y_pos <<= 1;
			y_pos += ((((u8*)level)[0x120+4*i+3]&0x80)?1:0);
			if (y_pos >= 0x100) {
				y_pos -= 516;
			} else {
				y_pos -= 4;
			}
			u8 terrain_id = ((u8*)level)[0x120+4*i+3] & 0x3F;
			// if (((u8*)level)[0x120+4*i+3] & 0x40) { TODO: what does this cause? }
			if (terrain_img[terrain_id] == 0) {
				continue;
			}
			s16 x,y;
			for (y=0;y<terrain_img[terrain_id]->height;y++) {
				s16 draw_y = ((flags&0x4)?(y_pos-y+terrain_img[terrain_id]->height-1):(y_pos+y));
				if (draw_y<0 || draw_y>=160) {
					continue;
				}
				for (x=0;x<terrain_img[terrain_id]->width;x++) {
					if (x_pos+x<0 || x_pos+x>=1584) {
						continue;
					}
					u8 color = terrain_img[terrain_id]->data[x+(terrain_img[terrain_id]->width)*y];
					if (color & 0xF0) {
						if (!(flags&0x8) || !(terrain[(x_pos+x)+1584*draw_y]&0xF0)) {
							// TODO: what does the combination of flags (0x2 | 0x8) really do? Why is it used , e.g. in LEVEL002.DAT, section 2?
							terrain[(x_pos+x)+1584*draw_y] = ((flags&0x2)?((flags&0x8)?color:0x0):color);
						}
					}
				}
			}
		}
	}
	free_terrain(terrain_img);

	// read STEEL - TODO: also for VGASPEC levels? TODO: break after empty object (4 times 0x00)?
	for (i=0;i<32;i++) {
		s16 x_pos = (s16)(((u8*)level)[0x760+4*i]) * 2 + ((((u8*)level)[0x760+4*i+1]&0x80)?1:0) - 4;
		s16 y_pos = (((u8*)level)[0x760+4*i+1] & 0x7F);
		s16 width = ((((u8*)level)[0x760+4*i+2]) >> 4)+1;
		s16 height = (((u8*)level)[0x760+4*i+2] & 0x0F) + 1;
		s16 x,y;
		for (y=0;y<height;y++) {
			if (y_pos + y < 0 || y_pos + y >= 160/4) {
				continue;
			}
			for (x=0;x<width;x++) {
				if (x_pos + x < 0 || x_pos + x >= 1584/4) {
					continue;
				}
				object_map[x_pos + x + (1584/4)*(y_pos + y)] = 9; // TODO: use OBJECT_STEEL
			}
		}
	}

	int num_entrances = 0;
	u8 player_exit = swap_exits; // player the next exit belongs to
	for (i=0;i<32;i++) {
		// read object

		u16 modifier = ((u8*)level)[0x20+8*i+6];
		modifier <<= 8;
		modifier |= ((u8*)level)[0x20+8*i+7];

		if ((modifier & 0xF) != 0xF) {
			//break;
			continue; // unset
		}
		if (((u8*)level)[0x20+8*i+5] > 0x0F || ((u8*)level)[0x20+8*i+4] != 0) {
			// error!
			continue;
		}

		objects[i].modifier =
				OBJECT_USED |
				((modifier & 0x80)?OBJECT_UPSIDE_DOWN:0) | 
				((modifier & 0x8000)?OBJECT_DONT_OVERWRITE:0) | 
				((modifier & 0x4000)?OBJECT_REQUIRE_TERRAIN:0);
		objects[i].type = ((u8*)level)[0x20+8*i+5] & 0x0F;
		objects[i].current_frame = 0;
		if (object_types[objects[i].type]) {
			// TODO: use start_frame instead?
			objects[i].current_frame = object_types[objects[i].type]->preview_frame;
		}


		s16 x_pos = ((u8*)level)[0x20+8*i];
		x_pos <<= 8;
		x_pos |= ((u8*)level)[0x20+8*i+1];
		if (!x_pos) {
			objects[i].modifier = 0;
		}
		x_pos -= 16;
		s16 y_pos = ((u8*)level)[0x20+8*i+2];
		y_pos <<= 8;
		y_pos |= ((u8*)level)[0x20+8*i+3];

		x_pos -= x_pos%8 - 8*((x_pos / 4)%2);

		objects[i].x = x_pos;
		objects[i].y = y_pos;

		if (objects[i].type == 1) {
			// handle entrance
			switch (num_entrances) {
				int j;
				case 0:
					for (j=0;j<4;j++) {
						output->entrances[j].x=objects[i].x + 25;
						output->entrances[j].y=objects[i].y + 14;
					}
					num_entrances++;
					break;
				case 1:
					if (ABBA_order && output->num_players == 1) {
						for (j=1;j<3;j++) {
							output->entrances[j].x=objects[i].x + 25;
							output->entrances[j].y=objects[i].y + 14;
						}
					}else{
						for (j=0;j<2;j++) {
							output->entrances[1+2*j].x=objects[i].x + 25;
							output->entrances[1+2*j].y=objects[i].y + 14;
						}
					}
					num_entrances++;
					break;
				case 2:
					if (ABBA_order && output->num_players == 1) {
						output->entrances[3].x = output->entrances[2].x;
						output->entrances[3].y = output->entrances[2].y;
					}
					output->entrances[2].x=objects[i].x + 25;
					output->entrances[2].y=objects[i].y + 14;
					num_entrances++;
					break;
				case 3:
					output->entrances[3].x=objects[i].x + 25;
					output->entrances[3].y=objects[i].y + 14;
					break;
				default:
					break;
			}
		} else {
			// TODO: UPSIDE-DOWN-OBJECTS: CHANGE TRIGGER AREA?????
			u8 trigger = object_types[objects[i].type]->trigger;
			s16 trig_x = object_types[objects[i].type]->trigger_x;
			s16 trig_y = object_types[objects[i].type]->trigger_y;
			s16 trig_w = object_types[objects[i].type]->trigger_width;
			s16 trig_h = object_types[objects[i].type]->trigger_height;
			trig_w = (trig_w?trig_w:256);
			trig_h = (trig_h?trig_h:256);
			trig_y--; // TODO: really?
			if (trigger) {
				trigger &= 0x0F;
				if (trigger == OBJECT_TRAP) {
					if (i<16) {
						trigger |= i<<4;
					}else{
						trigger = 0;
					}
				}
				if (trigger == OBJECT_EXIT) {
					trigger |= (player_exit++)<<4;
					if (players) {
						player_exit %= players;
					}else{
						player_exit = 0;
					}
				}
			}
			if (trigger) {
				s16 wi, hi;
				for (hi = 0; hi < trig_h; hi++) {
					if (objects[i].y/4 + trig_y + hi < 0) {
						continue;
					}
					if (objects[i].y/4 + trig_y + hi >= 160/4) {
						break;
					}
					for (wi = 0; wi < trig_w; wi++) {
						if (objects[i].x/4 + trig_x + wi < 0) {
							continue;
						}
						if (objects[i].x/4 + trig_x + wi >= 1584/4) {
							break;
						}
						object_map[
								objects[i].x/4 + trig_x + wi +
								(1584/4) * (objects[i].y/4 + trig_y + hi)] =
										trigger;
					}
				}
			}
		}
	}
	return 1; //success

}


int read_level(
		u8 game,
		u8 id,
		void* level,
		void* ground_data,
		struct Data** vgagr_s0,
		struct Data** vgagr_s1,
		struct Data** vgaspec) {
	if (!vgagr_s0 || !vgagr_s1 || !vgaspec || !ground_data || !level) {
		return 0; // error
	}
	if (!read_level_from_compressed_file(
			game,
			id,
			level)) {
		return 0;
	}
	char graphic_set_path[64];
	sprintf(graphic_set_path,"%s/%s",PATH_ROOT,import[game].path);
	return read_graphic_set(
			graphic_set_path,
			vgagr_s0,
			vgagr_s1,
			vgaspec,
			ground_data,
			level);
}

u8 count_custom_levels(const char* path) {
	if (!path) {
		return 0;
	}
	u8 cnt;
	char levelfilename[64];
	for (cnt = 1; cnt < 100; cnt++) {
		sprintf(levelfilename,"%s/%s/%02u.lvl",
				PATH_ROOT,path,cnt);
		FILE* in = fopen(levelfilename, "rb");
		if (!in) {
			break;
		}
		fclose(in);
	}
	return cnt-1;
}

int read_level_file(
		const char* filename,
		const char* ressource_path,
		void* level,
		void* ground_data,
		struct Data** vgagr_s0,
		struct Data** vgagr_s1,
		struct Data** vgaspec) {
	if (!level || !ground_data || !vgagr_s0 || !vgagr_s1 || !vgaspec) {
		return 0; // error
	}
	FILE* levelfile = fopen(filename, "rb");
	if (!levelfile) {
		return 0;
	}
	if (2048 != fread(level, 1, 2048, levelfile)) {
		fclose(levelfile);
		return 0;
	}
	fclose(levelfile);
	return read_graphic_set(
			ressource_path,
			vgagr_s0,
			vgagr_s1,
			vgaspec,
			ground_data,
			level);
}


struct Level* init_level_from_dat(u8 game, u8 lvl, char* level_id) {
	struct Level* level = (struct Level*)malloc(sizeof(struct Level));
	if (!level){
		return 0; // error
	}
	memset(level,0,sizeof(struct Level));
	void* level_data = malloc(2048);
	if (!level_data) {
		free(level);
		return 0; // error
	}
	void* ground_data = malloc(1056);
	if (!ground_data) {
		free(level);
		free(level_data);
		return 0; // error
	}
	struct Data* vgagr_s0 = 0;
	struct Data* vgagr_s1 = 0;
	struct Data* vgaspec = 0;

	if (!read_level(
			game,
			lvl,
			level_data,
			ground_data,
			&vgagr_s0,
			&vgagr_s1,
			&vgaspec)) {
		free(level);
		free(level_data);
		free(ground_data);
		return 0; // error
	}
	u8 res = parse_level(
			level_data,
			ground_data,
			vgagr_s0,
			vgagr_s1,
			vgaspec,
			import[game].ingame_palette,
			import[game].ABBA_order,
			1,
			level);
	if (vgagr_s0) {
		free(vgagr_s0);
		vgagr_s0 = 0;
	}
	if (vgagr_s1) {
		free(vgagr_s1);
		vgagr_s1 = 0;
	}
	if (vgaspec) {
		free(vgaspec);
		vgaspec = 0;
	}
	free(level_data);
	level_data = 0;
	free(ground_data);
	ground_data = 0;
	if (!res) {
		free_objects(level->object_types);
		free(level);
		return 0; // error
	}

	// write level id, e.g. FUN13
	if (level_id) {
		u8 level_no;
		if (import[game].num_of_level_per_difficulty > 1) {
			level_no = (lvl%import[game].num_of_level_per_difficulty)+1;
		}else{
			level_no = lvl+1;
		}
		sprintf(
				level_id,
				"%s%2u",
				import[game].difficulties[lvl/import[game].num_of_level_per_difficulty],
				level_no);
	}
	prepare_music(game, lvl);
	return level;
}
