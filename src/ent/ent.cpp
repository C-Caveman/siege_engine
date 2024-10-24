// Implementations of entity functions.
#include "ent.h"
#include "../audio/audio.h"
#include "../client/client.h"

extern volatile float mouse_angle; // Direction the mouse is pointed in.
extern volatile int mouse_x;
extern volatile int mouse_y;
extern float dt; // Delta time.
extern struct anim_info anim_data[];

struct handle_info handles[NUM_HANDLES] = { //=================================// ENTITY HANDLES //
    {nullptr,1,true}, // Handle 0 is the Null handle.
    {}
};
handle claim_handle(struct ent_basics* e) { //-------- Bind a handle to an entity.
    handle h = -1;
    for (int i=0; i<NUM_HANDLES; i++) {
        if (handles[i].copies == 0) {h=i; break;}
    }
    if (h == -1) { printf("\n*** Ran out of handles!!!\n"); exit(-1); }
    handles[h].copies = 1;
    handles[h].ent = e;
    handles[h].claimed = 1;
    //printf("Handle %d claimed by a '%s' ent.\n", h, get_type_name(handles[h].ent->type));
    return h;
}
void unclaim_handle(handle i) { //-------------------- Unbind a handle (not a copy of one).
    if (i != 0) { //Null handle.
        handles[i].claimed = 0;
        handles[i].copies--;
    }
    if (DEBUG_ENT_HANDLES) { printf("Unclaimed handle %d. Copies = %d.\n", i, handles[i].copies); }
}
handle copy_handle(handle i) { //--------------------- Copy a bound handle (another entity's handle).
    if (handles[i].claimed == true && i != 0)
        { handles[i].copies++; }
    else
        { i = 0; } //Null handle.
    if (DEBUG_ENT_HANDLES) { printf("Copied handle %d. Now has %d copies.\n", i, handles[i].copies); }
    return i;
}
handle uncopy_handle(handle i) { //------------------- Uncopy a bound handle, replace with null.
    if (i != 0) { handles[i].copies--; } //Null handle cannot be destroyed.
    return 0;
}
struct ent_basics*  get_ent(handle i) { //------------ Get an entity by its handle.
    if (handles[i].claimed == 1)
        { return handles[i].ent; }
    else
        { handles[i].copies--; return nullptr; }
}//===============================================================================// ENTITY FUNCTIONS. //
void ent_player::init() {
    if (DEBUG_ENTS) { printf("Player entity initializing!\n"); }
    pos = vec2f{0,0};
    // Init the sprites:
    num_sprites = NUM_PLAYER_SPRITES;
    sprites[PLAYER_BODY].anim = rocket_tank;
    sprites[PLAYER_GUN].anim = gun_grenade;
    sprites[PLAYER_GUN].flags |= LOOPING;
    // Sprint by default:
    movetype = MOVE_SPRINT;
}
void ent_player::think() {                              // PLAYER
}

void ent_scenery::init() {                              // SCENERY
    if (DEBUG_ENTS)
        printf("Scenery ent initializing!\n");
    num_sprites = NUM_SCENERY_SPRITES;
    sprites[SCENERY_SPRITE_1].anim = rocket_tank;
}
void ent_scenery::think() {
    //ent_basics* e = get_ent(fren);
    //if (e != nullptr) { vel = vel + ( e->pos-pos ).normalized() * 15; } //- Follow the player.
    //vel = vel * 1.1; //---------------------------------------------------- Slip around.
}

