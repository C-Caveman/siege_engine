#include "../defs.h"

struct world* main_world = nullptr;

world::world() {
    strncpy(name, "Default World", MAX_WORLD_NAME_LEN);
    memset((void*)chunks, 0, WORLD_WIDTH*WORLD_WIDTH);
    memset((void*)entity_bytes_array, 0, ENTITY_BYTES_ARRAY_LEN);
}

tile* world::get_tile(vec2i tile_i) {
    if (!tile_i.in_bounds(0, WORLD_WIDTH*CHUNK_WIDTH-1))
        return nullptr;
    vec2i chunk_i = tile_i / CHUNK_WIDTH;
    vec2i local_i = tile_i % CHUNK_WIDTH;
    return &chunks[chunk_i.y][chunk_i.x].tiles[local_i.y][local_i.x];
}
