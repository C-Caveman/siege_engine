// defines blocks, groups of blocks, ect.
#include "../chunk.h" 

int* chunk::get_blocks() {return blocks;}

void chunk::set_blocks(int tex_id) {
    for (int i=0; i<CHUNK_WIDTH*CHUNK_WIDTH; i++) {
        blocks[i*B_INTS + B_ANIM] = tex_id;
    }
}

void chunk::set_block(int x, int y, int new_anim, int new_frame) {
    blocks[x*B_INTS + y*CHUNK_WIDTH*B_INTS + B_ANIM] = new_anim;
    blocks[x*B_INTS + y*CHUNK_WIDTH*B_INTS + B_FRAME] = new_frame;
}