void ent_projectile::init() {                           // PROJECTILE
    num_sprites = 1;
    sprites[0].anim = grenade01Blink;
    sprites[0].flags |= LOOPING;
    flags = NOFRICTION;
    lifetime = 100;
    isExploding = 0;
}
void ent_projectile::think() {
    lifetime -= 1;
    struct tile* curTile = main_world->get_tile( (pos + vec2f{RSIZE/2,RSIZE/2}).to_int() / RSIZE);
    if (lifetime <= 0 && !isExploding) {
        isExploding = 1;
        sprites[0].anim = grenade01Explode;
        sprites[0].frame = 0;
        sprites[0].flags &= ~LOOPING;
        //vel = vec2f {0,0};
        lifetime = 500;
    }
    if (curTile != 0 && curTile->wall_height > 0 && !isExploding) {
        isExploding = 1;
        //playSound(chow);
        sprites[0].anim = grenade01Explode;
        sprites[0].frame = 3;
        sprites[0].flags &= ~LOOPING;
        vel = vec2f {0,0};
        lifetime = 500;
        //despawn_ent((ent_basics*)this);
        
        return;
    }
    if (lifetime <= 0 || sprites[0].frame >= anim_data[grenade01Explode].len-1) {
        playSound(explosion01);
        if (curTile != 0 && curTile->wall_height > 0) {
            curTile->wall_height = 0;
            curTile->floor_anim = tileGold01;
        }
        despawn_ent((ent_basics*)this);
    }
}

void ent_rabbit::init() {                               // RABBIT
    wanderDir = vec2f{1,0};
    wanderWait = 100;
    num_sprites = 1;
    //sprites[0].flags |= LOOPING;
    sprites[0].anim = rabbitHop01;
    sprites[0].flags |= PAUSED;
    sprites[0].frame = 2;
}
void ent_rabbit::think() {
    wanderWait -= 1;
    if (wanderWait <= 0) {
        wanderWait = 100;
        wanderDir = vec2f{ (float)(rand()) / (float)(RAND_MAX) - 0.5f, (float)(rand()) / (float)RAND_MAX - 0.5f };
        vel = wanderDir.normalized() * 800.f;
        sprites[0].rotation = atan2(wanderDir.y, wanderDir.x) * 180. / M_PI + 90;
        sprites[0].rotation = (float)((int)sprites[0].rotation % 360);
        sprites[0].frame = 0;
        sprites[0].flags &= ~PAUSED;
    }
}

void ent_zombie::init() {                               // ZOMBIE
    wanderDir = vec2f{1,0};
    wanderWait = 100;
    num_sprites = 1;
    sprites[0].flags |= LOOPING;
    sprites[0].anim = zombie;
    target = 1; // first entity handle should be the player TODO add a player-finding function for reliability! TODO
    targetPos = {0,0};
}
#define MSIZE 1024
char message[MSIZE] = "Example message.... Greetings! Hello world! Goodbye world! Farewell world? Nice to meet you world? Oh well, see ya world!";
void loadMessage(char* fName, char* outString) {
    FILE* fp = fopen(fName, "r");
    if (!fp) {
        printf("Couldn't open %s\n", fName);
        exit(1);
    }
    for (int i=0; i<MSIZE; i++) {
        char c = fgetc(fp);
        if (feof(fp))
            break;
        outString[i] = c;
    } 
}
void ent_zombie::think() {
    struct ent_basics* e = get_ent(target);
    if (e != 0 && e->pos.dist(pos) < RSIZE/2 && playerClient.dialogVisible == 0) {
        loadMessage((char*)"assets/worlds/testWorld/hello.txt", message);
        playerClient.startDialog(message);
    }
    wanderWait -= 1;
    if (wanderWait <= 0 && e != 0) {
        wanderWait = 60;
        //playSound(tik);
        //wanderDir = vec2f{ (float)(rand()) / (float)(RAND_MAX) - 0.5f, (float)(rand()) / (float)RAND_MAX - 0.5f };
        wanderDir = targetPos - this->pos;
        vel = wanderDir.normalized() * 2000.f;
        
        
        sprites[0].rotation = atan2(wanderDir.y, wanderDir.x) * 180. / M_PI + 270.0;
        sprites[0].rotation = (float)((int)sprites[0].rotation % 360);
        targetPos = e->pos;
    }
}

