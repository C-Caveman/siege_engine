// store all player-related information
#ifndef CLIENT
#define CLIENT

#include <stdio.h>
#include "ent.h"

struct client { // TODO move client stuff here
private:
    int id;
    struct ent* client_ent;
};

// client-side functions
void client_init(int id, ent* client_ent); //TODO make this a constructor?

void p_move(); // TODO this







#endif
