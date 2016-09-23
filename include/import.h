#ifndef IMPORT_H
#define IMPORT_H
#include <3ds.h>
int read_image(u16 image_length, u16 mask_offset, void* datastream, u8* image);
u32 ega2rgb(u8 ega);
#endif
