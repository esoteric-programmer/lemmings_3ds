#include "import.h"
#include <string.h>
// read from datastream, write into image
// image_length: width*height
int read_image(u16 image_length, u16 mask_offset, void* datastream, u8* image) {
	u8 plane;
	u16 pos;
	u8 in_mask = 0x80;
	u8 out_mask = 0x01;
	if (!datastream || !image || !image_length) {
		return 0; // error
	}
	memset(image,0,image_length);
	u16 stream_pos = 0;
	for (plane=0;plane<4;plane++) {
		for (pos=0;pos<image_length;pos++) {
			image[pos] |= ((in_mask & ((u8*)datastream)[stream_pos])!=0?out_mask:0);
			in_mask >>= 1;
			if (in_mask == 0) {
				in_mask = 0x80;
				stream_pos++;
			}
		}
		out_mask <<= 1;
	}
	for (pos=0;pos<image_length;pos++) {
		if (image[pos] != 0) {
			image[pos] |= 0xF0; // add alpha channel
		}
	}
	if (mask_offset != 0) {
		// read transparency from mask
		in_mask = 0x80;
		stream_pos = mask_offset;
		for (pos=0;pos<image_length;pos++) {
			image[pos] = (image[pos] & 0x0F) | ((in_mask & ((u8*)datastream)[stream_pos])?0xF0:0x00);
			in_mask >>= 1;
			if (in_mask == 0) {
				in_mask = 0x80;
				stream_pos++;
			}
		}
	}
	return 1; // success
}

u32 ega2rgb(u8 ega) {
	// TODO: colors of FUN 22 are not correct...
	return (((ega & 0x4)?0x0000AA:0) | ((ega & 0x2)?0x00AA00:0) | ((ega & 0x1)?0xAA0000:0))
	+ ((ega & 0x10)?0x555555:0);
}

