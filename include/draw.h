#ifndef DRAW_H
#define DRAW_H
#include <3ds.h>
#include <sf2d.h>
#include "settings.h"
#include "main_data.h"
#include "level.h"
#include "lemming_data.h"

// data structure for RGB image.
// (almost) all game graphics are drawn to this data structure before a sf2dtexture will be created from it.
struct RGB_Image {
	s16 width;
	s16 height;
	u32 data[0];
};

// scale with linear interpolation (sf2d seems o scale without interpolation)
void scale(struct RGB_Image* dest, float factor, struct RGB_Image* source);

// draw palette image into RGB image at specific position
int draw(struct RGB_Image* image, u32 palette[16], const u8* img, s16 x, s16 y, u16 w, u16 h);

// draw level view at top of RGB image while (without info text, toolbar, and lemmings)
int draw_level(struct RGB_Image* image, struct Level* level);

// draw a string using highperf-font into an RGB image
void draw_highperf_text(struct RGB_Image* image, struct MainInGameData* data,
		s16 y_offset, const char* text);

// draw a string using menu-font into an RGB image
void draw_menu_text(struct RGB_Image* image, struct MainMenuData* data,
		s16 x_offset, s16 y_offset, const char* text);

// draw a string using scaled (interpolated) menu-font into an RGB image
void draw_menu_text_small(struct RGB_Image* image, struct MainMenuData* data,
		s16 x_offset, s16 y_offset, const char* text);

// draw toolbar at bottom of RGB image
int draw_toolbar(struct RGB_Image* image, struct MainInGameData* data,
		struct Level* level, struct LevelState* state,
		struct Lemming[MAX_NUM_OF_LEMMINGS], const char* text);

// return number of lemmings drawn
u8 draw_lemmings(struct Lemming[MAX_NUM_OF_LEMMINGS], struct Image* lemmings_anim[337], struct Image* masks[23], u32 palette[16], struct RGB_Image* image, u16 x_offset);

// draw menu background into im_bottom (tiled)
void tile_menu_background(struct RGB_Image* im_bottom, struct MainMenuData* menu_data);

int draw_topscreen(struct MainMenuData* menu, struct RGB_Image* im_top, sf2d_texture** texture_top_screen, struct RGB_Image* logo_scaled, sf2d_texture** texture_logo);
#endif
