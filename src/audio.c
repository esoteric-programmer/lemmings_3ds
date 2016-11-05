#include <malloc.h>
#include <string.h>
#include <3ds.h>
#include "import_adlib.h"
#include "import_wave.h"
#include "settings.h"
#include "gamespecific.h"
#include "audio.h"

// if the values below are too large, sound effects get delayed
// if the values below are too small, jitters appear
#define NUM_DSP_BUFFERS 10
// note that OPL emulation does only generate a limited number of samples
// each time it is called
#define SAMPLES_PER_DSP_BUFFER 255

#define NUM_TUNE_BUFFERS 100
#define SAMPLES_PER_TUNE_BUFFER 1024

// if wave files are used (instead of ADLIB.DAT) for sfx effects,
// DSP is used to mix them.
// this is the number of channels reserve for that task (at most 21).
// For comparison: ADLIB.DAT driver uses 3 channels for sound effects.
#define NUM_DSP_SFX_CHANNELS 16

// general
u8 cur_song = 0;
u8 audio_error_occured = 0;

// wave
struct WaveSound sound_effects[18];
ndspWaveBuf sound_effect_buffers[NUM_DSP_SFX_CHANNELS];
u8 currently_triggered_wave[18];

struct WaveFile tune;
ndspWaveBuf tune_buffers[NUM_TUNE_BUFFERS];
int next_tune_buffer = 0;
int play_wave_tune = 0;
int last_preloaded_tunebuffer = -1;

// adlib
struct Data* adlib = 0;
double millis_since_update_adlib_gone = 0.0;
ndspWaveBuf ndsp_buffers[NUM_DSP_BUFFERS];
int next_ndsp_buffer = 0;
void (*opl_query_samples)(unsigned long) = 0; // sound callback
void install_opl_handler(void (*handler)(unsigned long)) {
	opl_query_samples = handler;
}

void add_sample(unsigned long length, s32* data) {
	if (audio_error_occured) {
		return;
	}
	if(ndsp_buffers[next_ndsp_buffer].status == NDSP_WBUF_PLAYING
			|| ndsp_buffers[next_ndsp_buffer].status == NDSP_WBUF_QUEUED) {
		// should not occur
		return;
	}

	int i;
	for (i=0;i<length;i++) {
		*((s16*)((u8*)ndsp_buffers[next_ndsp_buffer].data_vaddr+2*i)) =
				2*(data[i] < -0x4000?-0x4000:(data[i]> 0x3FFF?0x3FFF:data[i]));
	}
	ndsp_buffers[next_ndsp_buffer].nsamples = length;
	DSP_FlushDataCache(
			ndsp_buffers[next_ndsp_buffer].data_vaddr,
			2*SAMPLES_PER_DSP_BUFFER);
	ndspChnWaveBufAdd(0, &ndsp_buffers[next_ndsp_buffer]);

	next_ndsp_buffer++;
	next_ndsp_buffer%=NUM_DSP_BUFFERS;
}

void update_audio() {
	if (audio_error_occured) {
		return;
	}
	if (!settings.music_volume && !settings.sfx_volume) {
		return;
	}
	int i;
	for (i=0;i<18;i++) {
		currently_triggered_wave[i] = 0;
	}
	while(ndsp_buffers[next_ndsp_buffer].status != NDSP_WBUF_PLAYING
			&& ndsp_buffers[next_ndsp_buffer].status != NDSP_WBUF_QUEUED) {
		opl_query_samples(SAMPLES_PER_DSP_BUFFER);
		millis_since_update_adlib_gone += (double)SAMPLES_PER_DSP_BUFFER / 44.1;
		if (millis_since_update_adlib_gone >= 14.0) {
			call_adlib(0x0000);
			millis_since_update_adlib_gone -= 14.0;
		}
	}
	if (tune.file && play_wave_tune && settings.music_volume) {
		while(tune_buffers[next_tune_buffer].status != NDSP_WBUF_PLAYING
				&& tune_buffers[next_tune_buffer].status != NDSP_WBUF_QUEUED) {
			if (last_preloaded_tunebuffer < 0) {
				tune_buffers[next_tune_buffer].nsamples =
						wave_get_next_samples(
								tune_buffers[next_tune_buffer].data_pcm8,
								SAMPLES_PER_TUNE_BUFFER,
								&tune);
				if (!tune_buffers[next_tune_buffer].nsamples) {
					wave_rewind(&tune);
					tune_buffers[next_tune_buffer].nsamples =
							wave_get_next_samples(
									tune_buffers[next_tune_buffer].data_pcm8,
									SAMPLES_PER_TUNE_BUFFER,
									&tune);
					if (!tune_buffers[next_tune_buffer].nsamples) {
						wave_close_file(&tune);
						break;
					}
				}
			}
			u8 sample_size = tune.channels*((tune.bitdepth+7)/8);
			DSP_FlushDataCache(
					tune_buffers[next_tune_buffer].data_vaddr,
							sample_size*tune_buffers[next_tune_buffer].nsamples);
			ndspChnWaveBufAdd(2, &tune_buffers[next_tune_buffer]);

			if (last_preloaded_tunebuffer == next_tune_buffer) {
				last_preloaded_tunebuffer = -1;
			}

			next_tune_buffer++;
			next_tune_buffer%=NUM_TUNE_BUFFERS;
		}
	}
}

