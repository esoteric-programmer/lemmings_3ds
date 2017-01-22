#ifndef MAIN_H
#define MAIN_H
#define SUCCESS(res) (R_SUCCEEDED(res) && \
	(R_LEVEL(res) == RL_SUCCESS \
	  || R_SUMMARY(res) == RS_SUCCESS \
	  || R_DESCRIPTION(res) == RD_SUCCESS))

#define FAILED(res) (!SUCCESS(res))

int was_suspended();
#endif
