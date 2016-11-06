#include "import_adlib.h"
#include "adlib/8086.h"

static Emu8086* emu8086[2] = {0, 0};

void OPL_Init(unsigned long sample_rate) {
	if (emu8086[0]) {
		return;
	}
	emu8086[0] = new Emu8086(sample_rate);
	emu8086[1] = new Emu8086(sample_rate);
}

int OPL_LoadAdlibData(struct Data* decoded_adlib_dat) {
	if (emu8086[0]) {
		if (!emu8086[0]->load_adlib_data(decoded_adlib_dat)) {
			return 0;
		}
	}
	if (emu8086[1]) {
		return emu8086[1]->load_adlib_data(decoded_adlib_dat);
	}
	return 1;
}

int OPL_CallAdlib(u16 ax, int module) {
	if (module >= 0 && module < 2) {
		if (emu8086[module]) {
			return emu8086[module]->call_adlib(ax);
		}
	}
	return 0;
}

void OPL_FreeAdlibData() {
	if (emu8086[0]) {
		emu8086[0]->free_adlib_data();
	}
	if (emu8086[1]) {
		emu8086[1]->free_adlib_data();
	}
}

void OPL_QuerySamples(unsigned long samples, int module) {
	if (module >= 0 && module < 2) {
		if (emu8086[module]) {
			emu8086[module]->query_opl_samples(samples, module);
		}
	}	
}

void OPL_ShutDown(){
	if (emu8086[0]) {
		delete emu8086[0];
	}
	if (emu8086[1]) {
		delete emu8086[1];
	}
	emu8086[0] = 0;
	emu8086[1] = 0;
}
