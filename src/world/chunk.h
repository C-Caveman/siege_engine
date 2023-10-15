// A chunk is a 16x16 area of tiles.
// Each tile has a floor and a wall.
#include "../ent/ent.h"

struct tile {
    unsigned char flags, health, wall_height, unused;
    int wall_top_anim, wall_side_anim, floor_anim;
};

//TODO connect entities to the chunks!

#define CHUNK_WIDTH 16
// 16x16 area of tiles
struct chunk {
    struct tile tiles[CHUNK_WIDTH][CHUNK_WIDTH];
    struct tile (*get_tiles())[CHUNK_WIDTH];
    void set_floors(int wall_top_animation);
    void set_tile(int x, int y, int wall_top_anim, int new_frame);
    void set_floor(int x, int y, int floor_anim);
    void set_wall(int x, int y, int wall_top_anim, int wall_side_anim, unsigned char wall_height);
};


//TODO make tile types here
