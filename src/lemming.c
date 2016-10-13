#include <string.h>
#include "lemming.h"
#include "audio.h"

void process_interactive_objects(struct Lemming* lem, struct Level* level);

//void (*const lemming_do_action[18])(struct Lemming*, struct Level*);
int lemming_walk(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_splatter(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_fall(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_jump(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_climb(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_hoist(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_float(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_build(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_shrug(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_exit(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_drown(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_fry(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_block(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_explode(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_ohno(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_dig(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_mine(struct Lemming*, struct Level*, struct Image* masks[23]);
int lemming_bash(struct Lemming*, struct Level*, struct Image* masks[23]);

//void (*const lemming_switch_action[18])(struct Lemming*)
void lemming_start_splatter(struct Lemming*);
void lemming_start_fall(struct Lemming*);
void lemming_start_float(struct Lemming*);
void lemming_start_build(struct Lemming*);
void lemming_start_block(struct Lemming*);
void lemming_start_mine(struct Lemming*);
void lemming_start_dig(struct Lemming*);
void switch_action_dummy(struct Lemming*); // dummy

int assign_climb(struct Lemming*, struct Lemming*, struct Level*);
int assign_float(struct Lemming*, struct Lemming*, struct Level*);
int assign_bomb(struct Lemming*, struct Lemming*, struct Level*);
int assign_block(struct Lemming*, struct Lemming*, struct Level*);
int assign_build(struct Lemming*, struct Lemming*, struct Level*);
int assign_bash(struct Lemming*, struct Lemming*, struct Level*);
int assign_mine(struct Lemming*, struct Lemming*, struct Level*);
int assign_dig(struct Lemming*, struct Lemming*, struct Level*);

int (*const lemming_do_action[18])(struct Lemming*, struct Level*, struct Image* masks[]) = {
	lemming_walk,
	lemming_splatter,
	lemming_explode,
	lemming_fall,
	lemming_jump,
	lemming_dig,
	lemming_climb,
	lemming_hoist,
	lemming_build,
	lemming_block,
	lemming_bash,
	lemming_float,
	lemming_mine,
	lemming_drown,
	lemming_exit,
	lemming_fry,
	lemming_ohno,
	lemming_shrug
};


const struct {
	s8 x;
	s8 y;
} action_draw_offsets[18] = {
	{-8, -10}, // WALK
	{-8, -10}, // SPLATTER
	{-16,-25}, // EXPLODE
	{-8, -10}, // FALL
	{-8, -10}, // JUMP
	{-8,-12}, // DIG
	{-8, -12}, // CLIMB
	{-8,-12}, // HOIST
	{-8,-13}, // BUILD
	{-8,-10}, // BLOCK
	{-8,-10}, // BASH
	{-8, -16}, // FLOAT
	{-8,-13}, // MINE
	{-8,-10}, // DROWN
	{-8,-13}, // EXIT
	{-8,-14}, // FRY
	{-8,-10}, // OHNO
	{-8,-10} // SHRUG
};

const int action_image_offsets[18][2] = { // for each action: left, right
	{9, 0}, // WALK
	{250, 250}, // SPLATTER
	{336, 336}, // EXPLODE
	{230, 226}, // FALL
	{17, 8}, // JUMP
	{18, 18}, // DIG
	{42, 34}, // CLIMB
	{74, 66}, // HOIST // end of climbing
	{98, 82}, // BUILD
	{288, 288}, // BLOCK
	{146, 114}, // BASH
	{242, 234}, // FLOAT
	{202, 178}, // MINE
	{50, 50}, // DROWN // in water
	{266, 266}, // EXIT
	{274, 274}, // FRY // killed by flameblower etc.
	{320, 320}, // OHNO
	{312, 304}, // SHRUG
};

struct {
	s8 dy;
	s8 frame_offset;
} float_param[16] = {
	{3,1},
	{3,2},
	{3,3},
	{3,5},
	{-1,5},
	{0,5},
	{1,5},
	{1,5},
	{2,5},
	{2,6},
	{2,7},
	{2,7},
	{2,6},
	{2,5},
	{2,4},
	{2,4}
};

void (*const lemming_switch_action[18])(struct Lemming*) = {
	switch_action_dummy, // WALK; nothing special needed
	lemming_start_splatter,
	switch_action_dummy, // EXPLODE; nothing special needed
	lemming_start_fall,
	switch_action_dummy, // JUMP; nothing special needed
	lemming_start_dig,
	switch_action_dummy, // CLIMB; nothing special needed
	switch_action_dummy, // HOIST; nothing special needed
	lemming_start_build,
	lemming_start_block,
	switch_action_dummy,
	lemming_start_float,
	lemming_start_mine,
	switch_action_dummy,
	switch_action_dummy,
	switch_action_dummy,
	switch_action_dummy,
	switch_action_dummy
};

int (*const lemming_assign[8])(struct Lemming*, struct Lemming*, struct Level*) = {
	assign_climb,
	assign_float,
	assign_bomb,
	assign_block,
	assign_build,
	assign_bash,
	assign_mine,
	assign_dig
};

void set_lemaction(struct Lemming* lem, u8 action);
u8 nuking;
u8 timer_assign;
u8 next_lemming_id;
u8 next_lemming_countdown;


static inline int has_pixel_at(struct Level* level, s16 x, s16 y) {
	if (!level) {
		return 0;
	}
	if (x < 0 || x >= 1584 || y < 0 || y >= 160) {
		return 0;
	}
	return ((level->terrain[x+y*1584] & 0xF0)?1:0);
}

static inline void check_top_collision(struct Lemming* lem) {
	if (lem->y + lem->y_draw_offset <= LEM_MIN_Y) {
		lem->y = LEM_MIN_Y - lem->y_draw_offset - 2; // why -2?
		lem->look_right = !lem->look_right;
		if (lem->current_action == LEMACTION_JUMP) { // not when climbing etc.?
			set_lemaction(lem, LEMACTION_WALK);
		}
	}
}

static inline u8 read_object_map(struct Level* level, s16 x, s16 y) {
	if (x<0 || x >= 1584 || y < 0 || y >= 160) {
		return 0;
	}
	x /= 4;
	y /= 4;
	return level->object_map[x+y*(1584/4)];
}

static inline void write_object_map(struct Level* level, s16 x, s16 y, u8 value) {
	if (x<0 || x >= 1584 || y < 0 || y >= 160) {
		return;
	}
	x /= 4;
	y /= 4;
	level->object_map[x+y*(1584/4)] = value;
}

static inline void blocker_restore_object_map(struct Lemming* lem, struct Level* level) {
	s16 i,j;
	for (i=0;i<3;i++) {
		for (j=0;j<3;j++) {
			write_object_map(level, lem->x+4*(i-1), lem->y-2+4*(j-1), lem->saved_object_map[i+3*j]);
		}
	}
	lem->blocking = 0;
}

static inline void blocker_modify_object_map(struct Lemming* lem, struct Level* level) {
	s16 i,j;
	for (i=0;i<3;i++) {
		for (j=0;j<3;j++) {
			lem->saved_object_map[i+3*j] = read_object_map(level, lem->x+4*(i-1), lem->y-2+4*(j-1));
			write_object_map(level, lem->x+4*(i-1), lem->y-2+4*(j-1), (i==0?OBJECT_FORCE_LEFT:(i==2?OBJECT_FORCE_RIGHT:OBJECT_BLOCKER)));
		}
	}
	lem->blocking = 1;
}

static inline int blocker_present(struct Lemming* lem, struct Level* level) {
	s16 i,j;
	for (i=0;i<3;i++) {
		for (j=0;j<3;j++) {
			switch (read_object_map(level, lem->x+4*(i-1), lem->y-2+4*(j-1))) {
				case OBJECT_FORCE_LEFT:
				case OBJECT_FORCE_RIGHT:
				case OBJECT_BLOCKER:
					return 1;
				default:
					break;
			}
		}
	}
	return 0;
}

static inline void apply_mask(struct Image* mask, struct Level* level, s16 x_pos, s16 y_pos) {
	if (!mask || !level) {
		return;
	}
	s16 x,y;
	for (y=0;y<mask->height;y++) {
		if (y_pos+y < 0 || y_pos+y >= 160) {
			continue;
		}
		for (x=0;x<mask->width;x++) {
			if (x_pos+x < 0 || x_pos+x >= 1584) {
				continue;
			}
			if (mask->data[x+mask->width*y]) {
				// remove terrain
				level->terrain[x_pos+x + (y_pos+y)*1584] = 0x00;
			}
		}
	}
}


void init_lemmings(struct Lemming lemmings[MAX_NUM_OF_LEMMINGS]) {
	nuking = 0;
	timer_assign = 0;
	next_lemming_id = 0;
	next_lemming_countdown = 20;

	if (lemmings) {
		int i;
		for (i=0;i<MAX_NUM_OF_LEMMINGS;i++) {
			lemmings[i].removed = 1;
		}
	}
}

u8 lemmings_left(u8 total_lems) {
	if (nuking) {
		return 0;
	}
	if (next_lemming_id >= MAX_NUM_OF_LEMMINGS || next_lemming_id >= total_lems) {
		return 0;
	}
	u8 left = MAX_NUM_OF_LEMMINGS - next_lemming_id;
	u8 left2 = total_lems - next_lemming_id;
	return (left<left2?left:left2);
}

void add_lemming(struct Lemming lemmings[MAX_NUM_OF_LEMMINGS], struct Entrances* entrances, u8 release_rate, u8 total_lems) {
	if (!lemmings || !entrances || nuking) {
		return;
	}
	if (next_lemming_id >= MAX_NUM_OF_LEMMINGS || next_lemming_id >= total_lems) {
		return;
	}
	next_lemming_countdown--;
	if (next_lemming_countdown != 0) {
		return;
	}
	lemmings[next_lemming_id].removed = 0;
	lemmings[next_lemming_id].timer = 0;
	lemmings[next_lemming_id].x = entrances->pos[next_lemming_id & 0x03].x;
	lemmings[next_lemming_id].y = entrances->pos[next_lemming_id & 0x03].y;
	lemmings[next_lemming_id].look_right = 1;
	lemmings[next_lemming_id].abilities = 0;
	lemmings[next_lemming_id].float_index = 0;
	lemmings[next_lemming_id].bricks_left = 0;
	lemmings[next_lemming_id].blocking = 0;
	lemmings[next_lemming_id].start_digging = 0;
	lemmings[next_lemming_id].object_below = 0;
	lemmings[next_lemming_id].object_in_front = 0;
	memset(lemmings[next_lemming_id].saved_object_map,0,9);
	set_lemaction(lemmings+next_lemming_id, LEMACTION_FALL);
	next_lemming_id++;

	s16 n = 99 - (s16)release_rate;
	if (n<0) {
		n += 256;
	}
	next_lemming_countdown = n/2+4;
}

void nuke(struct Level* level) {
	if (!nuking) {
		if (is_custom_sound(0x05)) {
			// play "Oh no!" sound only once
			play_sound(0x05);
		}
		nuking = 1;
		timer_assign = 1;
		if (level && ENABLE_NUKE_GLITCH) {
			level->info.lemmings = next_lemming_id;
		}
	}
}

void update_lemmings(struct Lemming lemmings[MAX_NUM_OF_LEMMINGS], struct Level* level, struct Image* masks[23]) {
	int i;
	if (!lemmings) {
		return; // error
	}
	for (i=0;i<80;i++) {
		if (lemmings[i].removed || lemmings[i].current_action >= 18) {
			continue;
		}
		if (lemmings[i].timer) {
			// update timer
			lemmings[i].timer--;

			if (!lemmings[i].timer) {
				switch (lemmings[i].current_action) {
					case LEMACTION_FRY:
					case LEMACTION_DROWN:
					case LEMACTION_FLOAT:
					case LEMACTION_FALL:
						set_lemaction(lemmings+i,LEMACTION_EXPLODE);
						// play sound: explosion
						play_sound(0x0C);
						break;
					default:
						set_lemaction(lemmings+i,LEMACTION_OHNO);
						if (!nuking || !is_custom_sound(0x05)) {
							// play sound: Oh no!
							play_sound(0x05);
						}
						break;
				}
				continue;
			}
		}
		if ((*lemming_do_action[lemmings[i].current_action])(lemmings+i,level,masks)) {
			process_interactive_objects(lemmings+i,level);
		}
	}

	if (nuking && timer_assign) {
		while(timer_assign<=MAX_NUM_OF_LEMMINGS && lemmings[timer_assign-1].removed) {
			timer_assign++;
		}
		if (timer_assign <= MAX_NUM_OF_LEMMINGS) {
			if (!lemmings[timer_assign-1].timer && lemmings[timer_assign-1].current_action != LEMACTION_SPLATTER && lemmings[timer_assign-1].current_action != LEMACTION_EXPLODE) {
				lemmings[timer_assign-1].timer = 79; // start value
			}
		}
		timer_assign++;
	}
	return;
}


// DoCurrentAction functions
int lemming_walk(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset = (lem->frame_offset + 1)%8;
	lem->x+=(lem->look_right?1:-1);

	if (lem->x < 0){
		lem->look_right = 1;
		return 1;
	}

	if (has_pixel_at(level,lem->x,lem->y)) {
		// walk, jump, climb, or turn
		int i;
		for (i=1;i<8;i++) {
			if (!has_pixel_at(level,lem->x,lem->y-i)) {
				break;
			}
		}
		// collision with obstacle
		if (i==8) {
			if (lem->abilities & LEMABILITY_CLIMB) {
				// start climbing
				set_lemaction(lem, LEMACTION_CLIMB);
			}else{
				// turn around
				lem->look_right = !lem->look_right;
			}
			return 1;
		}
		if (i>3) {
			// jump
			set_lemaction(lem, LEMACTION_JUMP);
			lem->y -= 2;
		}else{
			// just walk
			lem->y -= i-1;
		}

		// test for collision with top of level
		check_top_collision(lem);
		return 1;
	}else{
		// walk or fall
		int i;
		for (i=1;i<4;i++) {
			if (has_pixel_at(level,lem->x,lem->y+i)) {
				break;
			}
		}
		lem->y += i;
		if (i==4) {
			set_lemaction(lem, LEMACTION_FALL);
		}
		if (lem->y > LEM_MAX_Y) {
			lem->removed = 1;
			return 0;
		}
		return 1;
	}
}

int lemming_splatter(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!lem) {
		return 0;
	}
	lem->frame_offset++;
	if (lem->frame_offset == 16) {
		lem->removed = 1;
	}
	return 0;
}

int lemming_fall(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset = (lem->frame_offset + 1)%4;
	if (lem->fall_distance > 16 && (lem->abilities & LEMABILITY_FLOAT)) {
		set_lemaction(lem, LEMACTION_FLOAT);
		return 1;
	}
	// fall down!
	int i;
	for (i=0;i<3;i++) {
		if (lem->y+i > LEM_MAX_Y) {
			lem->removed = 1;
			return 0;
		}
		if (has_pixel_at(level,lem->x,lem->y+i)) {
			break;
		}
	}
	lem->y += i;
	if (i==3) {
		lem->fall_distance += i;
		return 1;
	}else{
		// landed
		if (lem->fall_distance > LEM_MAX_FALLING) {
			set_lemaction(lem, LEMACTION_SPLATTER);
			return 1;
		}
		set_lemaction(lem, LEMACTION_WALK);
		return 1;
	}
}

int lemming_jump(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	int i;
	for (i=0;i<2;i++) {
		if (!has_pixel_at(level,lem->x,lem->y+i-1)) { // really -1?
			break;
		}
	}
	lem->y -= i;
	if (i<2) {
		set_lemaction(lem, LEMACTION_WALK);
	}
	check_top_collision(lem);
	return 1;
}


int lemming_climb(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset = (lem->frame_offset + 1)%8;
	if (lem->frame_offset <= 3) {
		// check for top
		if (!has_pixel_at(level,lem->x, lem->y - 7 - lem->frame_offset)) { // WTF?
			lem->y = lem->y - lem->frame_offset + 2;
			set_lemaction(lem, LEMACTION_HOIST);
			check_top_collision(lem);
		}
	}else{
		lem->y--;
		if (lem->y + lem->y_draw_offset < LEM_MIN_Y ||
				has_pixel_at(level, lem->x + (lem->look_right?-1:1), lem->y-8)) {
			set_lemaction(lem, LEMACTION_FALL);
			lem->look_right = !lem->look_right;
			lem->x += (lem->look_right?2:-2);
		}
	}
	return 1;
}


int lemming_hoist(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset++;
	if (lem->frame_offset <= 4) {
		lem->y -= 2;
		check_top_collision(lem);
		return 1;
	}
	if (lem->frame_offset == 8) {
		set_lemaction(lem, LEMACTION_WALK);
		check_top_collision(lem);
		return 1;
	}
	return 0;
}


int lemming_float(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset = float_param[lem->float_index].frame_offset;
	s8 dy = float_param[lem->float_index].dy;
	lem->float_index++;
	if (lem->float_index == 16) {
		lem->float_index = 8;
	}
	s8 i;
	for (i=0;i<dy;i++) {
		if (has_pixel_at(level,lem->x, lem->y+i)) {
			lem->y += i;
			set_lemaction(lem, LEMACTION_WALK);
			return 1;
		}
	}
	lem->y += dy;
	if (lem->y > LEM_MAX_Y) {
		lem->removed = 1;
		return 0;
	}
	return 1;
}

int lemming_build(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset = (lem->frame_offset + 1)%16;
	if (lem->frame_offset == 10 && lem->bricks_left <= 3) {
		// play sound: builder warning
		play_sound(0x12);
	}
	if (lem->frame_offset == 9 ||
			(lem->frame_offset == 10 && lem->bricks_left == 9)) {
		// WHY is there this second condition?

		// lay brick
		s16 x = lem->x+(lem->look_right?0:-4);
		s16 i;
		for (i=0;i<6;i++) {
			if (x+i < 0 || x+i >= 1584 || lem->y-1 < 0 || lem->y-1 >= 160) {
				continue;
			}
			if (!(level->terrain[x+i+(lem->y-1)*1584] & 0xF0)) {
				level->terrain[x+i+(lem->y-1)*1584] = 0xF7;
			}
		}
		return 0;
	}
	if (lem->frame_offset == 0) {
		lem->x += (lem->look_right?1:-1);
		lem->y--;
		// TODO: if ((lemming.x <= LEMMING_MIN_X AND lemming.x > LEMMING_MAX_X)) - correct??
		if (lem->x <= 0 || has_pixel_at(level,lem->x,lem->y-1)) {
			lem->look_right = !lem->look_right;
			set_lemaction(lem, LEMACTION_WALK);
			check_top_collision(lem);
			return 1;
		}
		lem->x += (lem->look_right?1:-1);
		if (has_pixel_at(level,lem->x,lem->y-1)) {
			lem->look_right = !lem->look_right;
			set_lemaction(lem, LEMACTION_WALK);
			check_top_collision(lem);
			return 1;
		}
		lem->bricks_left--;
		if (!lem->bricks_left) {
			set_lemaction(lem, LEMACTION_SHRUG);
			check_top_collision(lem);
			return 1;
		}
		// TODO: (lemming.x <= LEMMING_MIN_X AND lemming.x > LEMMING_MAX_X)) - correct?
		if (has_pixel_at(level,lem->x+(lem->look_right?2:-2),lem->y-9) ||
				lem->x <= 0) {
			lem->look_right = !lem->look_right;
			set_lemaction(lem, LEMACTION_WALK);
			check_top_collision(lem);
			return 1;
		}
		if (lem->y + lem->y_draw_offset < LEM_MIN_Y) {
			set_lemaction(lem, LEMACTION_WALK);
			check_top_collision(lem);
		}
	}
	return 1;
}

int lemming_shrug(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset++;
	if (lem->frame_offset == 8) {
		set_lemaction(lem, LEMACTION_WALK);
		return 1;
	}
	return 0;
}

int lemming_exit(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset++;
	if (lem->frame_offset == 8) {
		level->rescued++;
		lem->removed = 1;
	}
	return 0;
}

int lemming_drown(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset++;
	if (lem->frame_offset == 16) {
		lem->removed = 1;
	}else if (!has_pixel_at(level,lem->x+(lem->look_right?8:-8), lem->y)) {
		lem->x += (lem->look_right?1:-1);
	}
	return 0;
}

int lemming_fry(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset++;
	if (lem->frame_offset == 14) {
		lem->removed = 1;
	}
	return 0;
}

int lemming_block(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset = (lem->frame_offset + 1)%16;
	if (!has_pixel_at(level,lem->x,lem->y)) {
		// lem->look_right = 1; // TODO: walking direction??
		set_lemaction(lem,LEMACTION_WALK);
		blocker_restore_object_map(lem,level);
	}
	return 0;
}

int lemming_explode(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset++;
	if (lem->frame_offset == 1) {
		if (lem->blocking) {
			blocker_restore_object_map(lem,level);
		}
		u8 obj = read_object_map(level,lem->x,lem->y);
		if (obj != OBJECT_STEEL && obj != OBJECT_WATER) {
			// apply explosion mask (if possible)
			if (masks) {
				apply_mask(masks[12], level, lem->x-8, lem->y-14);
			}
		}
	}
	if (lem->frame_offset == 52) {
		lem->removed = 1;
	}
	return 0;
}

int lemming_ohno(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset++;
	if (lem->frame_offset == 16) {
		set_lemaction(lem,LEMACTION_EXPLODE);
		// play sound: explosion
		play_sound(0x0C);
		return 0;
	}
	int i;
	for (i=0;i<3;i++) {
		if (has_pixel_at(level,lem->x,lem->y+i)) {
			break;
		}
	}
	lem->y += i;
	if (lem->y > LEM_MAX_Y) {
		lem->removed = 1;
		return 0;
	}
	return 1;
}

static inline int dig_row(struct Lemming* lem, struct Level* level, s16 y) {
    int ret = 0;
    s16 x;
    for (x=lem->x-4;x<lem->x+5;x++) {
        if (has_pixel_at(level, x, y)) {
            level->terrain[x+1584*y] = 0x00;
            ret = 1;
        }
    }
    return ret;
}

int lemming_dig(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
    if (lem->start_digging) {
        dig_row(lem,level,lem->y-2);
        dig_row(lem,level,lem->y-1);
        lem->start_digging = 0;
    }else{
        lem->frame_offset = (lem->frame_offset+1)%16;
    }
    if (!(lem->frame_offset & 0x07)) {
        lem->y++;
        if (lem->y > LEM_MAX_Y) {
            lem->removed = 1;
            return 0;
        }
        if (!dig_row(lem,level,lem->y-1)) {
            set_lemaction(lem,LEMACTION_FALL);
            return 1;
        }
        if (read_object_map(level,lem->x, lem->y) == OBJECT_STEEL) {
            // play sound effect: hitting steel
            play_sound(0x0A);
            set_lemaction(lem,LEMACTION_WALK);
        }
        return 1;
    }
    return 0;
}

int lemming_mine(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset = (lem->frame_offset+1)%24;
	if (lem->frame_offset == 1) {
		if (masks) {
			apply_mask(masks[lem->look_right?8:10], level, lem->x+lem->x_draw_offset, lem->y+lem->y_draw_offset);
		}
		return 0;
	}
	if (lem->frame_offset == 2) {
		if (masks) {
			apply_mask(masks[lem->look_right?9:11], level, lem->x+(lem->look_right?1:-1)+lem->x_draw_offset, lem->y+1+lem->y_draw_offset);
		}
		return 0;
	}
	if (lem->frame_offset == 3 || lem->frame_offset == 15) {
		int i;
		for (i=0;i<2;i++) {
			lem->x += (lem->look_right?1:-1);
			if (lem->x < 0 || lem->x >= 1584) {
				lem->look_right = !lem->look_right;
				set_lemaction(lem,LEMACTION_WALK);
				return 1;
			}
		}
		if (lem->frame_offset == 3) {
			lem->y++;
			if (lem->y > LEM_MAX_Y) {
				lem->removed = 1;
				return 0;
			}
		}
		if (!has_pixel_at(level,lem->x,lem->y)) {
			set_lemaction(lem,LEMACTION_FALL);
			return 1;
		}
		u8 below = read_object_map(level,lem->x,lem->y);
		if (below == OBJECT_STEEL ||
				(below == OBJECT_ONEWAY_LEFT && lem->look_right) ||
				(below == OBJECT_ONEWAY_RIGHT && (ENABLE_MINING_ONEWAY_BUG || !lem->look_right))) {
			if (below == OBJECT_STEEL) {
				// play sound: hit steel
				play_sound(0x0A);
			}
			lem->look_right = !lem->look_right;
			set_lemaction(lem,LEMACTION_WALK);
		}
		return 1;
	}
	if (lem->frame_offset == 0) {
		lem->y++;
		if (lem->y > LEM_MAX_Y) {
			lem->removed = 1;
			return 0;
		}
		return 1;
	}
	return 0;
}

int lemming_bash(struct Lemming* lem, struct Level* level, struct Image* masks[23]) {
	if (!level || !lem) {
		return 0;
	}
	lem->frame_offset = (lem->frame_offset+1)%32;
	if ((lem->frame_offset & 0xF) > 10) {
		lem->x += (lem->look_right?1:-1);
		if (lem->x < 0 || lem->x > 1584) {
			lem->look_right = !lem->look_right;
			set_lemaction(lem,LEMACTION_WALK);
			return 1;
		}
		s16 i;
		for (i=0;i<3;i++) {
			if (has_pixel_at(level,lem->x,lem->y+i)) {
				break;
			}
		}
		lem->y += i;
		if (i==3) {
			set_lemaction(lem,LEMACTION_FALL);
			return 1;
		}
		u8 in_front = read_object_map(level, lem->x+(lem->look_right?8:-8), lem->y-8);
		if (in_front == OBJECT_STEEL ||
				(in_front == OBJECT_ONEWAY_LEFT && lem->look_right) ||
				(in_front == OBJECT_ONEWAY_RIGHT && !lem->look_right)) {
			if (in_front == OBJECT_STEEL) {
				// play sound: hit steel
				play_sound(0x0A);
			}
			lem->look_right = !lem->look_right;
			set_lemaction(lem,LEMACTION_WALK);
			return 1;
		}
		return 1;
	}
	if ((lem->frame_offset & 0xF) > 1 && (lem->frame_offset & 0xF) < 6) {
		if (masks) {
			apply_mask(masks[(lem->frame_offset & 0xF)-2 + (lem->look_right?0:4)],level,lem->x+lem->x_draw_offset,lem->y+lem->y_draw_offset);
		}
		if (lem->frame_offset == 5) {
			s16 i;
			for (i=0;i<4;i++) {
				if (has_pixel_at(level,lem->x+(lem->look_right?8+i:-8-i),lem->y-6)) {
					break;
				}
			}
			if (i==4) {
				set_lemaction(lem,LEMACTION_WALK);
			}
		}
	}
	return 0;
}

int foo(struct Lemming* lem, struct Level* level, struct Image* masks[]) {
	return 0;
}

void set_lemaction(struct Lemming* lem, u8 action) {
	lem->current_action = action;
	lem->draw_action = action;
	lem->frame_offset = 0;
	lem->x_draw_offset = action_draw_offsets[action].x;
	lem->y_draw_offset = action_draw_offsets[action].y;
	(*lemming_switch_action[action])(lem);
}

void switch_action_dummy(struct Lemming* lem) {
}

void lemming_start_splatter(struct Lemming* lem) {
	lem->timer = 0;
	// play sound: splatter
	play_sound(0x08);
}
void lemming_start_fall(struct Lemming* lem) {
	lem->fall_distance = 3;
}
void lemming_start_float(struct Lemming* lem) {
	lem->float_index = 0;
}
void lemming_start_build(struct Lemming* lem) {
	lem->bricks_left = 12;
}
void lemming_start_block(struct Lemming* lem) {
	lem->blocking = 1;
}

void lemming_start_mine(struct Lemming* lem) {
	lem->y++;
}

void lemming_start_dig(struct Lemming* lem) {
	lem->start_digging = 1;
}

void process_interactive_objects(struct Lemming* lem, struct Level* level) {
	if (!lem || !level) {
		return;
	}
	lem->object_below = read_object_map(level,lem->x,lem->y);
	lem->object_in_front = read_object_map(level,lem->x+(lem->look_right?8:-8),lem->y-8);
	switch (lem->object_below & 0x0F) {
		case OBJECT_EXIT:
			if (lem->current_action != LEMACTION_FALL) {
				set_lemaction(lem, LEMACTION_EXIT);
				// play sound: yippieh
				play_sound(0x10);
			}
			break;
		case OBJECT_FORCE_LEFT:
			lem->look_right = 0;
			break;
		case OBJECT_FORCE_RIGHT:
			lem->look_right = 1;
			break;
		case OBJECT_TRAP:
		{
			struct ObjectInstance* trap = level->obj + (lem->object_below >> 4);
			if (trap->current_frame == 0) {
				trap->current_frame++;
				lem->removed = 1;
				// play sound effect depending on trap->type
				play_sound(level->o[trap->type]->sound);
			}
			break;
		}
		case OBJECT_WATER:
			if (lem->current_action != LEMACTION_DROWN) {
				set_lemaction(lem, LEMACTION_DROWN);
				// play sound: drowning
				play_sound(0x11);
			}
			break;
		case OBJECT_FIRE:
			if (lem->current_action != LEMACTION_FRY) {
				set_lemaction(lem, LEMACTION_FRY);
				// play sound: fried
				play_sound(0x0D);
			}
			break;
	}
}

u8 select_lemming(struct Lemming lemmings[MAX_NUM_OF_LEMMINGS], s16 mouse_x, s16 mouse_y, u8 right_mouse_btn, s16* lem1_idx, s16* lem2_idx) {
	if (lem1_idx) {
		*lem1_idx = -1;
	}
	if (lem2_idx) {
		*lem2_idx = -1;
	}
	if (!lemmings) {
		return 0;
	}
	s16 prio = -1;
	s16 norm = -1;
	s16 i;
	u8 cnt = 0;
	for (i=0;i<80;i++) {
		if (lemmings[i].removed || lemmings[i].current_action >= 18) {
			continue;
		}
		if (lemmings[i].current_action == LEMACTION_EXPLODE) {
			continue;
		}
		s16 x = lemmings[i].x + lemmings[i].x_draw_offset + 4;
		s16 y = lemmings[i].y + lemmings[i].y_draw_offset - 1;
		if (x<=mouse_x && x+12>=mouse_x && y<=mouse_y && y+12>=mouse_y) {
			cnt++;
			switch (lemmings[i].current_action) {
				case LEMACTION_BLOCK:
				case LEMACTION_BUILD:
				case LEMACTION_SHRUG:
				case LEMACTION_BASH:
				case LEMACTION_MINE:
				case LEMACTION_DIG:
				case LEMACTION_OHNO:
					prio = i;
					break;
				default:
					norm = i;
					break;
			}
		}
	}
	if (lem1_idx) {
		if (prio == -1 || right_mouse_btn) {
			*lem1_idx = norm;
		}else{
			*lem1_idx = prio;
		}
	}
	if (lem2_idx) {
		*lem2_idx = norm;
	}
	return cnt;
}

const char* get_lemming_description(struct Lemming* lem) {
	if (!lem) {
		return 0;
	}
	if (lem->current_action == LEMACTION_SHRUG) {
		return "BUILDER";
	}
	if ((lem->abilities & LEMABILITY_CLIMB) && (lem->abilities & LEMABILITY_FLOAT)) {
		return "ATHLETE";
	}else if (lem->abilities & LEMABILITY_CLIMB) {
		return "CLIMBER";
	}else if (lem->abilities & LEMABILITY_FLOAT) {
		return "FLOATER";
	}
	switch (lem->current_action) {
		case LEMACTION_FALL:
			return "FALLER";
		case LEMACTION_BUILD:
			return "BUILDER";
		case LEMACTION_DIG:
			return "DIGGER";
		case LEMACTION_BLOCK:
			return "BLOCKER";
		case LEMACTION_BASH:
			return "BASHER";
		case LEMACTION_MINE:
			return "MINER";
		default:
			return "WALKER";
	}
}

// return 0 on failure, 1 on success
// skills: CLIMBING=0, FLOATING=1, etc. (ordering like at the skill panel)
u8 assign_skill(u8 skill, struct Lemming* lem1, struct Lemming* lem2, struct Level* level) {
	if (skill >= 8 || !level) {
		return 0;
	}
	if (lem1) {
		if (lem1->removed) {
			return 0;
		}
		if (lem2) {
			if (lem2->removed) {
				lem2 = 0;
			}
		}
		u8 success = (*lemming_assign[skill])(lem1,lem2,level);
		if (success) {
			// play assignment sound
			play_sound(0x04);
		}
		return success;
	}
	return 0;
}

int foo3(struct Lemming* lem1, struct Lemming* lem2, struct Level* level) {
	return 0;
}

int assign_climb(struct Lemming* lem1, struct Lemming* lem2, struct Level* level) {

	if (!(lem1->abilities & LEMABILITY_CLIMB) &&
			lem1->current_action != LEMACTION_BLOCK &&
			lem1->current_action != LEMACTION_SPLATTER &&
			lem1->current_action != LEMACTION_EXPLODE) {
		lem1->abilities |= LEMABILITY_CLIMB;
		if (lem1->current_action == LEMACTION_SHRUG) {
			if (ENABLE_SHRUGGER_GLITCH) {
				lem1->current_action = LEMACTION_WALK; // this triggers a bug of the original game since draw_action is not updated
			}else{
				set_lemaction(lem1,LEMACTION_WALK);
			}
		}
		return 1;
	}

	return 0;
}

int assign_float(struct Lemming* lem1, struct Lemming* lem2, struct Level* level) {
	if (!(lem1->abilities & LEMABILITY_FLOAT) &&
			lem1->current_action != LEMACTION_BLOCK &&
			lem1->current_action != LEMACTION_SPLATTER &&
			lem1->current_action != LEMACTION_EXPLODE) {
		lem1->abilities |= LEMABILITY_FLOAT;
		return 1;
	}
	return 0;
}

int assign_block(struct Lemming* lem1, struct Lemming* lem2, struct Level* level) {
	int assign = 0;
	switch (lem1->current_action) {
		case LEMACTION_WALK:
		case LEMACTION_SHRUG:
		case LEMACTION_BUILD:
		case LEMACTION_BASH:
		case LEMACTION_MINE:
		case LEMACTION_DIG:
			assign = 1;
	}
	if (assign && blocker_present(lem1,level)) {
		assign = 0;
	}
	if (assign) {
		set_lemaction(lem1, LEMACTION_BLOCK);
		blocker_modify_object_map(lem1,level);
		return 1;
	}
	return 0;
}

int assign_bomb(struct Lemming* lem1, struct Lemming* lem2, struct Level* level) {
	if (lem1->timer) {
		return 0;
	}
	switch (lem1->current_action) {
		case LEMACTION_OHNO:
		case LEMACTION_EXPLODE:
		case LEMACTION_FRY:
		case LEMACTION_SPLATTER:
			return 0;
		default:
			lem1->timer = 79;
			return 1;
	}
}

int assign_build(struct Lemming* lem1, struct Lemming* lem2, struct Level* level) {
	if (lem1->y + lem1->y_draw_offset < LEM_MIN_Y) {
		return 0;
	}
	int assign = 0;
	switch (lem1->current_action) {
		case LEMACTION_WALK:
		case LEMACTION_SHRUG:
		case LEMACTION_BASH:
		case LEMACTION_MINE:
		case LEMACTION_DIG:
			assign = 1;
	}
	if (assign) {
		set_lemaction(lem1, LEMACTION_BUILD);
		return 1;
	}
	if (!lem2) {
		return 0;
	}
	switch (lem2->current_action) {
		case LEMACTION_WALK:
		case LEMACTION_SHRUG:
		case LEMACTION_BASH:
		case LEMACTION_MINE:
		case LEMACTION_DIG:
			assign = 1;
	}
	if (assign) {
		set_lemaction(lem2, LEMACTION_BUILD);
		return 1;
	}
	return 0;
}

int assign_mine(struct Lemming* lem1, struct Lemming* lem2, struct Level* level) {
	struct Lemming* lem = 0;
	switch(lem1->current_action) {
		case LEMACTION_WALK:
		case LEMACTION_SHRUG:
		case LEMACTION_BUILD:
		case LEMACTION_BASH:
		case LEMACTION_DIG:
			lem = lem1;
			break;
		default:
			break;
	}
	if (!lem && lem2) {
		switch(lem2->current_action) {
			case LEMACTION_WALK:
			case LEMACTION_SHRUG:
			case LEMACTION_BUILD:
			case LEMACTION_BASH:
			case LEMACTION_DIG:
				lem = lem2;
				break;
			default:
				break;
		}
	}
	if (!lem) {
		return 0;
	}
	if (lem->object_in_front == OBJECT_STEEL) {
		// play sound effect: hit steel
		play_sound(0x0A);
		return 0;
	}
	if (lem->object_below == OBJECT_STEEL ||
			(lem->object_below == OBJECT_ONEWAY_LEFT && lem->look_right) ||
			(lem->object_below == OBJECT_ONEWAY_RIGHT && !lem->look_right)) {
		return 0;
	}
	set_lemaction(lem,LEMACTION_MINE);
	return 1;
}

int assign_dig(struct Lemming* lem1, struct Lemming* lem2, struct Level* level) {
    if (lem1->object_below == OBJECT_STEEL) {
        return 0;
    }
    switch (lem1->current_action) {
        case LEMACTION_WALK:
        case LEMACTION_SHRUG:
        case LEMACTION_BUILD:
        case LEMACTION_BASH:
        case LEMACTION_MINE:
            set_lemaction(lem1, LEMACTION_DIG);
            return 1;
        default:
            break;
    }
    if (!lem2) {
        return 0;
    }
    switch (lem2->current_action) {
        case LEMACTION_WALK:
        case LEMACTION_SHRUG:
        case LEMACTION_BUILD:
        case LEMACTION_BASH:
        case LEMACTION_MINE:
            set_lemaction(lem2, LEMACTION_DIG);
            return 1;
        default:
            return 0;
    }
}


int assign_bash(struct Lemming* lem1, struct Lemming* lem2, struct Level* level) {
	struct Lemming* lem = 0;
	switch(lem1->current_action) {
		case LEMACTION_WALK:
		case LEMACTION_SHRUG:
		case LEMACTION_BUILD:
		case LEMACTION_MINE:
		case LEMACTION_DIG:
			lem = lem1;
			break;
		default:
			break;
	}
	if (!lem && lem2) {
		switch(lem2->current_action) {
			case LEMACTION_WALK:
			case LEMACTION_SHRUG:
			case LEMACTION_BUILD:
			case LEMACTION_MINE:
			case LEMACTION_DIG:
				lem = lem2;
				break;
			default:
				break;
		}
	}
	if (!lem) {
		return 0;
	}
	if (lem->object_in_front == OBJECT_STEEL ||
			(lem->object_in_front == OBJECT_ONEWAY_LEFT && lem->look_right) ||
			(lem->object_in_front == OBJECT_ONEWAY_RIGHT && !lem->look_right)) {
		if (lem->object_in_front == OBJECT_STEEL) {
			// play sound effect: hit steel
			play_sound(0x0A);
		}
		return 0;
	}
	set_lemaction(lem,LEMACTION_BASH);
	return 1;
}

