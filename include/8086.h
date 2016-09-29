#ifndef EMU8086_H
#define EMU8086_H
#include "decode.h"
int call_adlib(u16 ax);
int load_adlib_data(struct Data* decoded_adlib_dat);
void free_adlib_data();
#endif
