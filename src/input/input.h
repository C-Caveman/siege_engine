#ifndef INPUT
#define INPUT

#include "../ent/ent.h"
#include "../client/client.h"
#include <SDL2/SDL.h> 
#include <iostream>

extern int mouse_x;
extern int mouse_y;

void client_input(client_data* client);


// TODO replace old ent system TODO
extern struct client_data player_client;

#endif
