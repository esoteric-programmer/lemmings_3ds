#include <malloc.h>
#include <string.h>
#include <3ds.h>
#include "decode.h"

static inline s8 get_bit(s8** in, s8* cur_bit_mask, s8* start_bits, s8* in_data) {
	// no check for nullptr to increase speed
	s8 ret;
	if (*start_bits == 0 || *cur_bit_mask == 0) {
		if (*in < in_data) {
			return 0; // ERROR: end of stream
		}
		*cur_bit_mask = 1;
		--*in;
		*start_bits = -1;
	}else if (*start_bits > 0) {
		--*start_bits;
	}
	ret = (((**in) & (*cur_bit_mask))?1:0);
	*cur_bit_mask <<= 1;
	return ret;
}

static inline s16 get_bits(s8** in, s8* cur_bit_mask, s8* start_bits, s8* in_data, s8 number) {
	s8 i;
	s16 ret = 0;
	for (i=number-1;i>=0;i--) {
		ret |= get_bit(in, cur_bit_mask, start_bits, in_data)<<i;
	}
	return ret;
}


int goto_section(FILE* input, u16 section) {
	if (fseek(input,0,SEEK_SET)) {
		return 0; // error
	}
	while(section > 0) {
		u16 size = 0;
		if (fseek(input,8,SEEK_CUR)) {
			return 0; // unexpected EOF or error reading file
		}
		if (2 != fread(&size,1,2,input)) {
			return 0; // unexpected EOF or error reading file
		}
		size = (size>>8) | (size<<8);
		if (size < 10) {
			return 0; // invalid file format
		}
		size-=10;
		if (fseek(input,size,SEEK_CUR)) {
			return 0; // unexpected EOF or error reading file
		}
		section--;
	}
	return 1;
}


struct Data* decompress_cur_section(FILE* input) {
	struct CompressionHeader header;
	memset(&header,0,sizeof(struct CompressionHeader));
	if (1 != fread(&(header.start_bits),1,1,input)) {
		return 0; // unexpected EOF or error reading file
	}
	if (1 != fread(&(header.checksum),1,1,input)) {
		return 0; // unexpected EOF or error reading file
	}
	if (fseek(input,2,SEEK_CUR)) {
		return 0; // unexpected EOF or error reading file
	}
	if (2 != fread(&(header.size_dec),1,2,input)) {
		return 0; // unexpected EOF or error reading file
	}
	if (fseek(input,2,SEEK_CUR)) {
		return 0; // unexpected EOF or error reading file
	}
	if (2 != fread(&(header.size_enc),1,2,input)) {
		return 0; // unexpected EOF or error reading file
	}
	header.size_dec = (header.size_dec>>8) | (header.size_dec<<8);
	header.size_enc = (header.size_enc>>8) | (header.size_enc<<8);
	//printf("header: %d, %d, %d.\n",header.start_bits, header.size_dec, header.size_enc);
	if (header.size_enc < 10) {
		return 0; // invalid file format
	}
	header.size_enc-=10;
	if (header.start_bits < 0 || header.start_bits > 8) {
		return 0; // invalid file format
	}
	//printf("found section: %d, %d, %d.\n",header.start_bits, header.size_dec, header.size_enc);
	struct Data* in = (struct Data*)malloc(sizeof(struct Data) + header.size_enc);
	if (in == 0) {
		fprintf(stderr,"out of memory\n");
		return 0; // out of memory
	}
	in->size = header.size_enc;
	if (header.size_enc != fread(in->data,1,header.size_enc,input)) {
		free(in);
		fprintf(stderr,"unexpected eof\n");
		return 0; // unexpected EOF or error reading file
	}
	s8* in_ptr = in->data + header.size_enc - 1;
	s8 in_mask = 1;

	s8 checksum = 0;
	u16 i;
	for (i=0;i<in->size;i++) {
		checksum ^= in->data[i];
	}
	if (checksum != header.checksum) {
		printf("Header checksum: %02X, Computed checksum: %02X\n",header.checksum,checksum);
	}

	struct Data* data = (struct Data*)malloc(sizeof(struct Data) + header.size_dec);
	if (data == 0) {
		free(in);
		fprintf(stderr,"out of memory\n");
		return 0; // out of memory
	}
	data->size = header.size_dec;

