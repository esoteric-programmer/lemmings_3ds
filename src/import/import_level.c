#include <malloc.h>
#include <string.h>
#include <3ds.h>
#include "import_level.h"
#include "gamespecific.h"
#include "decode.h"
#include "import.h"
#include "import_ground.h"
#include "settings.h"

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

int read_vgagr(u8 game, FILE* ground_file, FILE* vgagr_file, struct Palette* palette, struct Image* terrain_img[64], struct Object* objects[16]) {
	struct GroundInfo info;
	int ter;
	int obj;
	int i;

	if (!ground_file || !vgagr_file || !palette || ! terrain_img || !objects) {
		return 0; // error
	}

	memset(terrain_img,0,64*sizeof(void*));
	memset(objects, 0,16*sizeof(void*));
	for (i=0;i<7;i++) {
		palette->vga[i] = import[game].ingame_palette[i];
		palette->vga_preview[i] = palette->vga[i]; // TODO: correct?
	}
	palette->ega[0] = ega2rgb(0x00); // black
	palette->ega[1] = ega2rgb(0x39); // blue
	palette->ega[2] = ega2rgb(0x02); // green
	palette->ega[3] = ega2rgb(0x3F); // white
	palette->ega[4] = ega2rgb(0x3E); // yellow
	palette->ega[5] = ega2rgb(0x3C); // red
	palette->ega[6] = ega2rgb(0x07); // grey
	for (i=0;i<7;i++) {
		palette->ega_preview[i] = palette->ega[i]; // TODO: correct?
	}

	read_ground_data(&info, ground_file);
	struct Data* dec = decompress_cur_section(vgagr_file);
	if (!dec) {
		return 0; // error
	}

	for (i=0;i<8;i++) {
		palette->vga[i+8] = info.palette.vga_custom[i];
	}
	palette->vga[7] = palette->vga[8];
	for (i=0;i<8;i++) {
		palette->vga_preview[i+8] = info.palette.vga_preview[i];
	}
	palette->vga_preview[7] = palette->vga_preview[8];
	for (i=0;i<8;i++) {
		palette->ega[i+8] = ega2rgb(info.palette.ega_custom[i]);
	}
	palette->ega[7] = palette->ega[8];
	for (i=0;i<8;i++) {
		palette->ega_preview[i+8] = ega2rgb(info.palette.ega_preview[i]);
	}
	palette->ega_preview[7] = palette->ega_preview[8];

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
		read_image(img_length, info.terrain_info[ter].mask_loc - info.terrain_info[ter].image_loc,dec->data + info.terrain_info[ter].image_loc, terrain_img[ter]->data);
	}

	free(dec);
	dec = decompress_cur_section(vgagr_file);
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
		objects[obj] = (struct Object*)malloc(sizeof(struct Object)+frames*img_length*sizeof(u32));
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
		objects[obj]->preview_frame = (info.object_info[obj].preview_image_index - info.object_info[obj].animation_frames_base_loc) / data_size;
		if (objects[obj]->preview_frame > objects[obj]->end_frame) {
			objects[obj]->preview_frame = 0;
		}
		for (i=0;i<frames;i++) {
			// TODO: test whether data->size is large enough
			read_image(
					img_length,
					info.object_info[obj].mask_offset_from_image,
					dec->data + info.object_info[obj].animation_frames_base_loc + i*data_size,
					objects[obj]->data + i*img_length);
		}
	}

	free(dec);
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

