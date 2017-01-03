#include <string.h>
#include <stdlib.h>
#include <sf2d.h>
#include "draw.h"
#include "highperf_font.h"
#include "lemming.h"
#include "particles.h"
#include "settings.h"

#ifdef NO_SF2D
#define SET_PIXEL(buf,x,y,val) { \
		(buf)->data[3*((x+1)*(buf)->height-(y))] = (u8)(((val)>>8)&0xFF); \
		(buf)->data[3*((x+1)*(buf)->height-(y))+1] = (u8)(((val)>>16)&0xFF); \
		(buf)->data[3*((x+1)*(buf)->height-(y))+2] = (u8)(((val)>>24)&0xFF); \
		}
#define GET_PIXEL(buf,x,y) (((u32)((buf)->data[3*((x+1)*(buf)->height-(y))])<<8) \
		| ((u32)((buf)->data[3*((x+1)*(buf)->height-(y))+1])<<16) \
		| ((u32)((buf)->data[3*((x+1)*(buf)->height-(y))+2])<<24) \
		| 0xFF)
#else
#define SET_PIXEL(buf,x,y, val) {(((u32*)(buf)->data)[(x)+(y)*(buf)->width]) = (val); }
#define GET_PIXEL(buf,x,y) (((u32*)(buf)->data)[(x)+(y)*(buf)->width])
#endif

struct Buffer {
	u16 width;
	u16 height;
	u8* data;
};

struct Buffer top_screen = {0,0,0};
struct Buffer bottom_screen = {0,0,0};
struct Buffer top_screen_backbuffer = {0,0,0};
struct Buffer bottom_screen_backbuffer = {0,0,0};

int initialized = 0;

static inline struct Buffer* getScreenBuffer(ScreenBuffer buffer) {
	switch (buffer) {
		case TOP_SCREEN:
			return &top_screen;
		case BOTTOM_SCREEN:
			return &bottom_screen;
		case TOP_SCREEN_BACK:
			return &top_screen_backbuffer;
		case BOTTOM_SCREEN_BACK:
			return &bottom_screen_backbuffer;
		default:
			return 0;
	}
}

void init_drawing() {
	#ifdef NO_SF2D
	gfxSetScreenFormat(GFX_TOP,GSP_BGR8_OES);
	gfxSetDoubleBuffering(GFX_TOP,1);
	gfxSetScreenFormat(GFX_BOTTOM,GSP_BGR8_OES);
	gfxSetDoubleBuffering(GFX_BOTTOM,1);
	top_screen_backbuffer.data = (u8*)malloc(3*400*240);
	bottom_screen_backbuffer.data = (u8*)malloc(3*320*240);
	#else
	gfxExit();
	sf2d_init();
	sf2d_start_frame(GFX_TOP, GFX_LEFT);
	sf2d_end_frame();
	sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
	sf2d_end_frame();
	sf2d_swapbuffers();
	top_screen_backbuffer.data = (u8*)malloc(4*400*240);
	bottom_screen_backbuffer.data = (u8*)malloc(4*320*240);
	top_screen.data = (u8*)malloc(4*400*240);
	bottom_screen.data = (u8*)malloc(4*320*240);
	#endif
	top_screen_backbuffer.width = 400;
	top_screen_backbuffer.height = 240;
	bottom_screen_backbuffer.width = 320;
	bottom_screen_backbuffer.height = 240;
	top_screen.width = 400;
	top_screen.height = 240;
	bottom_screen.width = 320;
	bottom_screen.height = 240;
	initialized = 1;
}

void fade_palette(u32 palette[16], float fading) {
	if (fading >= 1.0f) {
		return;
	}
	if (fading < 0.0f) {
		fading = 0.0f;
	}
	u8 i;
	for (i=0;i<16;i++) {
		palette[i] =
				((u32)((float)((palette[i]>>8) & 0xFF)*fading)<<8)
				| ((u32)((float)((palette[i]>>16) & 0xFF)*fading)<<16)
				| ((u32)((float)((palette[i]>>24) & 0xFF)*fading)<<24)
				| (palette[i] & 0xFF);
	}
}

void begin_frame() {
	if (!initialized) {
		return;
	}
	#ifdef NO_SF2D
	top_screen.data = gfxGetFramebuffer(GFX_TOP,GFX_LEFT,0,0);
	bottom_screen.data = gfxGetFramebuffer(GFX_BOTTOM,GFX_LEFT,0,0);
	#endif
}

