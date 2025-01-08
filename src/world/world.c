#include "../defs.h"

struct world* mainWorld = 0;

void initMainWorld() {
    memset((void*)mainWorld, 0, sizeof(struct world));
    strncpy(mainWorld->name, "Default World", MAX_WORLD_NAME_LEN);
    mainWorld->entArraySpace = ENTITY_BYTES_ARRAY_LEN;
}

struct tile* worldGetTile(vec2i tile_i) {
    if (!v2iInBounds(tile_i, 0, WORLD_WIDTH*CHUNK_WIDTH-1))
        return 0;
    vec2i chunk_i = v2iScalarDiv(tile_i, CHUNK_WIDTH);
    vec2i local_i = v2iModulo(tile_i, CHUNK_WIDTH);
    return &mainWorld->chunks[chunk_i.y][chunk_i.x].tiles[local_i.y][local_i.x];
}

struct tile* worldTileFromPos(vec2f pos) {
    vec2i tile_i = v2fToI(v2fScalarDiv(pos, RSIZE));
    if (!v2iInBounds(tile_i, 0, WORLD_WIDTH*CHUNK_WIDTH-1))
        return 0;
    vec2i chunk_i = v2iScalarDiv(tile_i, CHUNK_WIDTH);
    vec2i local_i = v2iModulo(tile_i, CHUNK_WIDTH);
    return &mainWorld->chunks[chunk_i.y][chunk_i.x].tiles[local_i.y][local_i.x];
}

vec2i worldTileIndexFromPos(vec2f pos) {
    return v2fToI(v2fScalarDiv(pos, RSIZE));
}

vec2i tileNumberToIndex(uint32_t tileNumber) {
    vec2i index = { tileNumber % (CHUNK_WIDTH*WORLD_WIDTH), tileNumber / (CHUNK_WIDTH*WORLD_WIDTH) };
    return index;
}
uint32_t tileIndexToNumber(vec2i tileIndex) {
    uint32_t tileNumber = (uint32_t) (tileIndex.x + tileIndex.y*CHUNK_WIDTH*WORLD_WIDTH);
    return tileNumber;
}
