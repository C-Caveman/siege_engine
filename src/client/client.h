// store all player-related information
#ifndef CLIENT
#define CLIENT

#include <stdio.h>
#include "../ent/ent.h"

struct client { // TODO move client stuff here
private:
    int id; // needed by the server to differentiate players from each other
    int mouse_x;
    int mouse_y;
    struct ent* client_ent;
public:
    struct ent* get_ent();
    void set_ent(struct ent* e);
};

// client-side functions
void client_init(struct client* c, ent* client_ent); //TODO make this a constructor?

void p_move(); // TODO this







#endif
