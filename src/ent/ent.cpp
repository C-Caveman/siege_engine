// Implementations of entity functions.
#include "ent.h"

extern volatile float mouse_angle; // Direction the mouse is pointed in.
extern volatile int mouse_x;
extern volatile int mouse_y;
extern float dt; // Delta time.

// Used to give each spawned entity a unique id number.
uint16_t unique_entity_id = 0;

/////////////////////////////////////////////////////////////////////////////////////////;;
// Initialize an entity (to the default values for its type).
//
void ent_EMPTY::init() {
    // Our work here is done.
}
void ent_PLAYER::init() {
    if (DEBUG_ENTS)
        printf("Player entity initializing!\n");
    data[head].head.flags = DRAWABLE | ANIMATABLE | MOVABLE | COLLIDABLE | THINKABLE;
    data[head].head.num_sprites = num_player_sprites;
    // Init the sprites:
    data[p_sprite_body_pos].pos.pos = vec2f{0,0};
    data[p_sprite_body_anim].anim.anim = rocket_tank;
    data[p_sprite_gun_pos].pos.pos = vec2f{0,0};
    data[p_sprite_gun_anim].anim.anim = gun_grenade;
    // Sprint by default:
    data[player_movetype].movetype.movetype = MOVE_SPRINT; // TODO connect this with the client object TODO
}
void ent_SCENERY::init() {
    if (DEBUG_ENTS)
        printf("Scenery ent initializing!\n");
    data[head].head.flags = DRAWABLE | ANIMATABLE;
    data[head].head.num_sprites = num_scenery_sprites;
    // Init the sprites:
    data[scenery_sprite_pos].pos.pos = vec2f{0,0};
    data[scenery_sprite_anim].anim.anim = floor_test; // Default sprite.
}

/////////////////////////////////////////////////////////////////////////////////////////;;
// Update an entity for the next frame.
//
void ent_EMPTY::think() {
    // Our work here is done.
}
void ent_PLAYER::think() {
    //data[p_sprite_gun_anim].anim.rotation = mouse_angle;
    /*
    data[player_dir].dir.dir = vec2f{(float)cos(mouse_angle/180*M_PI), (float)sin(mouse_angle/180*M_PI)};
    float friction = data[vel].vel.vel.vlen();
    if (friction < 1.0)
        friction = 0;
    friction = (friction * friction) / 20000;
    data[vel].vel.vel = (data[vel].vel.vel + data[player_dir].dir.dir * 10) - (data[vel].vel.vel.normalized() * friction);
    //vec2f{(float)mouse_x, (float)mouse_y};
    */
}
void ent_SCENERY::think() {}

/////////////////////////////////////////////////////////////////////////////////////////;;
// Entity management functions. (spawning, despawning, getting next ent in array, ect.)
//