void free_objects(struct Object* objects[16]) {
	int i;
	for (i=0;i<16;i++) {
		if (objects[i] != 0) {
			free(objects[i]);
			objects[i] = 0;
		}
	}
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
	for (j=0;j<(int)import[game].num_of_difficulties * (int)import[game].num_of_level_per_difficulty; j++) {
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


int read_level(u8 game, u8 id, struct Level* level) {
	u8 levelposition = import[game].level_position[id];
	u8 level_file = (levelposition & 0x78) >> 3;
	u8 level_nr = (levelposition & 0x07);
	u8 modified = ((levelposition & 0x80)?1:0);
	char levelfilename[64];
	FILE* levelfile = 0;
	FILE* ground_file = 0;
	FILE* vgagr_file = 0;
	struct Data* dec = 0;
	int i;
	char groundname[64];
	char vgrgrname[64];
	struct Image* terrain_img[64];
	u16 graphic_set = 0;
	u16 extended_graphic_set = 0;

	if (!level) {
		return 0; // error
	}
	memset(level,0,sizeof(struct Level));

	u8* terrain = level->terrain;
	struct Object** terrain_obj = level->o;
	struct ObjectInstance* objects = level->obj;
	struct Palette* palette = &(level->palette);
	struct Entrances* entrances = &(level->entrances);
	u8* object_map = level->object_map;

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
		// apply modifier
		// TODO: check whether modifier can be applied? (only original)
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


	if (dec->data[26] != 0 || (u8)dec->data[27] > 4) {
		// invalid graphic set
		free(dec);
		return 0; // error
	}
	if ((u8)dec->data[28] != 0 || (u8)dec->data[29] > 4) {
		// invalid extended graphic set
		free(dec);
		return 0; // error
	}
	graphic_set = dec->data[27];
	extended_graphic_set = dec->data[29];

	level->info.x_pos = ((u8)dec->data[24]);
	level->info.x_pos <<= 8;
	level->info.x_pos |= ((u8)dec->data[25]);
	if (level->info.x_pos > 1264) {
		level->info.x_pos = 1264;
	}
	if (level->info.x_pos % 8 >= 4) {
		level->info.x_pos -= level->info.x_pos%8;
		level->info.x_pos++;
	}else{
		level->info.x_pos -= level->info.x_pos%8;
	}

	level->info.rate = dec->data[1];
	level->info.lemmings = dec->data[3];
	level->info.to_rescue = dec->data[5];
	level->info.percentage_needed = ((u16)level->info.to_rescue)*100 / (u16)level->info.lemmings;
	level->info.minutes = dec->data[7];
	for (i=0;i<8;i++) {
		level->info.skills[i] = dec->data[9+2*i];
	}
	memcpy(level->info.name,dec->data+2016,32);
	// cut off level name at the end (not at the beginning, because we want to keep original alignment of level name)
	i = 32;
	while (i > 0 && level->info.name[i-1] == ' ') {
		i--;
	}
	level->info.name[i] = 0;


	/* read graphic set */
	sprintf(groundname,"%s/%s/GROUND%dO.DAT",PATH_ROOT,import[game].path,graphic_set);
	sprintf(vgrgrname,"%s/%s/VGAGR%d.DAT",PATH_ROOT,import[game].path,graphic_set);
	ground_file = fopen(groundname,"rb");
	if (!ground_file) {
		free(dec);
		return 0;
	}
	vgagr_file = fopen(vgrgrname,"rb");
	if (!vgagr_file) {
		fclose(ground_file);
		free(dec);
		return 0;
	}
	if (!read_vgagr(game, ground_file, vgagr_file, palette, terrain_img, terrain_obj)) {
		// error reading level
		fclose(ground_file);
		fclose(vgagr_file);
		free(dec);
		return 0;
	}
	fclose(ground_file);
	fclose(vgagr_file);


	if (extended_graphic_set != 0) {
		struct Data* extended = 0;
		// decompress extended graphic set
		// TODO: only allowed for original lemmings!! -> check
		sprintf(levelfilename,"%s/%s/VGASPEC%d.DAT",
				PATH_ROOT,import[game].path,extended_graphic_set-1);
		FILE* extended_file = fopen(levelfilename,"rb");
		extended = decompress_cur_section(extended_file);
		fclose(extended_file);
		if (!extended) {
			free(dec);
			return 0;
		}
		if (extended->size < 40) {
			free(dec);
			return 0;
		}
		// read palette... ignore first entry (which should be 0x00)
		// TODO: overwrite palette[0] with first entry??
		palette->vga[8] = 0x007C78; // correct for all levels. but really hard-coded? TODO: EGA?
		palette->vga_preview[8] = palette->vga[8];
		for (i=1;i<8;i++) {
			palette->vga[i+8] = (u32)((u8)extended->data[3*i+2])*255/63;
			palette->vga[i+8]<<=8;
			palette->vga[i+8] |= ((u32)((u8)extended->data[3*i+1]))*255/63;
			palette->vga[i+8]<<=8;
			palette->vga[i+8] |= ((u32)((u8)extended->data[3*i+0]))*255/63;
			palette->vga_preview[i+8] = palette->vga[i+8];
		}
		palette->vga[7] = palette->vga[8];
		palette->vga_preview[7] = palette->vga_preview[8];
		for (i=1;i<8;i++) {
			palette->ega[i+8] = ega2rgb(extended->data[i+24]);
		}
		for (i=1;i<8;i++) {
			palette->ega_preview[i+8] = ega2rgb(extended->data[i+32]);
		}
		s8* chunk = (s8*)malloc(19200);
		if (!chunk) {
			free(dec);
			free(extended);
			return 0;
		}
		memset(chunk,0,19200);
		u8* chunk_img = (u8*)malloc(38400*sizeof(u8));
		if (!chunk_img) {
			free(dec);
			free(extended);
			free(chunk);
			return 0;
		}
		u16 chunk_offset = 40;
		for (i=0;i<4;i++) { //iterate over chunks
			int j;
			if (extended->size <= chunk_offset ){
				break;
			}
			chunk_offset += decompress_rle(extended->data+chunk_offset, chunk, 14400, extended->size-chunk_offset);
			for (j=0;j<4800;j++) {
				// 0->0, 1->9, 2->10, ..., 7->15
				chunk[14400+j] = (chunk[j] | chunk[4800+j] | chunk[9600+j]);
			}
			while ((u8)extended->data[chunk_offset] != 0x80
					&& chunk_offset < extended->size) {
				chunk_offset++;
			}
			chunk_offset++;

			if (read_image(38400, 0, chunk, chunk_img)) {
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
		free(extended);
		extended = 0;
	} else {
		/* draw graphic set terrain objects */
		for (i=0;i<400;i++) {
			u16 x_pos_tmp = (u8)dec->data[0x120+4*i];
			x_pos_tmp <<= 8;
			x_pos_tmp |= (u8)dec->data[0x120+4*i+1];
			if (x_pos_tmp == 0xFFFF) {
				//continue;
				break;
			}
			u8 flags = (u8)((s16)(x_pos_tmp >> 12));
			s16 x_pos = (u16)(x_pos_tmp & 0x7FF);
			x_pos -= 16;
			s16 y_pos = (u8)dec->data[0x120+4*i+2];
			y_pos <<= 1;
			y_pos += ((dec->data[0x120+4*i+3]&0x80)?1:0);
			if (y_pos >= 0x100) {
				y_pos -= 516;
			} else {
				y_pos -= 4;
			}
			u8 terrain_id = dec->data[0x120+4*i+3] & 0x3F;
			// if (dec->data[0x120+4*i+3] & 0x40) { TODO: what does this cause? }
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
		s16 x_pos = (s16)((u8)dec->data[0x760+4*i]) * 2 + ((dec->data[0x760+4*i+1]&0x80)?1:0) - 4;
		s16 y_pos = (dec->data[0x760+4*i+1] & 0x7F);
		s16 width = (((u8)dec->data[0x760+4*i+2]) >> 4)+1;
		s16 height = (dec->data[0x760+4*i+2] & 0x0F) + 1;
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
	for (i=0;i<32;i++) {
		// read object

		u16 modifier = (u8)dec->data[0x20+8*i+6];
		modifier <<= 8;
		modifier |= (u8)dec->data[0x20+8*i+7];

		if ((modifier & 0xF) != 0xF) {
			//break;
			continue; // unset
		}
		if ((u8)dec->data[0x20+8*i+5] > 0x0F || dec->data[0x20+8*i+4] != 0) {
			// error!
			continue;
		}

		objects[i].modifier =
				OBJECT_USED |
				((modifier & 0x80)?OBJECT_UPSIDE_DOWN:0) | 
				((modifier & 0x8000)?OBJECT_DONT_OVERWRITE:0) | 
				((modifier & 0x4000)?OBJECT_REQUIRE_TERRAIN:0);
		objects[i].type = dec->data[0x20+8*i+5] & 0x0F;
		objects[i].current_frame = 0;
		if (terrain_obj[objects[i].type]) {
			// TODO: use start_frame instead?
			objects[i].current_frame = terrain_obj[objects[i].type]->preview_frame;
		}


		s16 x_pos = dec->data[0x20+8*i];
		x_pos <<= 8;
		x_pos |= (u8)dec->data[0x20+8*i+1];
		if (!x_pos) {
			objects[i].modifier = 0;
		}
		x_pos -= 16;
		s16 y_pos = dec->data[0x20+8*i+2];
		y_pos <<= 8;
		y_pos |= (u8)dec->data[0x20+8*i+3];

		x_pos -= x_pos%8 - 8*((x_pos / 4)%2);

		objects[i].x = x_pos;
		objects[i].y = y_pos;

		if (objects[i].type == 1) {
			// handle entrance
			switch (num_entrances) {
				int j;
				case 0:
					for (j=0;j<4;j++) {
						entrances->pos[j].x=objects[i].x + 24;
						entrances->pos[j].y=objects[i].y + 14;
					}
					num_entrances++;
					break;
				case 1:
					if (import[game].ABBA_order) {
						for (j=1;j<3;j++) {
							entrances->pos[j].x=objects[i].x + 24;
							entrances->pos[j].y=objects[i].y + 14;
						}
					}else{
						for (j=0;j<2;j++) {
							entrances->pos[1+2*j].x=objects[i].x + 24;
							entrances->pos[1+2*j].y=objects[i].y + 14;
						}
					}
					num_entrances++;
					break;
				case 2:
					if (import[game].ABBA_order) {
						entrances->pos[3].x = entrances->pos[2].x;
						entrances->pos[3].y = entrances->pos[2].y;
					}
					entrances->pos[2].x=objects[i].x + 24;
					entrances->pos[2].y=objects[i].y + 14;
					num_entrances++;
					break;
				case 3:
					entrances->pos[3].x=objects[i].x + 24;
					entrances->pos[3].y=objects[i].y + 14;
					break;
				default:
					break;
			}
		} else {
			// TODO: UPSIDE-DOWN-OBJECTS: CHANGE TRIGGER AREA?????
			u8 trigger = terrain_obj[objects[i].type]->trigger;
			s16 trig_x = terrain_obj[objects[i].type]->trigger_x;
			s16 trig_y = terrain_obj[objects[i].type]->trigger_y;
			s16 trig_w = terrain_obj[objects[i].type]->trigger_width;
			s16 trig_h = terrain_obj[objects[i].type]->trigger_height;
			trig_w = (trig_w?trig_w:256);
			trig_h = (trig_h?trig_h:256);
			trig_y--; // TODO: really?
			if (trigger) {
				trigger &= 0x0F;
				if (trigger == 4) {
					if (i<16) {
						trigger |= i<<4;
					}else{
						trigger = 0;
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
	free(dec);
	return 1; //success
}

void init_level_state(struct LevelState* state, struct Level* level) {
	if (!state || !level) {
		return;
	}
	state->frames_left = level->info.minutes * 60 * FPS;
	state->paused = 0;
	state->cur_rate = level->info.rate;
	state->selected_skill = 0;
	state->cursor.x = 142; // TODO: SCREEN_WIDTH/2 - 18 ??
	state->cursor.y = 92;
	state->entrances_open = 0;
}
