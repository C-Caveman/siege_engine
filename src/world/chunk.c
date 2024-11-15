// defines tiles, groups of tiles, ect.
#include "../defs.h" 

void chunkSetFloors(struct chunk* c, int floor_anim) {
    for (int i=0; i<CHUNK_WIDTH; i++) {
        for (int j=0; j<CHUNK_WIDTH; j++)
            c->tiles[i][j].floor_anim = floor_anim;
    }
}

void chunkSetTile(struct chunk* c, int x, int y, int new_anim, int new_frame) {
    c->tiles[y][x].wall_top_anim = new_anim;
    //tiles[x*TILE_INTS + y*CHUNK_WIDTH*TILE_INTS + TILE_FRAME] = new_frame;
}

void chunkSetFloor(struct chunk* c, int x, int y, int floor_anim) {
    c->tiles[y][x].floor_anim = floor_anim;
}
void chunkSetWall(struct chunk* c, int x, int y, int wall_top_anim, int wall_side_anim, unsigned char wall_height) {
    c->tiles[y][x].wall_top_anim = wall_top_anim;
    c->tiles[y][x].wall_side_anim = wall_side_anim;
    c->tiles[y][x].wall_height = wall_height;
}



