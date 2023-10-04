// store all player-related information

#include "client.h"



//TODO this
void get_input_state() {}

struct ent* client::get_ent() {return client_ent;}
void client::set_ent(struct ent* e) {client_ent = e;}

// TODO this
void client_init(struct client* c, ent* client_entity) {
    //printf("Client initializing...\n");
    //id = my_id;
    c->set_ent(client_entity);
}
