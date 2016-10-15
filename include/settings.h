#ifndef SETTINGS_H
#define SETTINGS_H
#include <3ds.h>

#define NO_SF2D

#define LEMMINGS_DIR "/lemmings" // if it does not exist, "." will be used instead
extern const char* PATH_ROOT;

#define MAX_NUM_OF_LEMMINGS   80
#define FPS                   17 // dosframes per second (for time counter in level)
#define SCREEN_WIDTH         320
#define MS_PER_FRAME          71 // duration of frame in milliseconds
#define MS_PER_FRAME_SUPERLEM 28 // frame duration in "introducing superlemming"
#define INPUT_SAMPLING_MILLIS 25 // affects cursor speed and so on...
#define MS_PER_FRAME_SPEED_UP 20
#define FADE_IN_DOSFRAMES     18
#define FADE_OUT_DOSFRAMES    32

#define ENABLE_NUKE_GLITCH              1
#define ENABLE_ENTRANCE_PAUSING_GLITCH  1
#define ENABLE_SHRUGGER_GLITCH          0
#define ENABLE_MINING_ONEWAY_BUG        1

#ifdef NO_SF2D
#define ABGR
#endif
#endif
