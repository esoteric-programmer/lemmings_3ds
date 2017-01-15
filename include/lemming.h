#ifndef LEMMING_H
#define LEMMING_H
#include "level.h"
#include "draw.h"
#include "main_data.h"
#include "settings.h"
#include "lemming_data.h"

extern const int action_image_offsets[18][2];

void init_lemmings(struct Level* level);
int add_lemming(struct Level* level);
u8 lemmings_left(struct LevelPlayer* player_state); // get number of lemmings that did not have entered the level yet.
void nuke(struct LevelPlayer* player);

void update_lemmings(
		struct Level*,
		struct Image* masks[23]);

// x,y: mouse position
// returns number of lemmings under cursor
u8 select_lemming(struct Lemming[MAX_NUM_OF_LEMMINGS], s16 x, s16 y, u8 right_mouse_btn, s16* lem1_idx, s16* lem2_idx);

// return 0 on failure, 1 on success
// skills: CLIMBING=0, FLOATING=1, etc. (ordering like at the skill panel)
u8 assign_skill(u8 skill, struct Lemming* lem1, struct Lemming* lem2, struct Level*);

const char* get_lemming_description(struct Lemming*); // returns ptr to pre-initialized string (no heap is reserved, do not free the pointer)

// get number of lemmings alive
int count_lemmings(struct Lemming lemmings[MAX_NUM_OF_LEMMINGS]);
#endif