//=====================================================// Entity management functions. (spawn, despawn, get_next, ect.) //
// X macro for ENTITY_TYPES_LIST:
#define expand(name) #name, 
char entity_type_names[NUM_ENT_TYPES][MAX_ENTITY_TYPE_NAME_LEN] = { ENTITY_TYPES_LIST };
#undef expand
// Return the name for a given entity type.
char* get_type_name(int type) {
    return entity_type_names[type];
}
// Return the size of the current entity type (in segments).
int get_ent_size(int type) {
    int size = -1;
    switch (type) {
    // X macro for ENTITY_TYPES_LIST:
    #define expand(name) case name##_type:  size = sizeof(struct ent_##name); break; 
    ENTITY_TYPES_LIST
    #undef expand
    default:
        printf("*** Unknown entity type in get_ent_size()\n");
        exit(-1);
    }
    return size;
}
// Return index of the next entity's first byte. Returns -1 if there are no more entities.
int get_next_ent(int i, char* array, int array_len) {
    if (array[i] == HEADER_BYTE) {
        int ent_size = ((struct ent_basics*)&array[i])->size;
        i += ent_size; // Skip past this entity.
        if (DEBUG_ENTS) { printf("get_next_ent() entity at %d. Size is %d.\n", i, ent_size); }
    }
    while (array[i] != HEADER_BYTE && i < array_len) { // Increment i until reaching the next entity.
        i += 1;
        if (DEBUG_ENTS) { printf("get_next_ent() skipping past %d.\n", i); }
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
    new_entity->header_byte = HEADER_BYTE;
    new_entity->type = type;
    new_entity->size = required_space;
    new_entity->h = claim_handle((struct ent_basics*)&array[i]);
    // Initialize the entity.
    switch (type) {
        #define expand(name) case name##_type:  ((struct ent_##name *)(&array[i]))->init(); break; //--- Init the entity.
        ENTITY_TYPES_LIST
        #undef expand
        default:
            printf("*** spawn_ent() error: invalid entity type: %d", type);
            exit(-1);
    }
    return &array[i];
}
// Remove an entity from an entity segment array. TODO ent-specific cleanup TODO
void despawn_ent(struct ent_basics* e) {
    tile* old_tile = &main_world->chunks[e->chunk.y][e->chunk.x].tiles[e->tile.y][e->tile.x];
    bool old_tile_was_valid = (e->tile.x > -1 && e->tile.x < CHUNK_WIDTH && e->tile.y > -1 && e->tile.y < CHUNK_WIDTH);
    for (int i=0; i<MAX_ENTS_PER_TILE; i++) {
        if (old_tile_was_valid && old_tile->ents[i] == e->h) { old_tile->ents[i] = 0; } //---- Remove handle from old tile.
    }
    unclaim_handle(e->h);
    int size = e->size;
    if (DEBUG_ENTS) { printf("Despawning ent of size %d\n", size); }
    memset((void*)e, 0, size*sizeof(char));
}
// Run the think() function for each entitiy in a segment array.
void think_all_ents(char* array, int array_len) {
    int type;
    for (int i=get_first_ent(array, array_len); i != -1; i=get_next_ent(i, array, array_len)) {
        if (array[i] != HEADER_BYTE) {
            if (DEBUG_ENTS) { printf("*** Invalid index given by get_next_ent() in think_all_ents()\n"); }
            break;
        }
        // Run the correct think function for this entity:
        type = ((struct ent_basics*)&array[i])->type;
        //printf("Thinking entity type: '%s' at index %d.\n", get_type_name(type), i);
        switch (type) {
            // X macro for ENTITY_TYPES_LIST:
            #define expand(name) case name##_type:  ((struct ent_##name *)(&array[i])) -> think(); break; 
            ENTITY_TYPES_LIST
            #undef expand
            default:
                printf("*** entity at %d not recognized in think_all_ents().\n",  i);
                exit(-1);
        }
    }
}
void move_ent(struct ent_basics* e) { //------------ Update an ent's position based on its velocity:
    e->pos = e->pos + (e->vel * dt);
    // Apply friction:
    float speed = e->vel.vlen();
    float friction = speed*8*dt;
    int hasFriction = (e->flags & NOFRICTION) == 0;
    e->vel = e->vel - (e->vel.normalized() * friction * hasFriction);
    if (e->vel.vlen() < 5) { e->vel = vec2f{0,0}; } // Minimum vel.
}
