#ifndef ADLIB_DAT_H
#define ADLIB_DAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "decode.h"
void add_sample(unsigned long length, s32* data, unsigned long param);

void OPL_Init(unsigned long sample_rate);
int OPL_LoadAdlibData(struct Data* decoded_adlib_dat);
int OPL_CallAdlib(u16 ax, int module);
void OPL_FreeAdlibData();
void OPL_QuerySamples(unsigned long samples, int module);
void OPL_ShutDown();

#ifdef __cplusplus
}
#endif

#endif
