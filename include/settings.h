#ifndef SETTINGS_H
#define SETTINGS_H
#include <3ds.h>

#define LEMMINGS_DIR "/lemmings" // if it does not exist, "." will be used instead
extern const char* PATH_ROOT;

#define MAX_NUM_OF_LEMMINGS   80
#define FPS                   17
#define SCREEN_WIDTH         320
#define FRAMES_PER_DOS_FRAME   3 // slow down 3ds animation...
#define FADE_IN_FRAMES        (18 * FRAMES_PER_DOS_FRAME) // duration of fade-in (level start)
#define FADE_OUT_FRAMES       (32 * FRAMES_PER_DOS_FRAME) // duration of fade-out (level end)

#define ENABLE_NUKE_GLITCH              1
#define ENABLE_ENTRANCE_PAUSING_GLITCH  1
#define ENABLE_SHRUGGER_GLITCH          0
#define ENABLE_MINING_ONEWAY_BUG        1
#endif
