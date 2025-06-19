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

#define NUM_SAVE_FILES 64
#define MAX_FILE_NAME_LEN 256
// Load a map from a save file, return true on success.
bool loadWorld(int saveIndex) {
    // Fail if index out of range:
    if (saveIndex < 0 || saveIndex > NUM_SAVE_FILES)
        return false;
    
    // Open the chosen save file:
    char fName[MAX_FILE_NAME_LEN] = {0};
    snprintf(fName, MAX_FILE_NAME_LEN, "assets/saves/save%02d.txt", saveIndex);
    dlog(WORLD, "Opening save file '%s'...\n", fName);
    FILE* f = fopen(fName, "r");
    if (!f) {
        printf("*** Couldn't open save file '%s'\n", fName);
        perror("");
        return false;
    }
    
    /* Display file contents:
    char printString[1024] = {0};
    fgets(printString, 1024, f);
    printf("Save contents: %s\n", printString);
    */
    
    // Set the world state from the save file:
    size_t bytesRead = fread(mainWorld, 1, sizeof(struct world), f);
    if (bytesRead < sizeof(struct world)) {
        printf("*** Failed to finish reading save file '%s', only read %ld of %ld bytes.\n", fName, bytesRead, sizeof(struct world));
        perror("");
        return false;
    }
    
    fclose(f);
    return true;
}
// Save the world state to a file, return true on success.
bool saveWorld(int saveIndex) {
    // Fail if index out of range:
    if (saveIndex < 0 || saveIndex > NUM_SAVE_FILES)
        return false;
    
    // Open the chosen save file:
    char fName[MAX_FILE_NAME_LEN] = {0};
    snprintf(fName, MAX_FILE_NAME_LEN, "assets/saves/save%02d.txt", saveIndex);
    dlog(WORLD, "Writing to save file '%s'...\n", fName);
    FILE* f = fopen(fName, "w");
    if (!f) {
        printf("*** Couldn't write to save file '%s'\n", fName);
        perror("");
        return false;
    }
    
    // Copy the state of mainWorld to the selected save file:
    size_t bytesWritten = fwrite(mainWorld, 1, sizeof(struct world), f);
    if (bytesWritten < sizeof(struct world)) {
        printf("*** Failed to finish writing save file '%s', only wrote %ld of %ld bytes.\n", fName, bytesWritten, sizeof(struct world));
        perror("");
        return false;
    }
    
    fclose(f);
    return true;
}
