#include "draw.h"
#include "highperf_font.h"
#include <string.h>
#include <stdlib.h>
#include "lemming.h"
#include "particles.h"

void draw_lemmings_minimap(struct Lemming[MAX_NUM_OF_LEMMINGS], struct MainInGameData* data, struct RGB_Image* image);

void draw_single_lemming(struct Lemming* lem, struct Image* lemmings_anim[337], struct Image* masks[23], u32 palette[16], struct RGB_Image* image, u16 x_offset);

int draw(struct RGB_Image* image, u32 palette[16], const u8* img, s16 x, s16 y, u16 w, u16 h) {
	if (!image || !palette || !img) {
		return 0; // error
	}
	s16 xi, yi;
	for (yi = 0; yi<h; yi++) {
		s16 draw_y = yi + y;
		if (draw_y < 0 || draw_y >= image->height) {
			continue;
		}
		for (xi = 0; xi<w; xi++) {
			s16 draw_x = xi + x;
			if (draw_x < 0 || draw_x >= image->width) {
				continue;
			}
			if (img[xi + yi*(s16)w] & 0xF0) {
				image->data[draw_x + draw_y*image->width] = palette[img[xi + yi*(s16)w] & 0x0F] | 0xFF000000;
			}
		}
	}
	return 1;
}


