#ifndef WORLD
#define WORLD

#include "chunk.h"
#include <string>

#define WORLD_WIDTH 16
#define MAX_ENTS 2048 //  temp value
#define MAX_CLIENTS 1 // temp value

class world {
private:
    string name;
    chunk chunks[WORLD_WIDTH][WORLD_WIDTH]; // chunk array (for tiles)
    // TODO move the server's entity array here <------------------------------- TODO
public:
    world();
    // TODO make this happen
    chunk* get_chunk(int x, int y);
};

#endif
