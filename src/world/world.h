#ifndef WORLD
#define WORLD

#include "chunk.h"
#include <string>

#define WORLD_WIDTH 16
#define MAX_WORLD_NAME_LEN 64
#define MAX_ENTS 2048 //  temp value
#define MAX_CLIENTS 1 // temp value
#define ENTITY_BYTES_ARRAY_LEN 8192

struct world {
    char name[MAX_WORLD_NAME_LEN];
    chunk chunks[WORLD_WIDTH][WORLD_WIDTH]; //---------- Tiles.
    char entity_bytes_array[ENTITY_BYTES_ARRAY_LEN]; //- Entities.

    world();
    // TODO make this happen
    chunk* get_chunk(int x, int y);
};

extern struct world* main_world;

#endif
