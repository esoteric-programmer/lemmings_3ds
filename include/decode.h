#ifndef DECODE_H
#define DECODE_H
#include <stdio.h>
#include <3ds.h>

struct CompressionHeader {
	s8 start_bits;
	s8 checksum;
	u16 size_dec;
	u16 size_enc;
};


struct Data {
	u16 size;
	s8 data[0];
};

int goto_section(FILE* input, u16 section);
struct Data* decompress_cur_section(FILE* input);
int planar_image_to_pixmap(
		u8* pixmap_image,
		void* planar_image,
		u16 num_of_pixels,
		u16 mask_offset);
#endif
