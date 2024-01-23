// Implementations of entity functions.
#include "ent.h"

extern volatile float mouse_angle; // Direction the mouse is pointed in.
extern volatile int mouse_x;
extern volatile int mouse_y;
extern float dt; // Delta time.

// Used to give each spawned entity a unique id number.
uint16_t unique_entity_id = 0;

//---------------------------------------- Initialize an entity (to the default values for its type).
void ent_player::init() {
    if (DEBUG_ENTS)
        printf("Player entity initializing!\n");
    flags = DRAWABLE | ANIMATABLE | MOVABLE | COLLIDABLE | THINKABLE;
    pos = vec2f{0,0};
    // Init the sprites:
    num_sprites = NUM_PLAYER_SPRITES;
    sprites[PLAYER_BODY].anim = rocket_tank;
    sprites[PLAYER_GUN].anim = gun_grenade;
    // Sprint by default:
    movetype = MOVE_SPRINT;
}
void ent_scenery::init() {
    if (DEBUG_ENTS)
        printf("Scenery ent initializing!\n");
    flags = DRAWABLE | ANIMATABLE;
    pos = vec2f{0,0};
    num_sprites = NUM_SCENERY_SPRITES;
    sprites[SCENERY_SPRITE].anim = sand;
}

//--------------------------------------------------------------------- Update an entity for the next frame.
void ent_player::think() {
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
void ent_scenery::think() {}

//--------------------------- Entity management functions. (spawning, despawning, getting next ent in array, ect.)
#undef f
#define f(x) #x, 
// This macro formats the ENTITY_TYPES_LIST into an array of strings.
char entity_type_names[NUM_ENT_TYPES][MAX_ENTITY_TYPE_NAME_LEN] = { ENTITY_TYPES_LIST };
// Return the name for a given entity type.
char* get_type_name(int type) {
    return entity_type_names[type];
}
// Return the size of the current entity type (in segments).
int get_ent_size(int type) {
    int size = -1;
    switch (type) {
    // This macro formats the entity types list into a series of case statements.
    // Each case statement sets the appropriate size for that entity type.
    #undef f
    #define f(x) case x##_type:  size = sizeof(struct ent_##x); break; 
    ENTITY_TYPES_LIST
    default:
        printf("*** Unknown entity type in get_ent_size()\n");
        exit(-1);
    }
    return size;
}
// Return index of the next entity's first byte. Returns -1 if there are no more entities.
int get_next_ent(int i, char* array, int array_len) {
    // Is byte i the head of an entity?
    if (array[i] == HEADER_BYTE) {
        int ent_size = ((struct ent_basics*)&array[i])->size;
        i += ent_size; // Skip past this entity.
        if (DEBUG_ENTS) { printf("get_next_ent() entity at %d. Size is %d.\n", i, ent_size); }
    }
    // Increment i until reaching the next entity:
    while (i < array_len && array[i] != HEADER_BYTE) {
        //if (array[i] != 0) { printf("Found %c\n", array[i]); exit(-1); }
        if (DEBUG_ENTS) { printf("get_next_ent() skipping past %d.\n", i); }
        i += 1;
    }
    if (i >= array_len-1) // Out of bounds.
        i = -1;
    return i;
}
// Return index of the first entity in a segment array. Return -1 if no entities are found.
int get_first_ent(char* array, int array_len) {
    int i=0;
    // Search for the first valid header segment with non-empty entity.
    for (i=0; i<array_len; i++) {
        if (array[i] == HEADER_BYTE)
            break;
    }
    if (i >= (array_len-1))
        i = -1;
    return i;
}
// Make a new entity in the given segment array. Return its index.
void* spawn_ent(int type, char* array, int array_len) {
    int required_space = get_ent_size(type);
    int empty_space_len = 0;
    int i = 0;
    while (i<array_len) {
        // Empty slot?
        if (array[i] != HEADER_BYTE) {
            if (DEBUG_ENT_SPAWNING) { printf("Found an open slot at %d.\n", i); }
            empty_space_len += 1;
            i += 1;
        }
        // Slot occupied.
        else {
            int skip_bytes = ((struct ent_basics*)&array[i])->size;
            if (DEBUG_ENT_SPAWNING) { printf("Slots [%d, %d] already taken.\n", i, i+skip_bytes-1); }
            empty_space_len = 0;
            i += skip_bytes;
        }
        // Got enough space to store the ent.
        if (empty_space_len == required_space) {
            if (DEBUG_ENT_SPAWNING) { printf("Found enough space for ent in [%d, %d]\n", i-required_space, i-1); }
            i = i-required_space;
            break;
        }
    }
    if (i >= array_len-1) {
        printf("***\n*** No space left in the entity array!!!\n***\n");
        exit(-1);
    }
    // Initialize the entity's header info:
    struct ent_basics* new_entity = (struct ent_basics*)&array[i];
    //new_entity->header_byte = HEADER_BYTE;
    array[i] = HEADER_BYTE;
    new_entity->type = type;
    new_entity->size = required_space;
    new_entity->handle = unique_entity_id; // TODO add a handle here!
    unique_entity_id += 1; // Make sure the next entity id number is different. TODO ensure unique
    // Initialize the entity.
    switch (type) {
        // This macro formats the entity types list into a series of case statements.
        // Each case statement calls the appropriate init() function for the entity type.
        #undef f
        #define f(x) case x##_type:  ((struct ent_##x *)(&array[i])) -> init(); break; 
        ENTITY_TYPES_LIST
        default:
            printf("*** spawn_ent() error: invalid entity type: %d", type);
            exit(-1);
    }
    return &array[i];
}
// Remove an entity from an entity segment array. TODO ent-specific cleanup TODO
void despawn_ent(void* ent) {
    int size = ((struct ent_basics*)ent)->size;
    if (DEBUG_ENTS) { printf("Despawning ent of size %d\n", size); }
    memset(ent, 0, size*sizeof(char));
}
// Run the think() function for each entitiy in a segment array.
void think_all_ents(char* array, int array_len) {
    int type;
    int i = 0;
    i = get_first_ent(array, array_len);
    while (i != -1) {
        if (array[i] != HEADER_BYTE) {
            if (DEBUG_ENTS) { printf("*** Invalid index given by get_next_ent() in think_all_ents()\n"); }
            break;
        }
        // Run the correct think function for this entity:
        type = ((struct ent_basics*)&array[i])->type;
        //printf("Thinking entity type: '%s' at index %d.\n", get_type_name(type), i);
        switch (type) {
            // This macro formats the entity types list into a series of case statements.
            // Each case statement calls the appropriate think() function for the entity type.
            #undef f
            #define f(x) case x##_type:  ((struct ent_##x *)(&array[i])) -> think(); break; 
            ENTITY_TYPES_LIST
            default:
                printf("*** entity at %d not recognized in think_all_ents().\n",  i);
                exit(-1);
        }
        i = get_next_ent(i, array, array_len);
    }
}
// Update an ent's position based on its velocity:
void move_ent(struct ent_basics* e) {
    vec2f v = e->vel;
    vec2f p = e->pos;
    e->pos = p + v*dt;
    // Apply friction:
    float friction = v.vlen();
    if (friction < 0) { friction = 0; }
    friction = (friction * friction) / 10000; //TODO clean this up a lot
    e->vel = v - (v.normalized() * friction);
    if (e->vel.vlen() < 5) { e->vel = vec2f{0,0}; } // Minimum vel.
}
