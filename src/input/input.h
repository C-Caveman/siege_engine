#ifndef INPUT
#define INPUT

#include "../ent/ent.h"
#include "../client/client.h"
#include "../config/vars.h"
#include <SDL2/SDL.h> 

extern int mouse_x;
extern int mouse_y;
    
#define MAX_KEYS_PER_BIND 4
struct inputKeybindings {
    int keys[MAX_KEYS_PER_BIND];
};
extern struct inputKeybindings inputs[NUM_INPUTS];
void setBinding(int inputIndex, int keyCode);
int  getBinding(int inputIndex);

void client_input(struct client* client);


// TODO replace old ent system TODO
extern struct client playerClient;

#endif
