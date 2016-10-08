#include <malloc.h>
#include <string.h>
#include <3ds.h>
#include "adlib/adlib.dat.h"
#include "settings.h"
#include "gamespecific.h"
#include "audio.h"

// if the values below are too large, sound effects get delayed
// if the values below are too small, jitters appear
#define NUM_DSP_BUFFERS 10
// note that OPL emulation does only generate a limited number of samples
// each time it is called
#define SAMPLES_PER_DSP_BUFFER 255

#define AUDIO_ERROR 0
#define AUDIO_DISABLED 1
#define AUDIO_ONLY_FX 2
#define AUDIO_ENABLED 3

u8 cur_song = 0;
u8 audio_active = AUDIO_ENABLED;
struct Data* adlib = 0;

double millis_since_update_adlib_gone = 0.0;

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
	if(ndsp_buffers[next_ndsp_buffer].status == NDSP_WBUF_PLAYING || ndsp_buffers[next_ndsp_buffer].status == NDSP_WBUF_QUEUED) {
		// should not occur
		return;
	}

	int i;
	for (i=0;i<length;i++) {
		*((s16*)((u8*)ndsp_buffers[next_ndsp_buffer].data_vaddr+2*i)) = 2*(data[i] < -0x4000?-0x4000:(data[i]> 0x3FFF?0x3FFF:data[i]));
	}
	ndsp_buffers[next_ndsp_buffer].nsamples = length;
	DSP_FlushDataCache(ndsp_buffers[next_ndsp_buffer].data_vaddr, 2*SAMPLES_PER_DSP_BUFFER);
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
		ndsp_buffers[i].data_vaddr = linearAlloc(2*SAMPLES_PER_DSP_BUFFER);
		if (!ndsp_buffers[i].data_vaddr) {
			audio_active = AUDIO_ERROR;
		}
		ndsp_buffers[i].looping = 0;
		ndsp_buffers[i].status = NDSP_WBUF_FREE;
	}
	OPL_Init(44100);
	millis_since_update_adlib_gone = 0.0;
}

int toggle_audio() {
	switch (audio_active) {
		case AUDIO_DISABLED:
			audio_active = AUDIO_ONLY_FX;
			return 1;
		case AUDIO_ONLY_FX:
			audio_active = AUDIO_ENABLED;
			return 1;
		case AUDIO_ENABLED:
			audio_active = AUDIO_DISABLED;
			return 1;
		default:
			return 0;
	}
}

int is_audio_enabled() {
	return (audio_active == AUDIO_ENABLED);
}

int is_audio_only_fx() {
	return (audio_active == AUDIO_ONLY_FX);
}

int import_audio(u8 game) {
	// additionally: read ADLIB
	if (adlib) {
		free_adlib_data();
		free(adlib);
		adlib = 0;
	}
	char adlib_fn[64];
	sprintf(adlib_fn,"%s/%s/ADLIB.DAT", PATH_ROOT, import[game].path);
	FILE* adlib_file = fopen(adlib_fn,"rb");
	if (!adlib_file) {
		return 0; // failed
	}
	adlib = decompress_cur_section(adlib_file);
	load_adlib_data(adlib);
	cur_song = 0;
	return 1; // success
}


void next_music(u8 game) {
	cur_song = (cur_song + 1) % import[game].num_of_songs;
}

void play_music(u8 game, u8 lvl) {
	u8 track;
	int i;
	if (audio_active != AUDIO_ENABLED) {
		return;
	}
	track = import[game].song_ids[cur_song];
	for (i=0;i<import[game].num_special_songs;i++) {
		if (import[game].special_songs[i].level == lvl) {
			track = import[game].special_songs[i].song;
			break;
		}
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
	millis_since_update_adlib_gone = 0.0;
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

	while(ndsp_buffers[next_ndsp_buffer].status != NDSP_WBUF_PLAYING && ndsp_buffers[next_ndsp_buffer].status != NDSP_WBUF_QUEUED) {
		opl_query_samples(SAMPLES_PER_DSP_BUFFER);
		millis_since_update_adlib_gone += (double)SAMPLES_PER_DSP_BUFFER / 44.1;
		if (millis_since_update_adlib_gone >= 14.0) {
			call_adlib(0x0000);
			millis_since_update_adlib_gone -= 14.0;
		}
	}
}

void deinit_audio() {
	stop_audio();
	OPL_ShutDown();
	ndspChnReset(0);
	ndspExit();
}
