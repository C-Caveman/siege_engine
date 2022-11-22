#ifndef WORLD
#define WORLD

#include "chunk.h"
#include <string>



class world {
private:
    string name;
    chunk chunks[16]; // chunk array (for blocks)
    ent ents[256];    // entity array
public:
    world();
    // TODO make this happen
    chunk* get_chunks();
};

#endif