// help function for interpolation
static inline u32 get_value(struct RGB_Image* image, float x1, float y1, float x2, float y2) {
	if (!image) {
		return 0;
	}
	if (x1 > x2) {
		float tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if (y1 > y2) {
		float tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	if (x2 < 0 || y2 < 0) {
		return 0;
	}
	s16 start_x = (s16)x1;
	s16 end_x = (s16)x2 + 1;
	s16 start_y = (s16)y1;
	s16 end_y = (s16)y2 + 1;
	if (start_x >= image->width || start_y >= image->height) {
		return 0;
	}
	s16 i,j;
	float weight_sum = 0.0;
	float alpha = 0.0;
	float r = 0.0;
	float g = 0.0;
	float b = 0.0;
	for (j=start_y;j<end_y;j++) {
		for (i=start_x;i<end_x;i++) {
			// calculate weight of pixel
			float x_weight = 1;
			if (i == start_x) {
				x_weight -= (x1 - (float)start_x);
			}
			if (i == end_x) {
				x_weight -= ((float)end_x - x2);
			}
			float y_weight = 1;
			if (j == start_y) {
				y_weight -= (y1 - (float)start_y);
			}
			if (j == end_y) {
				y_weight -= ((float)end_y - y2);
			}
			float weight = x_weight * y_weight;
			if (weight <= 0) {
				continue;
			}
			// add pixel value (if any)
			weight_sum += weight;

			float cur_a = 0.0;
			float cur_r = 0.0;
			float cur_g = 0.0;
			float cur_b = 0.0;
			if (i >= 0 && i < image->width && j >= 0 && j < image->height) {
				u32 px = image->data[i+j*image->width];
				cur_a = ((float)(px >> 24)) / 255.0f;
				cur_r = ((float)((px >> 16) & 0xFF));
				cur_g = ((float)((px >> 8) & 0xFF));
				cur_b = ((float)((px >> 0) & 0xFF));
			}
			alpha += weight * cur_a;
			r += weight * cur_a * cur_r;
			g += weight * cur_a * cur_g;
			b += weight * cur_a * cur_b;
		}
	}
	if (alpha > 0) {
		r /= alpha;
		g /= alpha;
		b /= alpha;
	}else{
		r = 0.0f;
		g = 0.0f;
		b = 0.0f;
	}
	if (weight_sum > 0) {
		alpha /= weight_sum;
	}else{
		alpha = 0.0f;
	}
	alpha *= 255.0f;
	if (r > 255.0f) {
		r = 255.0f;
	}
	if (g > 255.0f) {
		g = 255.0f;
	}
	if (b > 255.0f) {
		b = 255.0f;
	}
	if (alpha > 255.0f) {
		alpha = 255.0f;
	}
	if (r < 0) {
		r = 0;
	}
	if (g < 0) {
		g = 0;
	}
	if (b < 0) {
		b = 0;
	}
	if (alpha < 0) {
		alpha = 0;
	}
	u32 r_i = (u32)r;
	if (r - (float)r_i >= 0.5 && r_i < 255) {
		r_i++;
	}
	u32 g_i = (u32)g;
	if (g - (float)g_i >= 0.5 && g_i < 255) {
		g_i++;
	}
	u32 b_i = (u32)b;
	if (b - (float)b_i >= 0.5 && b_i < 255) {
		b_i++;
	}
	u32 alpha_i = (u32)alpha;
	if (alpha - (float)alpha_i >= 0.5 && alpha_i < 255) {
		alpha_i++;
	}
	return (alpha_i<<24) | (r_i<<16) | (g_i<<8) | b_i;
}

// scale with linear interpolation (sf2d seems o scale without interpolation)
void scale(struct RGB_Image* dest, float factor, struct RGB_Image* source) {
	if (!dest || !source) {
		return;
	}
	s16 x,y;
	for (y=0;y<dest->height;y++) {
		for (x=0;x<dest->width;x++) {
			dest->data[x+y*dest->width] = //source->data[(int)((float)x/factor) + source->width * (int)((float)y/factor)];
					get_value(source, (float)x/factor, (float)y/factor, (float)(x+1)/factor, (float)(y+1)/factor);
		}
	}
}

int draw_level(struct RGB_Image* image, struct Level* level) {
	s16 i,x,y;
	if (!image || !level) {
		return 0;
	}

	if (level->info.x_pos + image->width > 1584) {
		if (image->width >= 1584) {
			level->info.x_pos = 0;
		}else{
			level->info.x_pos = 1584 - image->width;
		}
	}

	for (y=0;y<160 && y<image->height;y++) {
		for (x=0;x<image->width;x++) {
			s16 level_x = x + (s16)level->info.x_pos;
			if (level_x >= 1584) {
				image->data[x+y*image->width] = 0x000000;
			}
			if (level->terrain[level_x+y*1584] & 0xF0) {
				image->data[x+y*image->width] = level->palette.vga[level->terrain[level_x+y*1584] & 0x0F] | 0xFF000000;
			}else{
				image->data[x+y*image->width] = level->palette.vga[0] | 0xFF000000;
			}
		}
	}


	// draw objects into level graphic
	for (i=0;i<32;i++) {
		// process object!
		if (!(level->obj[i].modifier & OBJECT_USED)) {
			continue;
		}
		struct Object* o = level->o[level->obj[i].type];
		if (!o) {
			continue;
		}

		// copy image
		s16 x,y;
		for (y=0;y<o->height;y++) {
			s16 draw_y;
			if (OBJECT_UPSIDE_DOWN & level->obj[i].modifier) {
				draw_y = (s16)level->obj[i].y-y+(s16)o->height-1;
			}else{
				draw_y = y+(s16)level->obj[i].y;
			}
			if (draw_y < 0 || draw_y >= 160 || draw_y >= image->height) {
				continue;
			}
			for (x=0;x<o->width;x++) {
				if (x+level->obj[i].x < 0 || x+level->obj[i].x >= 1584) {
					continue;
				}
				if (level->obj[i].current_frame > o->end_frame) {
					continue;
				}
				s16 draw_x = ((s16)(x+level->obj[i].x)) - ((s16)level->info.x_pos);
				if (draw_x < 0 || draw_x >= image->width) {
					continue;
				}

				u8 color = o->data[level->obj[i].current_frame * ((u32)o->width) * ((u32)o->height) + y * ((u32)o->width) + x];
				if ((color & 0xF0) == 0) {
					continue;
				}
				// TODO: OBJECT_DONT_OVERWRITE -> don't overwrite terrain or don't overwrite anything?

				if ((OBJECT_DONT_OVERWRITE & level->obj[i].modifier) && (level->terrain[x+level->obj[i].x+1584*(draw_y)] & 0xF0)) {
					continue;
				}
				if ((OBJECT_REQUIRE_TERRAIN & level->obj[i].modifier) && !(level->terrain[x+level->obj[i].x+1584*(draw_y)] & 0xF0)) {
					continue;
				}
				if (OBJECT_REQUIRE_TERRAIN & level->obj[i].modifier) {
					//1->5; 3->5; 4->4; ...
					color = 0xF4 | (color&0x1);
				}
				image->data[draw_x+image->width*draw_y] = level->palette.vga[color & 0x0F] | 0xFF000000;
			}
		}
	}
	return 1;
}

void draw_highperf_text(struct RGB_Image* image, struct MainInGameData* data,
		s16 y_offset, const char* text) {
	if (!text || !image || !data) {
		return;
	}
	int i=0;
	s16 x_offset = 0;
	s16 x,y;
	while(1) {

		int id = -1; // unsupported symbol
		const u8* other = 0;
		if (!text[i]) {
			break;
		}
			if (text[i] == '\n') {
				i++;
				x_offset = 0;
				y_offset += 16;
				continue;
			}
			if (text[i] == '%') {
				id = 0;
			}
			if (text[i]>='0' && text[i]<='9') {
				id = text[i]-'0'+1;
			}
			if (text[i] == '-') {
				id = 11;
			}
			if (text[i]>='A' && text[i]<='Z') {
				id = text[i]-'A'+12;
			}
			if (text[i]>='a' && text[i]<='z') {
				id = text[i]-'a'+12;
			}
			if (text[i] == '.') {
				other = highperf_font_dot;
			}
			if (text[i] == ',') {
				other = highperf_font_comma;
			}
			if (text[i] == '?') {
				other = highperf_font_questionmark;
			}
			if (text[i] == '!') {
				other = highperf_font_exclamationmark;
			}
			if (text[i] == ':') {
				other = highperf_font_colon;
			}
			if (text[i] == '(') {
				other = highperf_font_bracket_l;
			}
			if (text[i] == ')') {
				other = highperf_font_bracket_r;
			}
			if (text[i] == '\'' || text[i] == '`') {
				other = highperf_font_apostrophe;
			}
			if (text[i] == '"') {
				other = highperf_font_quote;
			}
			if (x_offset >= 40) {
				i++;
				continue;
			}
		// now draw character with index i
		for (y=0;y<16 && y+y_offset<image->height;y++) {
			for (x=8*x_offset;x<8*(x_offset+1);x++) {
				if (x >= image->width) {
					break;
				}
				s16 screen_x = x + ((s16)image->width-320)/2;
				if (screen_x < 0 || screen_x >= image->width) {
					continue;
				} else {
					// draw character
					if (id == -1) {
						if (other) {
							if (other[8*y + x-8*x_offset] & 0xF0) {
								image->data[screen_x+(y+y_offset)*image->width] = data->high_perf_palette[other[8*y + x-8*x_offset] & 0xF] | 0xFF000000;
							}
						}
					}else{
						if (data->high_perf_font[id*8*16 + 8*y + x-8*x_offset] & 0xF0) {
							image->data[screen_x+(y+y_offset)*image->width] = data->high_perf_palette[data->high_perf_font[id*8*16 + 8*y + x-8*x_offset] & 0xF] | 0xFF000000;
						}
					}
				}
			}
		}
		i++;
		x_offset++;
	}
}


void draw_menu_text(struct RGB_Image* image, struct MainMenuData* data,
		s16 x_offset, s16 y_offset, const char* text) {
	if (!text || !image || !data) {
		return;
	}
	s16 x_pos = 0;
	int i=0;
	s16 x,y;
	while(1) {

		int id = -1; // unsupported symbol
		if (!text[i]) {
			break;
		}
			if (text[i] == '\n') {
				i++;
				x_pos = 0;
				y_offset += 16;
				continue;
			}
			if (text[i] >= 33 && text[i] < 127) {
				id = text[i]-33;
			}
			if (x_pos >= 40) {
				i++;
				continue;
			}
		// now draw character with index i
		for (y=0;y<16 && y+y_offset<image->height;y++) {
			for (x=16*x_pos;x<16*(x_pos+1);x++) {
				if (x+x_offset >= image->width) {
					break;
				}
				s16 screen_x = x_offset + x + ((s16)image->width-320)/2;
				if (screen_x < 0 || screen_x >= image->width) {
					continue;
				} else {
					// draw character
					if (id >= 0) {
						if (data->menu_font[id*16*16 + 16*y + x-16*x_pos] & 0xF0) {
							image->data[screen_x+(y+y_offset)*image->width] = data->palette[data->menu_font[id*16*16 + 16*y + x-16*x_pos] & 0xF] | 0xFF000000;
						}
					}
				}
			}
		}
		i++;
		x_pos++;
	}
}


void draw_menu_text_small(struct RGB_Image* image, struct MainMenuData* data,
		s16 x_offset, s16 y_offset, const char* text) {
	if (!text || !image || !data) {
		return;
	}
	s16 x_pos = 0;
	int i=0;
	s16 x,y;
	while(1) {

		int id = -1; // unsupported symbol
		if (!text[i]) {
			break;
		}
			if (text[i] == '\n') {
				i++;
				x_pos = 0;
				y_offset += 8;
				continue;
			}
			if (text[i] >= 33 && text[i] < 127) {
				id = text[i]-33;
			}
			if (x_pos >= 40) {
				i++;
				continue;
			}
		// now draw character with index i
		for (y=0;y<8 && y+y_offset<image->height;y++) {
			for (x=8*x_pos;x<8*(x_pos+1);x++) {
				if (x+x_offset >= image->width) {
					break;
				}
				s16 screen_x = x_offset + x + ((s16)image->width-320)/2;
				if (screen_x < 0 || screen_x >= image->width) {
					continue;
				} else {
					// draw character
					if (id >= 0) {
						u8 px[4];
						px[0] = data->menu_font[id*16*16 + 16*2*y + 2*(x-8*x_pos)];
						px[1] = data->menu_font[id*16*16 + 16*2*y + 2*(x-8*x_pos)+1];
						px[2] = data->menu_font[id*16*16 + 16*(2*y+1) + 2*(x-8*x_pos)];
						px[3] = data->menu_font[id*16*16 + 16*(2*y+1) + 2*(x-8*x_pos)+1];
						u8 cnt = 0;
						u16 rgb[3] = {0,0,0};
						int i;
						for (i=0;i<4;i++) {
							if (px[i] & 0xF0) {
								cnt++;
								rgb[0] += data->palette[px[i] & 0xF] & 0xFF;
								rgb[1] += (data->palette[px[i] & 0xF]>>8) & 0xFF;
								rgb[2] += (data->palette[px[i] & 0xF]>>16) & 0xFF;
							}
						}
						if (cnt) {
							u16 rgb_old[3] = {
									image->data[screen_x+(y+y_offset)*image->width] & 0xFF,
									(image->data[screen_x+(y+y_offset)*image->width]>>8) & 0xFF,
									(image->data[screen_x+(y+y_offset)*image->width]>>16) & 0xFF
							};
							rgb[0] += rgb_old[0] * (4-cnt);
							rgb[1] += rgb_old[1] * (4-cnt);
							rgb[2] += rgb_old[2] * (4-cnt);
							rgb[0] /= 4;
							rgb[1] /= 4;
							rgb[2] /= 4;
							u32 color = ((u32)rgb[0]) | (((u32)rgb[1])<<8) | (((u32)rgb[2])<<16);
							image->data[screen_x+(y+y_offset)*image->width] = color | 0xFF000000;
						}
					}
				}
			}
		}
		i++;
		x_pos++;
	}
}


int draw_toolbar(struct RGB_Image* image, struct MainInGameData* data,
		struct Level* level, struct LevelState* state,
		struct Lemming lemmings[MAX_NUM_OF_LEMMINGS],
		const char* text) {
	if (!image || !data || !level || !state) {
		return 0;
	}

	s16 x,y,i;
	data->high_perf_palette[7] = level->palette.vga[8];
	for (y=0;y<40 && y+160<image->height;y++) {
		for (x=0;x<image->width;x++) {
			s16 toolbar_x = x - ((s16)image->width-320)/2;
			if (toolbar_x >= 320 || toolbar_x < 0) {
				image->data[x+(y+160)*image->width] = data->high_perf_palette[0] | 0xFF000000;
			} else {
				image->data[x+(y+160)*image->width] = data->high_perf_palette[data->high_perf_toolbar[toolbar_x+y*320] & 0x0F] | 0xFF000000;
			}
		}
	}

	draw_highperf_text(image, data, 160, text);

	// DRAW SKILLZ
	u8 nums[10];
	nums[0] = level->info.rate;
	nums[1] = state->cur_rate;
	for (i=0;i<8;i++) {
		nums[i+2] = level->info.skills[i];
	}
	for (i=0;i<10;i++) {
		for (y=0;y<8 && y+177<image->height;y++) {
			for (x=16*i+4;x<16*i+12;x++) {
				s16 screen_x = x + ((s16)image->width-320)/2;
				if (screen_x < 0 || screen_x >= image->width) {
					continue;
				} else {
					// draw rectangle
					if (nums[i] == 0) {
						image->data[screen_x+(y+177)*image->width] = data->high_perf_palette[3] | 0xFF000000;
					}else{
						if (nums[i] > 99) {
							nums[i] = 99;
						}
						image->data[screen_x+(y+177)*image->width] = data->high_perf_palette[0] | 0xFF000000;
						if (data->skill_numbers[(nums[i]%10)*2*8*8 + y*8 + (x-16*i-4)]) {
							image->data[screen_x+(y+177)*image->width] = data->high_perf_palette[3] | 0xFF000000;
						}
						if (data->skill_numbers[((nums[i]/10)*2+1)*8*8 + y*8 + (x-16*i-4)]) {
							image->data[screen_x+(y+177)*image->width] = data->high_perf_palette[3] | 0xFF000000;
						}
					}
				}
			}
		}
	}

	// draw active_skill
	if (state->selected_skill >= 8) {
		state->selected_skill = 0;
	}
	for (y=0;y<24 && y+176<image->height;y++) {
		for (x=16*(state->selected_skill+2);x<16*(state->selected_skill+3);x++) {
			s16 screen_x = x + ((s16)image->width-320)/2;
			if (screen_x < 0 || screen_x >= image->width) {
				continue;
			} else {
				// draw rectangle
				if (y==0 || y==23) {
					image->data[screen_x+(y+176)*image->width] = data->high_perf_palette[3] | 0xFF000000;
				}else{
					if (x==16*(state->selected_skill+2) || x==16*(state->selected_skill+2)+15) {
						image->data[screen_x+(y+176)*image->width] = data->high_perf_palette[3] | 0xFF000000;
					}
				}
			}
		}
	}

	// draw mini-map
	for (y=16;y<160;y+=8) {
		for (x=0;x<1584;x+=16) {
			int solid = 0;
			for (i=0;i<16;i++) {
				solid += ((level->terrain[x+i+y*1584] & 0xF0)?1:0);
			}
			solid = (solid>8?7:0); // palette entry to draw in minimap
			s16 minimap_x = 209 + x/16;
			s16 minimap_y = 177 + y/8;
			minimap_x += + ((s16)image->width-320)/2;
			if (minimap_x < 0 || minimap_x >= image->width || minimap_y < 0 || minimap_y >= image->height) {
				continue;
			}
			image->data[minimap_x+minimap_y*image->width] = data->high_perf_palette[solid]  | 0xFF000000;
		}
	}

	// draw lemmings
	draw_lemmings_minimap(lemmings,data,image);


	// draw current view into minimap
	s16 view_rect_width = 103 - (1584-image->width) / 16;
	if (view_rect_width < 3) {
		view_rect_width = 3;
	}else if (view_rect_width > 103) {
		view_rect_width = 103;
	}
	for (y=0;y<20;y++) {
		for (x=level->info.x_pos / 16;x<level->info.x_pos / 16 + view_rect_width; x++) {
			s16 screen_x = x + ((s16)image->width-320)/2 + 209;
			s16 screen_y = y + 178;
			if (screen_x < 0 || screen_x >= image->width || screen_y >= image->height) {
				continue;
			}
			// draw rectangle
			if (y==0 || y==19) {
				image->data[screen_x+screen_y*image->width] = data->high_perf_palette[3] | 0xFF000000;
			}else{
				if (x==level->info.x_pos / 16 || x==level->info.x_pos / 16 + view_rect_width - 1) {
					image->data[screen_x+screen_y*image->width] = data->high_perf_palette[3] | 0xFF000000;
				}
			}

		}
	}

	return 1; // all fine
}

void draw_lemmings_minimap(struct Lemming lemmings[MAX_NUM_OF_LEMMINGS], struct MainInGameData* data, struct RGB_Image* image) {
	int i;
	if (!lemmings || !image) {
		return; // error
	}
	for (i=0;i<80;i++) {
		if (lemmings[i].removed || lemmings[i].current_action >= 18) {
			continue;
		}
		if (lemmings[i].y >= 160 || lemmings[i].x >= 1664) { // or: 1584?
			continue;
		}

		s16 minimap_x = 209 + (lemmings[i].x>=0?lemmings[i].x:0)/16;
		s16 minimap_y = 177 + (lemmings[i].y>=16?lemmings[i].y:16)/8;
		image->data[minimap_x+minimap_y*image->width] = data->high_perf_palette[2] | 0xFF000000; // green
	}
}

u8 draw_lemmings(struct Lemming lemmings[MAX_NUM_OF_LEMMINGS], struct Image* lemmings_anim[337], struct Image* masks[23], u32 palette[16], struct RGB_Image* image, u16 x_offset) {
	int i;
	u8 ret = 0;
	if (!lemmings) {
		return ret; // error
	}
	for (i=0;i<80;i++) {
		if (lemmings[i].removed || lemmings[i].current_action >= 18) {
			continue;
		}
		ret++;
		draw_single_lemming(lemmings+i,lemmings_anim,masks,palette,image,x_offset);
	}
	return ret;
}

// particle: 0-79
// frame: 0-50
static inline int get_particle(u8 particle, u8 frame, s8* x, s8* y, u8* color) {
	if (!x || !y || !color || particle >= 80 || frame > 50) {
		return 0;
	}
	*x = particles[particle*2 + frame*160];
	*y = particles[1 + particle*2 + frame*160];
	const u8 colors[] = {
		4, 15, 14, 13, 12, 11, 10, 9, 8, 11, 10, 9, 8, 7, 6, 2
	};
	if (*x == -128 && *y == -128) {
		*color = 0x00; // gone
		return 0;
	}else{
		*color = colors[particle % 16] | 0xF0;
	}
	return 1;
}

void draw_single_lemming(struct Lemming* lem, struct Image* lemmings_anim[337], struct Image* masks[23], u32 palette[16], struct RGB_Image* image, u16 x_offset) {
	if (!lem || !lemmings_anim || !palette || !image) {
		return;
	}
	u8 act = 0;
	if (ENABLE_SHRUGGER_GLITCH) {
		act = lem->draw_action;
	}else{
		act = lem->current_action;
	}
	if (act >= 18 || lem->removed) {
		return;
	}
	s16 x_pos = lem->x + (s16)lem->x_draw_offset - (s16)x_offset;
	s16 y_pos = lem->y + (s16)lem->y_draw_offset;
	int im_idx = action_image_offsets[act][lem->look_right?1:0] + lem->frame_offset;
	if (im_idx < 0) {
		return;
	}
	if (im_idx >= 337) {
		// draw explosion particles
		u8 i;
		if (im_idx - 337 > 50) {
			return;
		}
		u8 frame = (u8)(im_idx-337);
		for (i=0;i<80;i++) {
			s8 x,y;
			u8 c;
			if (get_particle(i,frame,&x,&y,&c)) {
				x_pos = lem->x - (s16)x_offset + (s16)x;
				y_pos = lem->y + (s16)y;
				if (x_pos < 0 || x_pos >= image->width) {
					continue;
				}
				if (y_pos < 0 || y_pos >= image->height) {
					continue;
				}
				image->data[x_pos+y_pos*image->width] =
					palette[c & 0x0F] | 0xFF000000;
			}
		}
		return;
	}
	struct Image* im = lemmings_anim[im_idx];
	// copy im to image
	s16 x,y;
	for (y=0;y<im->height;y++) {
		if (y+y_pos < 0 || y+y_pos >= image->height) {
			continue;
		}
		for (x=0;x<im->width;x++) {
			if (x+x_pos < 0 || x+x_pos >= image->width) {
				continue;
			}
			if (im->data[x+y*im->width] & 0xF0) {
				// draw pixel
				image->data[x+x_pos+(y+y_pos)*image->width] =
					palette[im->data[x+y*im->width] & 0x0F] | 0xFF000000;
			}
		}
	}
	if (lem->timer && masks) {
		// draw timer!
		u8 number = lem->timer >> 4;
		number = (number<9?number+1:9);
		u8 idx = 22-number;
		if (masks[idx]) {
			s16 y_pos = lem->y + (s16)lem->y_draw_offset - masks[idx]->height;
			s16 x_pos = lem->x - (s16)x_offset;
			for (y=0;y<masks[idx]->height;y++) {
				if (y+y_pos < 0 || y+y_pos >= image->height) {
					continue;
				}
				for (x=0;x<masks[idx]->width;x++) {
					if (x+x_pos < 0 || x+x_pos >= image->width) {
						continue;
					}
					if (masks[idx]->data[x+y*masks[idx]->width] & 0xF0) {
						// draw pixel
						image->data[x+x_pos+(y+y_pos)*image->width] =
								palette[3] | 0xFF000000;
					}
				}
			}
		}
	}
}

void tile_menu_background(struct RGB_Image* im_bottom, struct MainMenuData* menu_data) {
	memset(im_bottom->data,0,sizeof(u32)*im_bottom->width*im_bottom->height);
	s16 i,j;
	for (i=0;i<im_bottom->width;i+=320) {
			for (j=0;j<im_bottom->height;j+=104) {
				draw(im_bottom,
						menu_data->palette,
						menu_data->static_pictures[0]->data,
						i,j,
						menu_data->static_pictures[0]->width,
						menu_data->static_pictures[0]->height);
			}
	}
}

int draw_topscreen(struct MainMenuData* menu, struct RGB_Image* im_top, sf2d_texture** texture_top_screen, struct RGB_Image* logo_scaled, sf2d_texture** texture_logo) {
	if (*texture_logo) {
		sf2d_free_texture(*texture_logo);
		*texture_logo = 0;
	}
	if (*texture_top_screen) {
		sf2d_free_texture(*texture_top_screen);
		*texture_top_screen = 0;
	}
	*texture_top_screen = sf2d_create_texture(im_top->width, im_top->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	if (!(*texture_top_screen)) {
		return 0;
	}
	tile_menu_background(im_top, menu);

	sf2d_fill_texture_from_RGBA8(*texture_top_screen, im_top->data, im_top->width, im_top->height);
	sf2d_texture_tile32(*texture_top_screen);


	struct RGB_Image* logo = (struct RGB_Image*)malloc(sizeof(struct RGB_Image)+sizeof(u32)*632*94);
	if (!logo) {
		sf2d_free_texture(*texture_top_screen);
		*texture_top_screen = 0;
		return 0;
	}
	logo->width = 632;
	logo->height = 94;
	memset(logo->data,0,sizeof(u32)*logo->width*logo->height);
	draw(logo,menu->palette,menu->static_pictures[1]->data,0,0,menu->static_pictures[1]->width,menu->static_pictures[1]->height);
	scale(logo_scaled, 0.6, logo);
	free(logo);
	logo = 0;

	*texture_logo = sf2d_create_texture(logo_scaled->width, logo_scaled->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	if (!*texture_logo) {
		sf2d_free_texture(*texture_top_screen);
		*texture_top_screen = 0;
		return 0;
	}

	sf2d_fill_texture_from_RGBA8(*texture_logo, logo_scaled->data, logo_scaled->width, logo_scaled->height);
	sf2d_texture_tile32(*texture_logo);
	return 1;
}
