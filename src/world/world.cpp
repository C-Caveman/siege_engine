#include "../defs.h"

struct world* main_world = nullptr;

world::world() {
    strncpy(name, "Default World", MAX_WORLD_NAME_LEN);
    memset((void*)chunks, 0, WORLD_WIDTH*WORLD_WIDTH);
    memset((void*)entity_bytes_array, 0, ENTITY_BYTES_ARRAY_LEN);
}

