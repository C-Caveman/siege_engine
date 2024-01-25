// A chunk is a 16x16 area of tiles.
// Each tile has a floor and a wall.
#include "../ent/ent.h"

#define MAX_ENTS_PER_TILE 4
#define CHUNK_WIDTH 16

struct tile {
    unsigned char flags, health, wall_height, unused;
    handle ents[MAX_ENTS_PER_TILE];
    int wall_top_anim, wall_side_anim, floor_anim;
};

// 16x16 area of tiles
struct chunk {
    struct tile tiles[CHUNK_WIDTH][CHUNK_WIDTH];
    void set_floors(int wall_top_animation);
    void set_tile(int x, int y, int wall_top_anim, int new_frame);
    void set_floor(int x, int y, int floor_anim);
    void set_wall(int x, int y, int wall_top_anim, int wall_side_anim, unsigned char wall_height);
};


//TODO make tile types here
