// how entities do stuff 

#ifndef ACTIONS
#define ACTIONS

#include "../ent/ent.h"
#include "../server/server_constants.h"

// use player's desired direction
// to accelerate them
void player_accel(ent* e);

// ent-ent collision
void push(ent* e);

// move an ent using its velocity
void move(ent* e);



#endif
