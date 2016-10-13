#ifndef IMPORT_WAVE_H
#define IMPORT_WAVE_H
#include <stdio.h>
#include <3ds.h>

struct WaveFile {
	FILE* file;
	u8 channels;
	u8 bitdepth;
	u32 frequency;

	u32 data_offset;
	u32 current_data_position;
	u32 data_size;
};

struct WaveSound {
	u8 channels;
	u8 bitdepth;
	u32 frequency;
	u32 samples;
	u16 size;
	void* data; // linearAlloc
};

// return: success (true) or failure (false)
// supported format: PCM wave, mono or stereo, 8 or 16 bit
int wave_open_file(struct WaveFile* file, const char* filename);

// return: number of samples read
u32 wave_get_next_samples(void* buffer, u32 num_samples, struct WaveFile* file);

void wave_rewind(struct WaveFile* file);

void wave_close_file(struct WaveFile* file);

// return: success (true) or failure (false)
int import_wave_sound(struct WaveSound* dest, const char* filename);

#endif
