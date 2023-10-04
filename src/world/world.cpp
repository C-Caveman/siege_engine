#include "world.h"

world::world() {
    name = "Default";
}

chunk* world::get_chunk(int x, int y) {return &chunks[y][x];}
