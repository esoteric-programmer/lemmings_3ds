#include <string.h>
#include "import_wave.h"

int wave_open_file(struct WaveFile* file, const char* filename) {
	if (!file) {
		return 0;
	}
	memset(file,0,sizeof(struct WaveFile));
	if (!filename) {
		return 0;
	}
	// open file
	FILE* f = fopen(filename,"rb");
	if (!f) {
		return 0;
	}
	// read RIFF chunk
	char buf[8];
	if (fread(buf,1,8,f) != 8) {
		fclose(f);
		return 0;
	}
	if (strncmp(buf,"RIFF",4)) {
		fclose(f);
		return 0;
	}
	u32 size_left = *((u32*)(buf+4));
	if (size_left < 4) {
		fclose(f);
		return 0;
	}
	if (fread(buf,1,4,f) != 4) {
		fclose(f);
		return 0;
	}
	size_left -= 4;
	if (strncmp(buf,"WAVE",4)) {
		fclose(f);
		return 0;
	}
	// now go through chunks,
	// ignore all chunks except of "data" and "fmt " chunk
	int fmt_chunk_read = 0;
	int data_chunk_read = 0;
	while (size_left && (!fmt_chunk_read || !data_chunk_read)) {
		// read chunk
		if (size_left < 8) {
			fclose(f);
			return 0;
		}
		if (fread(buf,1,8,f) != 8) {
			fclose(f);
			return 0;
		}
		size_left -= 8;
		u32 chunk_size = *((u32*)(buf+4));
		if (size_left < chunk_size) {
			fclose(f);
			return 0;
		}
		if (!strncmp(buf,"fmt ",4)) {
			// read "fmt " chunk
			if (chunk_size != 16) {
				fclose(f);
				return 0;
			}
			// format
			u16 tmp;
			if (fread(&tmp,1,2,f) != 2) {
				fclose(f);
				return 0;
			}
			if (tmp != 0x0001) {
				// not in PCM format -> not supported
				fclose(f);
				return 0;
			}
			// channels
			if (fread(&(file->channels),1,2,f) != 2) {
				fclose(f);
				return 0;
			}
			if (file->channels != 1 && file->channels != 2) {
				// neither mono nor stereo -> not supported
				fclose(f);
				return 0;
			}
			// sample rate
			if (fread(&(file->frequency),1,4,f) != 4) {
				fclose(f);
				return 0;
			}
			// bytes/sec
			u32 tmp2;
			if (fread(&tmp2,1,4,f) != 4) {
				fclose(f);
				return 0;
			}
			// frame size
			if (fread(&tmp,1,2,f) != 2) {
				fclose(f);
				return 0;
			}
			// bit depth
			if (fread(&file->bitdepth,1,2,f) != 2) {
				fclose(f);
				return 0;
			}
			if (file->bitdepth != 8 && file->bitdepth != 16) {
				// neither 8 bit nor 16 bit -> not supported
				fclose(f);
				return 0;
			}
			// check whether frame size is correct
			if (tmp != file->channels*((file->bitdepth+7)/8)) {
				fclose(f);
				return 0;
			}
			// check whether bytes per second are correct
			if (tmp2 != (u32)tmp * file->frequency) {
				fclose(f);
				return 0;
			}
			fmt_chunk_read = 1;
			size_left -= chunk_size;
			continue;
		}
		if (!strncmp(buf,"data",4)) {
			// read "data" chunk
			file->data_offset = ftell(f);
			file->data_size = chunk_size;
			// don't read data now, just skip it
			data_chunk_read = 1;
			u32 old_pos = ftell(f);
			fseek(f,chunk_size,SEEK_CUR);
			if (ftell(f)-old_pos != chunk_size) {
				fclose(f);
				return 0;
			}
			size_left -= chunk_size;
			continue;
		}
		// skip unsupported chunk
		u32 old_pos = ftell(f);
		fseek(f,chunk_size,SEEK_CUR);
		if (ftell(f)-old_pos != chunk_size) {
			fclose(f);
			return 0;
		}
		size_left -= chunk_size;
	}
	if (!fmt_chunk_read || !data_chunk_read) {
		// wave file does not contain all supported chunks
		fclose(f);
		return 0;
	}
	// success
	file->file = f;
	wave_rewind(file); // move to begin of data section
	return (file->file?1:0);
}

u32 wave_get_next_samples(void* buffer, u32 num_samples, struct WaveFile* file) {
	if (!file) {
		return 0;
	}
	if (!file->file) {
		return 0;
	}
	char* buf = (char*)buffer;
	u8 sample_size = file->channels*((file->bitdepth+7)/8);
	u32 samples_read;
	for (samples_read=0;samples_read<num_samples;samples_read++) {
		if (file->current_data_position+sample_size > file->data_size) {		
			return samples_read;
		}
		if (sample_size != fread(buf,1,sample_size,file->file)) {
			// error occured
			fclose(file->file);
			file->file = 0;
			return 0;
		}
		if (file->bitdepth == 8) {
			// convert unsigned to signed
			u8 j;
			for (j=0;j<sample_size;j++) {
				*((s8*)(buf+j)) = (s8)((s16)(*((u8*)(buf+j)))-128);
			}
		}
		buf += sample_size;
		file->current_data_position += sample_size;
	}
	return samples_read;
}

void wave_rewind(struct WaveFile* file) {
	if (!file) {
		return;
	}
	if (!file->file) {
		return;
	}
	file->current_data_position = 0;
	fseek(file->file,file->data_offset,SEEK_SET);
	if (ftell(file->file) != file->data_offset) {
		fclose(file->file);
		file->file = 0;
		return;
	}
}

void wave_close_file(struct WaveFile* file) {
	if (!file) {
		return;
	}
	if (file->file) {
		fclose(file->file);
	}
	memset(file,0,sizeof(struct WaveFile));
	return;
}

int import_wave_sound(struct WaveSound* dest, const char* filename) {
	if (!dest) {
		return 0;
	}
	memset(dest,0,sizeof(struct WaveSound));
	if (!filename) {
		return 0;
	}
	struct WaveFile file;
	if (!wave_open_file(&file, filename)) {
		return 0;
	}
	dest->channels = file.channels;
	dest->bitdepth = file.bitdepth;
	dest->frequency = file.frequency;
	dest->data = linearAlloc(file.data_size);
	if (!dest->data) {
		return 0;
	}
	dest->size = file.data_size;
	u32 samples = file.data_size / (file.channels * ((file.bitdepth+7)/8));
	dest->samples = wave_get_next_samples(
			dest->data,
			samples, // get all samples
			&file);
	wave_close_file(&file);
	if (dest->samples != samples) {
		linearFree(dest->data);
		dest->data = 0;
		return 0;
	}
	return 1;
}