// general

void init_audio() {
	int i;
	if (ndspInit() != 0) {
		audio_error_occured = 1;
		return;
	}
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	for (i=0;i<NUM_DSP_SFX_CHANNELS+3;i++) {
		ndspChnReset(i); // necessary?
		ndspChnSetInterp(i, NDSP_INTERP_LINEAR);
		ndspChnSetRate(i, 44100.0F);
		ndspChnSetFormat(i, NDSP_FORMAT_MONO_PCM16);
	}
    ndspSetOutputCount(0); // correct?
	for (i=0; i<NUM_DSP_BUFFERS; i++) {
		memset(&ndsp_buffers[i], 0, sizeof(ndspWaveBuf));
		ndsp_buffers[i].data_vaddr = linearAlloc(2*SAMPLES_PER_DSP_BUFFER);
		if (!ndsp_buffers[i].data_vaddr) {
			audio_error_occured = 1;
			return;
		}
		ndsp_buffers[i].looping = 0;
		ndsp_buffers[i].status = NDSP_WBUF_FREE;
	}
	for (i=0; i<NUM_TUNE_BUFFERS; i++) {
		memset(&tune_buffers[i], 0, sizeof(ndspWaveBuf));
		tune_buffers[i].data_vaddr = linearAlloc(4*SAMPLES_PER_TUNE_BUFFER);
		if (!tune_buffers[i].data_vaddr) {
			audio_error_occured = 1;
			return;
		}
		tune_buffers[i].looping = 0;
		tune_buffers[i].status = NDSP_WBUF_FREE;
	}
	for (i=0;i<NUM_DSP_SFX_CHANNELS;i++) {
		sound_effect_buffers[i].status = NDSP_WBUF_FREE;
	}
	memset(&tune,0,sizeof(struct WaveSound));
	OPL_Init(44100);
	millis_since_update_adlib_gone = 0.0;
	memset(sound_effects,0,18*sizeof(struct WaveSound));
	memset(sound_effect_buffers,0,NUM_DSP_SFX_CHANNELS*sizeof(ndspWaveBuf));
	for (i=0;i<18;i++) {
		char sound_fn[64];
		sprintf(sound_fn,"%s/audio/SFX%02u.WAV", PATH_ROOT, i+1);
		import_wave_sound(sound_effects+i, sound_fn);
		currently_triggered_wave[i] = 0;
	}
	play_wave_tune = 0;
	last_preloaded_tunebuffer = -1;
}

int audio_error() {
	return audio_error_occured;
}

