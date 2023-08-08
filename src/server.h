// store and handle ALL game state information, including clients 
#ifndef SERVER
#define SERVER

#include "audio.h"
#include "graphics.h"
#include "input.h"
#include "world.h"
#include "config.h"
#include "client.h"
#include "actions.h"

//TODO set up data structures for handling server data
extern int num_entities;
extern struct ent entities[MAX_ENTS];
extern int num_clients;
extern struct client clients[MAX_CLIENTS]; //TODO move server constants

// load server config data
void server_config();

//TODO manage the entities array
struct ent* add_ent(int type); // TODO this
void remove_ent(int id); // TODO this
//TODO manage client array
struct client* add_client();

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
