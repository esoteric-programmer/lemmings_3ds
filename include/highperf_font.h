#ifndef HIGHPERFFONT_H
#define HIGHPERFFONT_H
#include <3ds.h>
// the ingame fonts (high performance as well as normal) do not
// include all ASCII characters (see /doc/data/lemmings_main_dat_file_format.txt).
// to display level names, we need additional symbols for the font.
// these symbols are defined here.

// note that some printable ASCII characters still are not covered. e.g. { /
// so, in order to allow user levels (with arbitrary level names) these charactes have
// to be added here (and in draw.c file).

// 8x16 palette images
extern const u8 highperf_font_dot[];
extern const u8 highperf_font_comma[];
extern const u8 highperf_font_questionmark[];
extern const u8 highperf_font_exclamationmark[];
extern const u8 highperf_font_colon[];
extern const u8 highperf_font_bracket_l[];
extern const u8 highperf_font_bracket_r[];
extern const u8 highperf_font_apostrophe[];
extern const u8 highperf_font_quote[];
#endif
