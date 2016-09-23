#ifndef IMPORT_LEVEL_H
#define IMPORT_LEVEL_H
#include <3ds.h>
#include "level.h"

int read_level_names(u8 game, char* names);
int read_level(u8 game, u8 id, struct Level* level);
void free_objects(struct Object* objects[16]);

void init_level_state(struct LevelState* state, struct Level* level);
#endif
