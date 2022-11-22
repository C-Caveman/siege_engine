// store and handle ALL game state information, including clients 
#ifndef SERVER
#define SERVER

#include "graphics.h"
#include "input.h"
#include "world.h"
#include "config.h"
#include "client.h"
#include "actions.h"

//TODO set up data structures for handling server data
struct ent entities[MAX_ENTS];
struct client clients[MAX_CLIENTS];

// load config data
void server_config();

//TODO manage the entities array
void add_ent(int id); // TODO this
void remove_ent(int id); // TODO this

//TODO initialize a singleplayer server
// start up a singleplayer server
void server_init_singleplayer();
//TODO initialize a multiplayer server

//TODO read server messages
//TODO read client commands
void move_player();

//TODO messages update entities
//TODO physics update entities
void physics_tick(int num_ents);
//TODO send messages to server


#endif
