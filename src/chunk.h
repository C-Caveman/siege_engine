// A chunk is a 16x16 area of blocks.
#include "ent.h"

// textured tile with various properties

#define B_INTS 4 // block integers
#define CHUNK_WIDTH 16

enum block_data {
    B_ANIM=0,
    B_FRAME, // which frame in the animation should be used
    B_TYPE,
    B_HEALTH,
    B_FLAGS
};

//TODO connect entities to the chunks!
// 16x16 area of blocks
class chunk {
private:
    int blocks[CHUNK_WIDTH * CHUNK_WIDTH * B_INTS];
public:
    int* get_blocks();
    void set_blocks(int tex_id);
    void set_block(int x, int y, int new_anim, int new_frame);
};


//TODO make block types here