	//printf("start...\n");
	s8* data_ptr = data->data + header.size_dec;
	//int z=0;
	while (data_ptr > data->data) {
		//printf("dec %d\n",z);
		//z++;
		s16 type;
		if (get_bit(&in_ptr, &in_mask, &(header.start_bits),in->data)!=0) {
			type = get_bits(&in_ptr, &in_mask, &(header.start_bits),in->data,2)+2;
		}else{
			type = get_bit(&in_ptr, &in_mask, &(header.start_bits),in->data);
		}
		switch (type) {
			case 0:
				{
					//printf("t0\n");
					s16 n;
					s16 i;
					n = get_bits(&in_ptr, &in_mask, &(header.start_bits),in->data,3)+1;
					if (data_ptr-n < data->data) {
						free(data);
						free(in);
						return 0; // ERROR while decoding
					}
					for (i=0;i<n;i++) {
						data_ptr--;
						*data_ptr = (s8)get_bits(&in_ptr, &in_mask, &(header.start_bits),in->data,8);
						//printf("data: %02X\n",(unsigned char)(*data_ptr));
					}
				}
				break;
			case 1:
				{
					//printf("t1\n");
					s16 offset;
					s16 j;
					offset = get_bits(&in_ptr, &in_mask, &(header.start_bits),in->data,8)+1;
					if (data_ptr-2 < data->data) {
						free(data);
						free(in);
						return 0; // ERROR while decoding
					}
					for (j=0;j<2;j++) {
						data_ptr--;
						*data_ptr = data_ptr[offset];
						//printf("data: %02X\n",(unsigned char)(*data_ptr));
					}
				}
				break;
			case 2:
				{
					//printf("t2\n");
					s16 offset;
					s16 j;
					offset = get_bits(&in_ptr, &in_mask, &(header.start_bits),in->data,9)+1;
					if (data_ptr-3 < data->data) {
						free(data);
						free(in);
						return 0; // ERROR while decoding
					}
					for (j=0;j<3;j++) {
						data_ptr--;
						*data_ptr = data_ptr[offset];
						//printf("data: %02X\n",(unsigned char)(*data_ptr));
					}
				}
				break;
			case 3:
				{
					//printf("t3\n");
					s16 offset;
					s16 j;
					offset = get_bits(&in_ptr, &in_mask, &(header.start_bits),in->data,10)+1;
					if (data_ptr-4 < data->data) {
						free(data);
						free(in);
						return 0; // ERROR while decoding
					}
					for (j=0;j<4;j++) {
						data_ptr--;
						*data_ptr = data_ptr[offset];
						//printf("data: %02X\n",(unsigned char)(*data_ptr));
					}
				}
				break;
			case 4:
				{
					//printf("t4\n");
					s16 offset;
					s16 j;
					s16 n = get_bits(&in_ptr, &in_mask, &(header.start_bits),in->data,8)+1;
					offset = get_bits(&in_ptr, &in_mask, &(header.start_bits),in->data,12)+1;
					if (data_ptr-n < data->data) {
						free(data);
						free(in);
						return 0; // ERROR while decoding
					}
					for (j=0;j<n;j++) {
						data_ptr--;
						*data_ptr = data_ptr[offset];
						//printf("data: %02X\n",(unsigned char)(*data_ptr));
					}
				}
				break;
			case 5:
				{
					//printf("t5\n");
					s16 n;
					s16 i;
					n = get_bits(&in_ptr, &in_mask, &(header.start_bits),in->data,8)+9;
					if (data_ptr-n < data->data) {
						free(data);
						free(in);
						return 0; // ERROR while decoding
					}
					for (i=0;i<n;i++) {
						data_ptr--;
						*data_ptr = (s8)get_bits(&in_ptr, &in_mask, &(header.start_bits),in->data,8);
						//printf("data: %02X\n",(unsigned char)(*data_ptr));
					}
				}
				break;
			default:
				fprintf(stderr,"error parsing prefix\n");
				free(data);
				free(in);
				return 0; // ERROR while decoding
		}
	}

	free(in);
	return data;
}

int planar_image_to_pixmap(
		u8* pixmap_image,
		void* planar_image,
		u16 num_of_pixels,
		u16 mask_offset) {
	u8 plane;
	u16 pos;
	u8 in_mask = 0x80;
	u8 out_mask = 0x01;
	if (!planar_image || !pixmap_image || !num_of_pixels) {
		return 0; // error
	}
	memset(pixmap_image,0,num_of_pixels);
	u16 stream_pos = 0;
	for (plane=0;plane<4;plane++) {
		for (pos=0;pos<num_of_pixels;pos++) {
			pixmap_image[pos] |= ((in_mask & ((u8*)planar_image)[stream_pos])!=0?out_mask:0);
			in_mask >>= 1;
			if (in_mask == 0) {
				in_mask = 0x80;
				stream_pos++;
			}
		}
		out_mask <<= 1;
	}
	for (pos=0;pos<num_of_pixels;pos++) {
		if (pixmap_image[pos] != 0) {
			pixmap_image[pos] |= 0xF0; // add alpha channel
		}
	}
	if (mask_offset != 0) {
		// read transparency from mask
		in_mask = 0x80;
		stream_pos = mask_offset;
		for (pos=0;pos<num_of_pixels;pos++) {
			pixmap_image[pos] = (pixmap_image[pos] & 0x0F) | ((in_mask & ((u8*)planar_image)[stream_pos])?0xF0:0x00);
			in_mask >>= 1;
			if (in_mask == 0) {
				in_mask = 0x80;
				stream_pos++;
			}
		}
	}
	return 1; // success
}