int import_audio(u8 game) {
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

u16 next_adlib_music = 0x300;
void prepare_music(u8 game, u8 lvl) {
	u8 track;
	int i;
	play_wave_tune = 0;
	last_preloaded_tunebuffer = -1;
	if (audio_error_occured || !settings.music_volume) {
		next_adlib_music = 0x300;
		return;
	}
	track = import[game].song_ids[cur_song];
	for (i=0;i<import[game].num_special_songs;i++) {
		if (import[game].special_songs[i].level == lvl) {
			track = import[game].special_songs[i].song;
			break;
		}
	}
	char tune_fn[64];
	sprintf(
			tune_fn,
			"%s/audio/%s/TUNE%02u.WAV",
			PATH_ROOT,
			import[game].custom_audio_path,
			track);
	if (!wave_open_file(&tune, tune_fn)) {
		next_adlib_music = 0x0300+(u16)track;
	}else{
		next_adlib_music = 0x300;
		ndspChnReset(2); // necessary?
		ndspChnSetInterp(2, NDSP_INTERP_LINEAR);
		ndspChnSetRate(2, tune.frequency);
		if (tune.bitdepth == 8) {
			if (tune.channels == 1) {
				ndspChnSetFormat(2, NDSP_FORMAT_MONO_PCM8);
			}else {
				ndspChnSetFormat(2, NDSP_FORMAT_STEREO_PCM8);
			}
		}else{
			if (tune.channels == 1) {
				ndspChnSetFormat(2, NDSP_FORMAT_MONO_PCM16);
			}else {
				ndspChnSetFormat(2, NDSP_FORMAT_STEREO_PCM16);
			}
		}
		int i;
		for (i=0;i<NUM_TUNE_BUFFERS;i++) {
			int buf_id = (next_tune_buffer+i)%NUM_TUNE_BUFFERS;
			tune_buffers[buf_id].nsamples =
					wave_get_next_samples(
							tune_buffers[buf_id].data_pcm8,
							SAMPLES_PER_TUNE_BUFFER,
							&tune);
			if (!tune_buffers[buf_id].nsamples) {
				wave_rewind(&tune);
				tune_buffers[buf_id].nsamples =
						wave_get_next_samples(
								tune_buffers[buf_id].data_pcm8,
								SAMPLES_PER_TUNE_BUFFER,
								&tune);
				if (!tune_buffers[buf_id].nsamples) {
					wave_close_file(&tune);
					break;
				}
			}
			last_preloaded_tunebuffer = buf_id;
		}
	}
}

void play_music() {
	if (next_adlib_music > 0x300) {
		call_adlib(next_adlib_music);
		return;
	}
	if (tune.file) {
		play_wave_tune = 1;
		return;
	}
}

void stop_audio() {
	int i;
	call_adlib(0x0200);
	wave_close_file(&tune);
	for (i=0;i<NUM_DSP_SFX_CHANNELS+3;i++) {
		ndspChnWaveBufClear(i);
	}
	for (i=0;i<NUM_DSP_SFX_CHANNELS;i++) {
		sound_effect_buffers[i].status = NDSP_WBUF_FREE;
	}
	for (i=0; i<NUM_DSP_BUFFERS; i++) {
		ndsp_buffers[i].status = NDSP_WBUF_FREE;
	}
	for (i=0; i<NUM_TUNE_BUFFERS; i++) {
		tune_buffers[i].status = NDSP_WBUF_FREE;
	}
	millis_since_update_adlib_gone = 0.0;
	for (i=0;i<18;i++) {
		currently_triggered_wave[i] = 0;
	}
	play_wave_tune = 0;
	last_preloaded_tunebuffer = -1;
}

int is_custom_sound(u8 sound) {
	if (sound > 0 && sound <= 18) {
		if (sound_effects[sound-1].data) {
			return 1;
		}
	}
	return 0;
}

void play_sound(u8 sound) {
	int wave = 0;
	if (audio_error_occured || !settings.sfx_volume) {
		return;
	}
	if (sound > 0 && sound <= 18) {
		if (currently_triggered_wave[sound-1]) {
			return; // already triggered
		}
		currently_triggered_wave[sound-1] = 1;
		if (sound_effects[sound-1].data) {
			wave = 1; // don't play adlib sound
			int i;
			for (i=0;i<NUM_DSP_SFX_CHANNELS;i++) {
				if (!ndspChnIsPlaying(3+i)
						&& (sound_effect_buffers[i].status == NDSP_WBUF_FREE
							|| sound_effect_buffers[i].status == NDSP_WBUF_DONE)) {
					// play sound
					ndspChnWaveBufClear(3+i);
					ndspChnReset(3+i);
					ndspChnSetRate(3+i, sound_effects[sound-1].frequency);
					if (sound_effects[sound-1].bitdepth == 8) {
						if (sound_effects[sound-1].channels == 1) {
							ndspChnSetFormat(3+i, NDSP_FORMAT_MONO_PCM8);
						}else {
							ndspChnSetFormat(3+i, NDSP_FORMAT_STEREO_PCM8);
						}
					}else{
						if (sound_effects[sound-1].channels == 1) {
							ndspChnSetFormat(3+i, NDSP_FORMAT_MONO_PCM16);
						}else {
							ndspChnSetFormat(3+i, NDSP_FORMAT_STEREO_PCM16);
						}
					}
					sound_effect_buffers[i].status = NDSP_WBUF_FREE;
					memset(&sound_effect_buffers[i], 0, sizeof(ndspWaveBuf));
					sound_effect_buffers[i].looping = 0;
					sound_effect_buffers[i].nsamples = sound_effects[sound-1].samples;
					sound_effect_buffers[i].data_vaddr = sound_effects[sound-1].data;
					DSP_FlushDataCache(
							sound_effect_buffers[i].data_vaddr,
							sound_effects[sound-1].size);
					ndspChnWaveBufAdd(3+i, &sound_effect_buffers[i]);
					return;
				}
			}
		}
	}
	if (!wave) {
		call_adlib(0x0400+(u16)sound);
	}
}


void deinit_audio() {
	stop_audio();
	OPL_ShutDown();
	ndspChnReset(0);
	ndspExit();
}
