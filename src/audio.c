#include <3ds.h>
#include <string.h>
#include "adlib.dat.h"
#include "audio.h"

#define NUM_DSP_BUFFERS 32

u8 cur_song = 0;
u8 audio_active = AUDIO_ENABLED;

double millis_since_update_adlib_gone = 0.0;
u64 last_time = 0;
double samples_left = 0.0;

ndspWaveBuf ndsp_buffers[NUM_DSP_BUFFERS];
int next_ndsp_buffer = 0;
void (*opl_query_samples)(unsigned long) = 0; // sound callback
void install_opl_handler(void (*handler)(unsigned long)) {
	opl_query_samples = handler;
}

void add_sample(unsigned long length, s32* data) {
	if (audio_active == AUDIO_ERROR) {
		return;
	}
	while(ndsp_buffers[next_ndsp_buffer].status == NDSP_WBUF_PLAYING || ndsp_buffers[next_ndsp_buffer].status == NDSP_WBUF_QUEUED) {
		// wait
		aptMainLoop();
	}

	int i;
	for (i=0;i<length;i++) {
		*((s16*)((u8*)ndsp_buffers[next_ndsp_buffer].data_vaddr+2*i)) = 2*(data[i] < -0x4000?-0x4000:(data[i]> 0x3FFF?0x3FFF:data[i]));
	}
	ndsp_buffers[next_ndsp_buffer].nsamples = length;
	DSP_FlushDataCache(ndsp_buffers[next_ndsp_buffer].data_vaddr, 1024);
	ndspChnWaveBufAdd(0, ndsp_buffers+next_ndsp_buffer);

	next_ndsp_buffer++;
	next_ndsp_buffer%=NUM_DSP_BUFFERS;
}


void init_audio() {
	int i;
	if (ndspInit() != 0) {
		audio_active = AUDIO_ERROR;
		return;
	}
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
	ndspChnSetRate(0, 44100.0F);
	ndspChnSetFormat(0, NDSP_FORMAT_MONO_PCM16);
    ndspSetOutputCount(0);
	ndspChnReset(2);
	for (i=0; i<NUM_DSP_BUFFERS; i++) {
		memset(&ndsp_buffers[i], 0, sizeof(ndspWaveBuf));
		ndsp_buffers[i].data_vaddr = linearAlloc(1024);
		ndsp_buffers[i].looping = 0;
		ndsp_buffers[i].status = NDSP_WBUF_FREE;
	}
	OPL_Init(44100);
}

void start_audio() {
	stop_audio();
	if (audio_active == AUDIO_ERROR) {
		return;
	}
	millis_since_update_adlib_gone = 0.0;
	last_time = osGetTime() - 50;
	samples_left = 0.0;
	update_audio();
}

void start_music(u8 track){
	if (audio_active != AUDIO_ENABLED) {
		return;
	}
	call_adlib(0x0300+(u16)track);
}

void stop_audio() {
	call_adlib(0x0200);
	ndspChnWaveBufClear(0);
	int i;
	for (i=0; i<NUM_DSP_BUFFERS; i++) {
		ndsp_buffers[i].status = NDSP_WBUF_FREE;
	}
}

void play_sound(u8 sound) {
	if (audio_active == AUDIO_ERROR || audio_active == AUDIO_DISABLED) {
		return;
	}
	call_adlib(0x0400+(u16)sound);
}

void update_audio() {
	if (audio_active == AUDIO_ERROR || audio_active == AUDIO_DISABLED) {
		return;
	}
	u64 time;
	u32 millis_gone = 0;
	do {
		time = osGetTime();
		millis_gone = (u32)(time - last_time);
	}while(millis_gone < 1);
	if (millis_gone > 100) {
		millis_gone = 0; // probably sleep mode -> don't update...
		last_time = time;
		return;
	}
	last_time = time;
	double samples_gone = (double)millis_gone * 44.1;
	double new_samples_needed = samples_gone - samples_left;
	u32 n_of_new_samples = ((u32)new_samples_needed) + 1;
	samples_left = (double)n_of_new_samples - new_samples_needed;
	while(n_of_new_samples > 0) {
		u32 query_samples = (n_of_new_samples>255?255:n_of_new_samples);
		opl_query_samples(query_samples);
		millis_since_update_adlib_gone += (double)query_samples / 44.1;
		if (millis_since_update_adlib_gone >= 14.0) {
			call_adlib(0x0000);
			millis_since_update_adlib_gone -= 14.0;
		}
		n_of_new_samples -= query_samples;
	}
}

void deinit_audio() {
	stop_audio();
	OPL_ShutDown();
	ndspChnReset(0);
	ndspExit();
}