// This macro formats the ENTITY_TYPES_LIST into an array of strings.
#undef f
#define f(x) #x, 
char entity_type_names[NUM_ENT_TYPES][MAX_ENTITY_TYPE_NAME_LEN] = { ENTITY_TYPES_LIST };
// Return the name for a given entity type.
char* get_type_name(int type) {
    return entity_type_names[type];
}
// Return the size of the current entity type (in segments).
int get_ent_size(int type) {
    int size = -1;
    switch (type) {
    case EMPTY:
        size=0;
        break;
    case PLAYER:
        size=player_size;
        break;
    case SCENERY:
        size=scenery_size;
        break;
    default:
        printf("*** Unknown entity type in get_ent_size()\n");
        exit(-1);
    }
    return size;
}
// Return index of the next entity's header segment. Returns -1 if there are no more entities.
int get_next_ent(int i, segment* array, int array_len) {
    // Is segment i full?    (assumed to be be an entity header if so)
    if (array[i].head.type != EMPTY) {
        i += array[i].head.size; // Skip past this entity.
        if (DEBUG_ENTS)
            printf("get_next_ent() entity at %d. Size is %d.\n", i, array[i].head.size);
    }
    // Skip forward until reaching a non-empty segment.
    while (i < array_len && array[i].head.type == EMPTY) {
        if (DEBUG_ENTS)
            printf("get_next_ent() skipping past %d.\n", i);
        i += 1;
    }
    if (i >= array_len) // Out of bounds.
        i = -1;
    return i;
}
// Return index of the first entity in a segment array. Return -1 if no entities are found.
int get_first_ent(segment* array, int array_len) {
    int i=0;
    // Search for the first valid header segment with non-empty entity.
    for (i=0; i<array_len; i++) {
        if ((array[i].head.header_byte == HEADER_BYTE) && (array[i].head.type != EMPTY))
            break;
    }
    if (i == (array_len-1))
        i = -1;
    return i;
}
// Make a new entity in the given segment array. Return its index.
segment* spawn_ent(int type, segment* array, int array_len) {
    int index = -1;
    int required_space = get_ent_size(type);
    int empty_space_len = 0;
    int i = 0;
    while (i<array_len) {
        // Empty slot?
        if (array[i].head.type == EMPTY) {
            if (DEBUG_ENT_SPAWNING)
                printf("Found an open slot at %d.\n", i);
            empty_space_len += 1;
            i += 1;
        }
        // Slot occupied.
        else {
            if (DEBUG_ENT_SPAWNING)
                printf("Slots [%d, %d] already taken.\n", i, i+array[i].head.size-1);
            empty_space_len = 0;
            i += array[i].head.size;
        }
        // Got enough space to store the ent.
        if (empty_space_len == required_space) {
            if (DEBUG_ENT_SPAWNING)
                printf("Found enough space for ent in [%d, %d]\n", i-required_space, i-1);
            index = i-required_space;
            break;
        }
    }
    if (index == -1) {
        if (DEBUG_ENT_SPAWNING)
            printf("***\n*** No space left in the entity array!!!\n***\n");
        return nullptr;
    }
    // Initialize the entity's header info:
    array[index].head.header_byte = HEADER_BYTE;
    array[index].head.type = type;
    array[index].head.size = required_space;
    array[index].head.id = unique_entity_id;
    unique_entity_id += 1; // Make sure the next entity id number is different. TODO ensure unique
    // Initialize the entity.
    switch (type) {
        // This macro formats the entity types list into a series of case statements.
        // Each case statement calls the appropriate init() function for the entity type.
        #undef f
        #define f(x) case x:  ((ent_##x *)(&array[index])) -> init(); break; 
        ENTITY_TYPES_LIST
        default:
            printf("*** spawn_ent() error: invalid entity type: %d", type);
            exit(-1);
    }
    return &array[index];
}
// Remove an entity from an entity segment array. TODO ent-specific cleanup TODO
void despawn_ent(segment* ent_header) {
    int size = ent_header->head.size;
    if (DEBUG_ENTS)
        printf("Despawning ent of size %d\n", size);
    memset(ent_header, 0, size*sizeof(segment));
}
// Run the think() function for each entitiy in a segment array.
void think_all_ents(segment* array, int array_len) {
    int type = EMPTY;
    int i = 0;
    i = get_first_ent(array, array_len);
    while (i != -1) {
        if (array[i].head.header_byte != HEADER_BYTE) {
            if (DEBUG_ENTS)
                printf("*** Invalid index given by get_next_ent() in think_all_ents()\n");
            break;
        }
        // Run the correct think function for this entity:
        type = array[i].head.type;
        //printf("Thinking entity type: '%s' at index %d.\n", get_type_name(type), i);
        switch (type) {
            // This macro formats the entity types list into a series of case statements.
            // Each case statement calls the appropriate think() function for the entity type.
            #undef f
            #define f(x) case x:  ((ent_##x *)(&array[i])) -> think(); break; 
            ENTITY_TYPES_LIST
            default:
                printf("*** entity at %d not recognized in think_all_ents().\n",  i);
                exit(-1);
        }
        i = get_next_ent(i, array, array_len);
    }
}
// Update an ent's position based on its velocity:
void move_ent(segment* e) {
    vec2f v = e[vel].vel.vel;
    vec2f p = e[pos].pos.pos;
    e[pos].pos.pos = p + v*dt;
    // Apply friction:
    float friction = v.vlen();
    if (friction < 0) // Prevent negative friction.
        friction = 0;
    friction = (friction * friction) / 10000;
    e[vel].vel.vel = v - (v.normalized() * friction);
    // Set a lower bound on velocity:
    if (e[vel].vel.vel.vlen() < 5)
        e[vel].vel.vel = vec2f{0,0};
}
