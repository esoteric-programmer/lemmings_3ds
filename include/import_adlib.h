#ifndef ADLIB_DAT_H
#define ADLIB_DAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "decode.h"
void install_port_handler(unsigned long (*OPL_Read)(unsigned long,unsigned long), void (*OPL_Write)(unsigned long port,unsigned long val,unsigned long iolen));
void install_opl_handler(void (*handler)(unsigned long));
void add_sample(unsigned long length, s32* data);

void OPL_Init(unsigned long sample_rate);
void OPL_ShutDown();

int call_adlib(u16 ax);
int load_adlib_data(struct Data* decoded_adlib_dat);
void free_adlib_data();

#ifdef __cplusplus
}
#endif

#endif
