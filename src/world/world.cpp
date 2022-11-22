#include "../world.h"

world::world() {
    name = "Default";
}

chunk* world::get_chunks() {return &chunks[0];}
