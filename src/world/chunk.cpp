// defines tiles, groups of tiles, ect.
#include "../defs.h" 

void chunk::set_floors(int floor_anim) {
    for (int i=0; i<CHUNK_WIDTH; i++) {
        for (int j=0; j<CHUNK_WIDTH; j++)
            tiles[i][j].floor_anim = floor_anim;
    }
}

void chunk::set_tile(int x, int y, int new_anim, int new_frame) {
    tiles[y][x].wall_top_anim = new_anim;
    //tiles[x*TILE_INTS + y*CHUNK_WIDTH*TILE_INTS + TILE_FRAME] = new_frame;
}

void chunk::set_floor(int x, int y, int floor_anim) { tiles[y][x].floor_anim = floor_anim;}
void chunk::set_wall(int x, int y, int wall_top_anim, int wall_side_anim, unsigned char wall_height) {
    tiles[y][x].wall_top_anim = wall_top_anim;
    tiles[y][x].wall_side_anim = wall_side_anim;
    tiles[y][x].wall_height = wall_height;
}