void end_frame() {
	if (!initialized) {
		return;
	}
	#ifdef NO_SF2D
	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();
	#else
	sf2d_texture* top_screen_t = sf2d_create_texture(400, 240, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	sf2d_fill_texture_from_RGBA8(top_screen_t, top_screen.data, 400, 240);
	sf2d_texture_tile32(top_screen_t);
	sf2d_texture* bot_screen_t = sf2d_create_texture(320, 240, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	sf2d_fill_texture_from_RGBA8(bot_screen_t, bottom_screen.data, 320, 240);
	sf2d_texture_tile32(bot_screen_t);

	sf2d_start_frame(GFX_TOP, GFX_LEFT);
	sf2d_draw_texture(top_screen_t,0,0);
	sf2d_end_frame();
	sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
	sf2d_draw_texture(bot_screen_t,0,0);
	sf2d_end_frame();
	sf2d_swapbuffers();
	sf2d_free_texture(top_screen_t);
	sf2d_free_texture(bot_screen_t);
	#endif
}

void copy_from_backbuffer(ScreenBuffer screen) {
	if (!initialized) {
		return;
	}
	switch (screen) {
		case TOP_SCREEN:
		case TOP_SCREEN_BACK:
			memcpy(
					top_screen.data,
					top_screen_backbuffer.data,
					#ifdef NO_SF2D
					3*top_screen.width*top_screen.height
					#else
					4*top_screen.width*top_screen.height
					#endif
					);
			break;
		case BOTTOM_SCREEN:
		case BOTTOM_SCREEN_BACK:
			memcpy(
					bottom_screen.data,
					bottom_screen_backbuffer.data,
					#ifdef NO_SF2D
					3*bottom_screen.width*bottom_screen.height
					#else
					4*bottom_screen.width*bottom_screen.height
					#endif
					);
			break;
		default:
			break;
	}
}

void draw_lemmings_minimap(
		struct Level* level,
		u32 highperf_palette[16]);

void draw_single_lemming(
		ScreenBuffer screen,
		s16 x,
		s16 y,
		struct Lemming* lem,
		struct Image* lemmings_anim[337],
		struct Image* masks[23],
		u32 palette[16],
		s16 x_offset,
		s16 y_offset,
		u8 player2);

int clear(ScreenBuffer screen) {
	if (!initialized) {
		return 0;
	}
	struct Buffer* dest = getScreenBuffer(screen);
	if (!dest->data) {
		return 0;
	}
	memset(dest->data, 0,
			#ifdef NO_SF2D
			3*dest->width*dest->height
			#else
			4*dest->width*dest->height
			#endif
			);
	return 1;
}

int clear_rectangle(
		ScreenBuffer screen,
		u16 x,
		u16 y,
		u16 w,
		u16 h) {
	if (!initialized) {
		return 0;
	}
	struct Buffer* dest = getScreenBuffer(screen);
	if (!dest->data) {
		return 0;
	}
	#ifdef NO_SF2D
	u16 xi;
	for (xi = 0; xi < w; xi++) {
		memset(dest->data + 3*((xi+x+1)*dest->height - (y+h-1)), 0, 3*h);
	}
	#else
	u16 yi;
	for (yi = 0; yi < h; yi++) {
		memset(dest->data + 4*(x + (yi+y)*dest->width), 0, 4*w);
	}
	#endif
	return 1;
}

int draw(
		ScreenBuffer screen,
		s16 x,
		s16 y,
		const u8* img,
		u16 w,
		u16 h,
		u32 palette[16]) {
	if (!initialized) {
		return 0; // error
	}
	struct Buffer* dest = getScreenBuffer(screen);
	if (!dest->data || !palette || !img) {
		return 0; // error
	}
	s16 xi, yi;
	for (yi = 0; yi<h; yi++) {
		s16 draw_y = yi + y;
		if (draw_y < 0 || draw_y >= dest->height) {
			continue;
		}
		for (xi = 0; xi<w; xi++) {
			s16 draw_x = xi + x;
			if (draw_x < 0 || draw_x >= dest->width) {
				continue;
			}
			if (img[xi + yi*(s16)w] & 0xF0) {
				SET_PIXEL(dest, draw_x, draw_y, palette[img[xi + yi*(s16)w] & 0x0F]);
			}
		}
	}
	return 1;
}


// help function for interpolation
static inline u32 get_value(
		const u8* image,
		u16 w,
		u16 h,
		float x1,
		float y1,
		float x2,
		float y2,
		u32 palette[16]) {
	if (!image || !palette) {
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
	if (start_x >= w || start_y >= h) {
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
			if (i >= 0 && i < w && j >= 0 && j < h) {
				u32 px = palette[image[i+j*w] & 0x0F];
				if (!(image[i+j*w] & 0xF0)) {
					px = 0x00000000;
				}
				#ifdef ABGR
				cur_a = ((float)((px >> 0) & 0xFF)) / 255.0f;
				cur_r = ((float)((px >> 24) & 0xFF));
				cur_g = ((float)((px >> 16) & 0xFF));
				cur_b = ((float)((px >> 8) & 0xFF));
				#else
				cur_a = ((float)((px >> 24) & 0xFF)) / 255.0f;
				cur_r = ((float)((px >> 0) & 0xFF));
				cur_g = ((float)((px >> 8) & 0xFF));
				cur_b = ((float)((px >> 16) & 0xFF));
				#endif
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
	#ifdef ABGR
	return (alpha_i<<0) | (r_i<<24) | (g_i<<16) | (b_i << 8);
	#else
	return (alpha_i<<24) | (r_i<<0) | (g_i<<8) | (b_i << 16);
	#endif
}

// draw after scale with linear interpolation
int draw_scaled(
		ScreenBuffer screen,
		s16 x,
		s16 y,
		const u8* img,
		u16 w,
		u16 h,
		u32 palette[16],
		float scaling) {
	if (!initialized) {
		return 0;
	}
	struct Buffer* dest = getScreenBuffer(screen);
	if (!dest->data || !palette || !img) {
		return 0; // error
	}
	if (scaling >= 0.999f && scaling <= 1.001f) {
		return draw(
				screen,
				x,
				y,
				img,
				w,
				h,
				palette);
	}
	s16 xi, yi;
	u16 dest_w, dest_h;
	dest_w = (u16)(((float)w) * scaling + 1.0f);
	dest_h = (u16)(((float)h) * scaling + 1.0f);
	for (yi=0;yi<dest_h;yi++) {
		s16 draw_y = yi + y;
		if (draw_y < 0 || draw_y >= dest->height) {
			continue;
		}
		for (xi=0;xi<dest_w;xi++) {
			s16 draw_x = xi + x;
			if (draw_x < 0 || draw_x >= dest->width) {
				continue;
			}
			u32 color = get_value(
					img,
					w,
					h,
					(float)xi/scaling,
					(float)yi/scaling,
					(float)(xi+1)/scaling,
					(float)(yi+1)/scaling,
					palette);
			u32 old = GET_PIXEL(dest, x+xi, y+yi);
			#ifdef ABGR
			float alpha = (float)((color >> 0)&0xFF) / 255.0f;
			float b = (float)((color >> 8)&0xFF) * alpha / 255.0f
					+ (float)((old >> 8)&0xFF) * (1.0f-alpha) / 255.0f;
			if (b>1.0f) {
				b = 1.0f;
			}
			float g = (float)((color >> 16)&0xFF) * alpha / 255.0f
					+ (float)((old >> 16)&0xFF) * (1.0f-alpha) / 255.0f;
			if (g>1.0f) {
				g = 1.0f;
			}
			float r = (float)((color >> 24)&0xFF) * alpha / 255.0f
					+ (float)((old >> 24)&0xFF) * (1.0f-alpha) / 255.0f;
			if (r>1.0f) {
				r = 1.0f;
			}
			u32 new = (((u32)(r*255.0f)) << 24)
					| (((u32)(g*255.0f)) << 16)
					| (((u32)(b*255.0f)) << 8)
					| 0x000000FF;
			SET_PIXEL(dest, x+xi, y+yi, new);
			#else
			float alpha = (float)((color >> 24)&0xFF) / 255.0f;
			float b = (float)((color >> 16)&0xFF) * alpha / 255.0f
					+ (float)((old >> 16)&0xFF) * (1.0f-alpha) / 255.0f;
			if (b>1.0f) {
				b = 1.0f;
			}
			float g = (float)((color >> 8)&0xFF) * alpha / 255.0f
					+ (float)((old >> 8)&0xFF) * (1.0f-alpha) / 255.0f;
			if (g>1.0f) {
				g = 1.0f;
			}
			float r = (float)((color >> 0)&0xFF) * alpha / 255.0f
					+ (float)((old >> 0)&0xFF) * (1.0f-alpha) / 255.0f;
			if (r>1.0f) {
				r = 1.0f;
			}
			SET_PIXEL(dest, x+xi, y+yi,
					(((u32)(r*255.0f)) << 0)
					| (((u32)(g*255.0f)) << 8)
					| (((u32)(b*255.0f)) << 16)
					| 0xFF000000);
			#endif
		}
	}
	return 1;
}

int draw_level(
		ScreenBuffer screen,
		s16 x,
		s16 y,
		u16 w,
		u16 h,
		struct Level* level,
		struct MainInGameData* main_data,
		u32* level_palette) {
	if (!initialized) {
		return 0;
	}
	struct Buffer* dest = getScreenBuffer(screen);
	if (!dest->data || !level) {
		return 0; // error
	}
	if (!level_palette) {
		level_palette = level->palette;
	}
	// adjust everything
	if (x>=dest->width || y>dest->height) {
		return 1;
	}
	if (x+w<0 || y+h<0) {
		return 1;
	}
	if (x+w>dest->width) {
		w = dest->width-x;
	}
	if (y+h>dest->height) {
		h = dest->height-y;
	}
	s16 x_offset = level->player[0].x_pos;
	s16 y_offset = 0;
	if (x_offset + w > 1584) {
		if (w >= 1584) {
			x_offset = 0;
		}else{
			x_offset = 1584 - w;
		}
	}
	if (x<0) {
		x_offset -= x;
		w += x;
		x = 0;
	}
	if (y<0) {
		y_offset -= y;
		h += y;
		y = 0;
	}

	// draw terrain
	s16 xi,yi;
	for (yi=0;yi<h;yi++) {
		if (y+yi < 0 || y+yi >= dest->height) {
			continue;
		}
		x=0;w=320;
		for (xi=0;xi<w;xi++) {
			if (x+xi < 0 || x+xi >= dest->width) {
				continue;
			}
			s16 level_x = x_offset + xi;
			if (level_x >= 1584 || level_x < 0 || yi > 160) {
				SET_PIXEL(dest, x+xi, y+yi, level_palette[0]);
				continue;
			}
			if (level->terrain[level_x+yi*1584] & 0xF0) {
				SET_PIXEL(dest, x+xi, y+yi,
						level_palette[
								level->terrain[level_x+yi*1584] & 0x0F]);
			}else{
				SET_PIXEL(dest, x+xi, y+yi, level_palette[0]);
			}
		}
	}

	// draw objects
	u8 i;
	for (i=0;i<32;i++) {
		// process object!
		if (!(level->object_instances[i].modifier & OBJECT_USED)) {
			continue;
		}
		struct ObjectType* obj_type = level->object_types[level->object_instances[i].type];
		if (!obj_type) {
			continue;
		}

		// copy image
		for (yi=0;yi<obj_type->height;yi++) {
			s16 draw_y, level_y;
			if (OBJECT_UPSIDE_DOWN & level->object_instances[i].modifier) {
				level_y = (s16)level->object_instances[i].y-yi+(s16)obj_type->height-1;
			}else{
				level_y = yi+(s16)level->object_instances[i].y;
			}
			draw_y = level_y + y - y_offset;
			if (draw_y < 0 || draw_y >= y+h) {
				continue;
			}
			for (xi=0;xi<obj_type->width;xi++) {
				if (xi+level->object_instances[i].x < 0 || xi+level->object_instances[i].x >= 1584) {
					continue;
				}
				if (level->object_instances[i].current_frame > obj_type->end_frame) {
					continue;
				}
				s16 draw_x = xi+ (s16)level->object_instances[i].x - x_offset + x;
				if (draw_x < 0 || draw_x >= x+w) {
					continue;
				}

				u8 color = obj_type->data[
						level->object_instances[i].current_frame
							* (u32)obj_type->width
							* (u32)obj_type->height
						+ yi * (u32)obj_type->width
						+ xi];
				if ((color & 0xF0) == 0) {
					continue;
				}
				// TODO: how to handle OBJECT_DONT_OVERWRITE?
				// a) don't overwrite terrain or b) don't overwrite anything?

				if ((OBJECT_DONT_OVERWRITE & level->object_instances[i].modifier)
						&& !(OBJECT_REQUIRE_TERRAIN & level->object_instances[i].modifier)
						&& (level->terrain[xi+level->object_instances[i].x+1584*(level_y)]
							& 0xF0)) {
					continue;
				}
				if ((OBJECT_REQUIRE_TERRAIN & level->object_instances[i].modifier)
						&& !(level->terrain[xi+level->object_instances[i].x+1584*(level_y)]
							& 0xF0)) {
					continue;
				}
				if (OBJECT_REQUIRE_TERRAIN & level->object_instances[i].modifier) {
					//1->5; 3->5; 4->4; ...
					color = 0xF4 | (color&0x1);
				}
				SET_PIXEL(dest, draw_x, draw_y, level_palette[color & 0x0F]);
			}
		}
	}

	// draw lemmings
	draw_lemmings(
			screen,
			x,
			y,
			level,
			main_data->lemmings_anim,
			main_data->masks,
			level_palette,
			level->player[0].x_pos,
			0);
	return 1;
}

void draw_highperf_text(
		ScreenBuffer screen,
		s16 x,
		s16 y,
		struct MainInGameData* data,
		const char* text,
		u32* highperf_palette) {
	if (!initialized) {
		return;
	}
	struct Buffer* dest = getScreenBuffer(screen);
	if (!text || !dest->data || !data) {
		return;
	}
	if (!highperf_palette) {
		highperf_palette = data->high_perf_palette;
	}
	int i=0;
	s16 x_offset = x;
	s16 y_offset = y;
	s16 xi,yi;
	while(1) {
		int id = -1; // other symbol
		u32 code = 0;
		s16 bytes = decode_utf8(&code, (const u8*)&text[i]);
		if (bytes < 1 || !code) {
			break;
		}
		const u8* other = 0; // space
		if (code == '\n') {
			i+=bytes;
			x_offset = x;
			y_offset += 16;
			continue;
		}
		if (code != ' ') {
			// initilize with unsupported symbol
			other = highperf_font_questionmark;
		}
		if (code == '%') {
			id = 0;
		}
		if (code>='0' && code<='9') {
			id = code-'0'+1;
		}
		if (code == '-') {
			id = 11;
		}
		if (code>='A' && code<='Z') {
			id = code-'A'+12;
		}
		if (code>='a' && code<='z') {
			id = code-'a'+12;
		}
		if (code == '.') {
			other = highperf_font_dot;
		}
		if (code == ',') {
			other = highperf_font_comma;
		}
		if (code == '?') {
			other = highperf_font_questionmark;
		}
		if (code == '!') {
			other = highperf_font_exclamationmark;
		}
		if (code == ':') {
			other = highperf_font_colon;
		}
		if (code == '(') {
			other = highperf_font_bracket_l;
		}
		if (code == ')') {
			other = highperf_font_bracket_r;
		}
		if (code == '\'' || text[i] == '`') {
			other = highperf_font_apostrophe;
		}
		if (code == '"') {
			other = highperf_font_quote;
		}
		if (x_offset >= 40) {
			i+=bytes;
			continue;
		}
		// now draw character with index i
		for (yi=0;yi<16 && yi+y_offset<dest->height;yi++) {
			for (xi=8*x_offset;xi<8*(x_offset+1);xi++) {
				if (xi >= dest->width) {
					break;
				}
				s16 screen_x = xi + ((s16)dest->width-320)/2;
				if (screen_x < 0 || screen_x >= dest->width) {
					continue;
				} else {
					// draw character
					if (id == -1) {
						if (other) {
							if (other[8*yi + xi-8*x_offset] & 0xF0) {
								SET_PIXEL(dest, screen_x, yi+y_offset,
										highperf_palette[
												other[8*yi + xi-8*x_offset] & 0xF]);
							}
						}
					}else{
						u8 color = data->high_perf_font[id*8*16 + 8*yi + xi-8*x_offset];
						if (color & 0xF0) {
							SET_PIXEL(dest, screen_x, yi+y_offset,
									highperf_palette[color & 0xF]);
						}
					}
				}
			}
		}
		i+=bytes;
		x_offset++;
	}
}


void draw_menu_text(
		ScreenBuffer screen,
		struct MainMenuData* data,
		s16 x_offset,
		s16 y_offset,
		const char* text,
		u32* palette,
		float scaling) {
	if (!initialized) {
		return;
	}
	if (!palette) {
		palette = data->palette;
	}
	s16 x_pos = x_offset;
	int i=0;
	if (scaling < 0.0625f) {
		scaling = 0.0625f;
	}else if (scaling > 16.0f){
		scaling = 16.0f;
	}
	while(1) {
		int id = -1; // space symbol
		u32 code = 0;
		s16 bytes = decode_utf8(&code, (const u8*)&text[i]);
		if (bytes < 1 || !code) {
			break;
		}
		if (code == '\n') {
			i+=bytes;
			x_pos = x_offset;
			y_offset += ((15.999f * scaling)+1.0f);
			continue;
		}
		if (code >= 33 && code < 127) {
			id = code-33;
		}else if (code != ' ') {
			id = '?'-33; // unsupported symbol
		}
		// now draw character with index i
		if (id >= 0) {
			draw_scaled(
					screen,
					x_pos,
					y_offset,
					data->menu_font + id*16*16,
					16,
					16,
					palette,
					scaling);
		}
		i+=bytes;
		x_pos += ((15.999f * scaling)+1.0f);
	}
}


int draw_toolbar(
		struct MainInGameData* data,
		struct Level* level,
		const char* text,
		u32* highperf_palette) {
	if (!initialized) {
		return 0;
	}
	struct Buffer* dest = getScreenBuffer(BOTTOM_SCREEN);
	s16 y = 160+32;
	if (!dest->data || !data || !level) {
		return 0;
	}
	if (!highperf_palette) {
		highperf_palette = data->high_perf_palette;
	}

	s16 x,yi,i;
	for (yi=0;yi<40 && yi+y<dest->height;yi++) {
		for (x=0;x<dest->width;x++) {
			s16 toolbar_x = x - ((s16)dest->width-320)/2;
			if (toolbar_x >= 320 || toolbar_x < 0) {
				SET_PIXEL(dest, x, y+yi, highperf_palette[0]);
			} else {
				SET_PIXEL(dest, x, y+yi,
						highperf_palette[
								data->high_perf_toolbar[
										toolbar_x+yi*320] & 0x0F]);
			}
		}
	}

	draw_highperf_text(
			GFX_BOTTOM,
			((s16)dest->width-320)/2,
			160+32,
			data,
			text,
			highperf_palette);

	// number of available draw skills
	u8 nums[10];
	nums[0] = level->rate;
	nums[1] = level->cur_rate;
	for (i=0;i<8;i++) {
		nums[i+2] = level->player[0].skills[i];
	}
	for (i=0;i<10;i++) {
		for (yi=0;yi<8 && yi+y+17<dest->height;yi++) {
			for (x=16*i+4;x<16*i+12;x++) {
				s16 screen_x = x + ((s16)dest->width-320)/2;
				if (screen_x < 0 || screen_x >= dest->width) {
					continue;
				} else {
					// draw rectangle
					if (nums[i] == 0) {
						SET_PIXEL(dest, screen_x, y+yi+17, highperf_palette[3]);
					}else{
						if (nums[i] > 99) {
							nums[i] = 99;
						}
						SET_PIXEL(dest, screen_x, y+yi+17, highperf_palette[0]);
						if (data->skill_numbers[
								(nums[i]%10)*2*8*8 + yi*8 + (x-16*i-4)]) {
							SET_PIXEL(dest, screen_x, y+yi+17, highperf_palette[3]);
						}
						if (data->skill_numbers[
								((nums[i]/10)*2+1)*8*8 + yi*8 + (x-16*i-4)]) {
							SET_PIXEL(dest, screen_x, y+yi+17, highperf_palette[3]);
						}
					}
				}
			}
		}
	}

	// mark active_skill
	if (level->player[0].selected_skill >= 8) {
		level->player[0].selected_skill = 0;
	}
	for (yi=0;yi<24 && y+yi+16<dest->height;yi++) {
		for (x=16*(level->player[0].selected_skill+2);x<16*(level->player[0].selected_skill+3);x++) {
			s16 screen_x = x + ((s16)dest->width-320)/2;
			if (screen_x < 0 || screen_x >= dest->width) {
				continue;
			} else {
				// draw rectangle
				if (yi==0 || yi==23) {
					SET_PIXEL(dest, screen_x, y+yi+16, highperf_palette[3]);
				}else{
					if (x==16*(level->player[0].selected_skill+2)
							|| x==16*(level->player[0].selected_skill+2)+15) {
						SET_PIXEL(dest, screen_x, y+yi+16, highperf_palette[3]);
					}
				}
			}
		}
	}

	// draw mini-map
	for (yi=16;yi<160;yi+=8) {
		for (x=0;x<1584;x+=16) {
			int solid = 0;
			for (i=0;i<16;i++) {
				solid += ((level->terrain[x+i+yi*1584] & 0xF0)?1:0);
			}
			solid = (solid>8?7:0); // palette entry to draw in minimap
			s16 minimap_x = 209 + x/16;
			s16 minimap_y = y+17 + yi/8;
			minimap_x += + ((s16)dest->width-320)/2;
			if (minimap_x < 0 || minimap_x >= dest->width
					|| minimap_y < y || minimap_y >= dest->height) {
				continue;
			}
			SET_PIXEL(dest, minimap_x, minimap_y, highperf_palette[solid]);
		}
	}

	// draw lemmings into minimap
	draw_lemmings_minimap(level,highperf_palette);


	// draw current view into minimap
	s16 view_rect_width = 103 - (1584-dest->width) / 16;
	if (view_rect_width < 3) {
		view_rect_width = 3;
	}else if (view_rect_width > 103) {
		view_rect_width = 103;
	}
	for (yi=0;yi<20;yi++) {
		for (x=level->player[0].x_pos / 16;x<level->player[0].x_pos / 16 + view_rect_width; x++) {
			s16 screen_x = x + ((s16)dest->width-320)/2 + 209;
			s16 screen_y = y + 18 + yi;
			if (screen_x < 0 || screen_x >= dest->width || screen_y >= dest->height) {
				continue;
			}
			// draw rectangle
			if (yi==0 || yi==19) {
				SET_PIXEL(dest, screen_x, screen_y, highperf_palette[3]);
			}else{
				if (x==level->player[0].x_pos / 16
						|| x==level->player[0].x_pos / 16 + view_rect_width - 1) {
					SET_PIXEL(dest, screen_x, screen_y, highperf_palette[3]);
				}
			}

		}
	}
	return 1; // all fine
}

void draw_lemmings_minimap(struct Level* level, u32 highperf_palette[16]) {
	if (!initialized) {
		return;
	}
	s16 y = 160+32;
	struct Buffer* dest = getScreenBuffer(BOTTOM_SCREEN);
	int i, p;
	if (!level) {
		return; // error
	}
	for (p=0;p<level->num_players;p++) {
		u8 color = 2;
		if (level->num_players > 1 && p == 0) {
			// color: blue (instead of green)
			color = 1;
		}
		for (i=0;i<80;i++) {
			if (level->player[p].lemmings[i].removed
					|| level->player[p].lemmings[i].current_action >= 18) {
				continue;
			}
			if (level->player[p].lemmings[i].y >= 160
					|| level->player[p].lemmings[i].x >= 1664) { // or: 1584?
				continue;
			}

			s16 minimap_x = 209
					+ (level->player[p].lemmings[i].x>=0?
							level->player[p].lemmings[i].x:0)/16;
			s16 minimap_y = y + 17
					+ (level->player[p].lemmings[i].y>=16?
							level->player[p].lemmings[i].y:16)/8;
			SET_PIXEL(dest, minimap_x, minimap_y, highperf_palette[color]); // green (or blue)
		}
	}
}

void draw_lemmings(
		ScreenBuffer screen,
		s16 x,
		s16 y,
		struct Level* level,
		struct Image* lemmings_anim[337],
		struct Image* masks[23],
		u32 palette[16],
		s16 x_offset,
		s16 y_offset) {
	if (!initialized) {
		return;
	}
	int i, p;
	if (!level) {
		return; // error
	}
	for (p=0;p<level->num_players;p++) {
		for (i=0;i<MAX_NUM_OF_LEMMINGS;i++) {
			if (level->player[p].lemmings[i].removed
					|| level->player[p].lemmings[i].current_action >= 18) {
			continue;
			}
			draw_single_lemming(
					screen,
					x,
					y,
					&level->player[p].lemmings[i],
					lemmings_anim,
					masks,palette,
					x_offset,
					y_offset,
					p);
		}
	}
	return;
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

void draw_single_lemming(
		ScreenBuffer screen,
		s16 x,
		s16 y,
		struct Lemming* lem,
		struct Image* lemmings_anim[337],
		struct Image* masks[23],
		u32 palette[16],
		s16 x_offset,
		s16 y_offset,
		u8 player2) {
	// swaps color 1 with color 2 if player2 is set
	u8 i;
	u8 swap_pal[16];
	for (i=0;i<16;i++) {
		swap_pal[i] = i;
	}
	if (player2) {
		swap_pal[1] = 2;
		swap_pal[2] = 1;
	}
	struct Buffer* dest = getScreenBuffer(screen);
	if (!lem || !lemmings_anim || !palette || !dest->data) {
		return;
	}
	if (y<0) {
		y_offset -= y;
		y = 0;
	}
	if (x<0) {
		x_offset -= x;
		x = 0;
	}
	u8 act = 0;
	if (settings.glitch_shrugger) {
		act = lem->draw_action;
	}else{
		act = lem->current_action;
	}
	if (act >= 18 || lem->removed) {
		return;
	}
	s16 x_pos = lem->x + (s16)lem->x_draw_offset - x_offset + x;
	s16 y_pos = lem->y + (s16)lem->y_draw_offset - y_offset + y;
	int im_idx = action_image_offsets[act][lem->look_right?1:0] + lem->frame_offset;
	if (im_idx < 0) {
		return;
	}
	if (im_idx >= 337) {
		// draw explosion particles
		if (im_idx - 337 > 50) {
			return;
		}
		u8 frame = (u8)(im_idx-337);
		for (i=0;i<80;i++) {
			s8 xi,yi;
			u8 c;
			if (get_particle(i,frame,&xi,&yi,&c)) {
				x_pos = lem->x + (s16)xi - x_offset + x;
				y_pos = lem->y + (s16)yi - y_offset + y;
				if (x_pos < x || x_pos >= dest->width) {
					continue;
				}
				if (y_pos < y || y_pos >= dest->height) {
					continue;
				}
				SET_PIXEL(dest, x_pos, y_pos, palette[swap_pal[c & 0x0F]]);
			}
		}
		return;
	}
	struct Image* im = lemmings_anim[im_idx];
	// copy im to image
	s16 xi,yi;
	for (yi=0;yi<im->height;yi++) {
		if (yi+y_pos < y || yi+y_pos >= dest->height) {
			continue;
		}
		for (xi=0;xi<im->width;xi++) {
			if (xi+x_pos < 0 || xi+x_pos >= dest->width) {
				continue;
			}
			if (im->data[xi+yi*im->width] & 0xF0) {
				// draw pixel
				SET_PIXEL(dest, xi+x_pos, yi+y_pos,
					palette[swap_pal[im->data[xi+yi*im->width] & 0x0F]]);
			}
		}
	}
	if (lem->timer && masks) {
		// draw timer!
		u8 number = lem->timer >> 4;
		number = (number<9?number+1:9);
		u8 idx = 22-number;
		if (masks[idx]) {
			s16 y_pos = lem->y - y_offset + y + (s16)lem->y_draw_offset - masks[idx]->height;
			s16 x_pos = lem->x - x_offset + x;
			for (yi=0;yi<masks[idx]->height;yi++) {
				if (yi+y_pos < y || yi+y_pos >= dest->height) {
					continue;
				}
				for (xi=0;xi<masks[idx]->width;xi++) {
					if (xi+x_pos < x || xi+x_pos >= dest->width) {
						continue;
					}
					if (masks[idx]->data[xi+yi*masks[idx]->width] & 0xF0) {
						// draw pixel
						SET_PIXEL(dest, xi+x_pos, yi+y_pos, palette[3]);
					}
				}
			}
		}
	}
}

void tile_menu_background(
		ScreenBuffer screen,
		struct MainMenuData* menu_data) {
	if (!initialized) {
		return;
	}
	struct Buffer* dest = getScreenBuffer(screen);
	if (!dest->data || !menu_data) {
		return;
	}
	clear(screen);
	s16 i,j;
	for (i=0;i<dest->width;i+=320) {
			for (j=0;j<dest->height;j+=104) {
				draw(
						screen,
						i,
						j,
						menu_data->static_pictures[0]->data,
						menu_data->static_pictures[0]->width,
						menu_data->static_pictures[0]->height,
						menu_data->palette);
			}
	}
}


int update_topscreen(struct MainMenuData* menu) {
	if (!initialized) {
		return 0;
	}
	tile_menu_background(TOP_SCREEN_BACK, menu);
	draw_scaled(
			TOP_SCREEN_BACK,
			10,
			20,
			menu->static_pictures[1]->data,
			menu->static_pictures[1]->width,
			menu->static_pictures[1]->height,
			menu->palette,
			0.6f);
	return 1;
}
