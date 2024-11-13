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
}//===============================================================================// ENTITY FUNCTIONS. //;;
void nearbyEntInteraction(vec2f position, void (*fn)(struct ent_basics*)) {
    vec2f p = position + HW - vec2f{RSIZE,RSIZE}; // Top left corner of the 3x3.
    for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
            for (int k=0; k<MAX_ENTS_PER_TILE; k++) {
                struct tile* curTile = main_world->tileFromPos(p + vec2f{0,RSIZE}*(float)(i) + vec2f{RSIZE,0}*(float)(j));
                if (curTile) {
                    ent_basics* useTarget = get_ent(curTile->ents[k]);
                    if (useTarget) {
                        fn(useTarget);
                    }
                }
            }
        }
    }
}
void nearbyEntInteractionBidirectional(struct ent_basics* user, void (*fn)(struct ent_basics*, struct ent_basics*)) {
    vec2f p = user->pos + HW - vec2f{RSIZE,RSIZE}; // Top left corner of the 3x3.
    for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
            for (int k=0; k<MAX_ENTS_PER_TILE; k++) {
                struct tile* curTile = main_world->tileFromPos(p + vec2f{0,RSIZE}*(float)(i) + vec2f{RSIZE,0}*(float)(j));
                if (curTile) {
                    ent_basics* useTarget = get_ent(curTile->ents[k]);
                    if (useTarget) {
                        fn(user, useTarget);
                    }
                }
            }
        }
    }
}
void ent_player::init() {
    if (DEBUG_ENTS) { printf("Player entity initializing!\n"); }
    health = 1;
    heat.interval = 4;
    pos = vec2f{0,0};
    // Init the sprites:
    num_sprites = NUM_PLAYER_SPRITES;
    sprites[PLAYER_BODY].anim = vTankBody03;
    sprites[PLAYER_BODY].flags |= PAUSED;
    sprites[PLAYER_FLAMES].anim = vRocketFire01;
    sprites[PLAYER_FLAMES].flags |= INVISIBLE | LOOPING;
    sprites[PLAYER_FLAMES_EXTRA].anim = vRocketFire02;
    sprites[PLAYER_FLAMES_EXTRA].flags |= INVISIBLE | LOOPING;
    sprites[PLAYER_GUN].anim = gunGrenadeRetract;
    sprites[PLAYER_GUN].frame = anim_data[gunGrenadeRetract].len-1;
    sprites[PLAYER_GUN].flags |= PAUSED;
    sprites[PLAYER_CROSSHAIR].anim = crosshair01;
    // Sprint by default:
    movetype = MOVE_SPRINT;
}
int numRabbitPets = 0;
#define INTERACT_DIALOG_LEN 256
char rabbitPetDialog[INTERACT_DIALOG_LEN];
void playerInteract(struct ent_basics* player, struct ent_basics* useTarget) {
    struct ent_player* user = (struct ent_player*)player;
    if (useTarget && useTarget->type == scenery_type && user->cl && user->cl->interacting) {
        printf("Used a scenery ent!\n");
        user->cl->interacting = false;
        //cl->interacting = false;
        //playerClient.loadDialog((char*)"assets/worlds/testWorld/hello.txt");
        strcpy(playerClient.loadedDialog, (char*)"<apig>Hello world!\n");
        playerClient.startDialog(playerClient.loadedDialog);
    }
    else if (useTarget && useTarget->type == rabbit_type) {
        user->cl->interacting = false;
        playSound(meow);
        numRabbitPets++;
        if (numRabbitPets == 1)
            snprintf(playerClient.dialogPrintString, INTERACT_DIALOG_LEN, "You pet the rabbit 1 time.                      \n");
        else
            snprintf(playerClient.dialogPrintString, INTERACT_DIALOG_LEN, "You pet the rabbit %d times.                    \n", numRabbitPets);
        if (!playerClient.dialogVisible) {
            strcpy(playerClient.dialogAnnotation, (char*)"book");
            playerClient.changeActor();
            playerClient.startDialog(playerClient.dialogPrintString);
        }
        else if ((size_t)playerClient.dialogStringPos >= strnlen(playerClient.dialogPrintString, INTERACT_DIALOG_LEN)-20) {
            playerClient.dialogWaitTimer = 5000;
        }
    }
}
void ent_player::think() {                              // PLAYER
    if (cl && cl->dashing) {
        sprites[PLAYER_FLAMES].flags &= ~INVISIBLE;
    }
    else {
        sprites[PLAYER_FLAMES].flags |= INVISIBLE;
        sprites[PLAYER_FLAMES_EXTRA].flags |= INVISIBLE;
    }
    if (cl && cl->dashing && heat.count < HEAT_MAX)
        counterInc(&heat);
    else if (heat.count > 0 && cl && !cl->dashing)
        counterDec(&heat);
    if (cl && cl->dashing && heat.count < HEAT_MAX && !isChannelPlaying(CHAN_ENGINE)) {
        playSoundChannel(rocketEngineLoop, CHAN_ENGINE);
    }
    else if (cl && cl->dashing && heat.count == HEAT_MAX && !isChannelPlaying(CHAN_ENGINE)) {
        playSoundChannel(rocketEngineLoopFast, CHAN_ENGINE);
    }
    if (heat.count > 0) {
        if (heat.count == HEAT_MAX && sprites[PLAYER_GUN].frame < anim_data[gunGrenadeBoost].len-1) {
            playSoundChannel(rocketBoostEngage, CHAN_WEAPON_ALT);
            playSoundChannel(rocketEngineLoopFast, CHAN_ENGINE);
            sprites[PLAYER_FLAMES_EXTRA].flags &= ~INVISIBLE;
        }
        sprites[PLAYER_GUN].anim = gunGrenadeBoost;
        sprites[PLAYER_GUN].flags |= PAUSED;
        sprites[PLAYER_GUN].frame = (int)((float)heat.count/(float)(HEAT_MAX)*(float)(anim_data[gunGrenadeBoost].len-1));
        sprites[PLAYER_BODY].frame = (int)((float)heat.count/(float)(HEAT_MAX)*(float)(anim_data[vTankBody03].len-1));
        sprites[PLAYER_CROSSHAIR].frame = (int)((float)heat.count/(float)(HEAT_MAX)*(float)(anim_data[crosshair01].len-1));
    }
    else if (cl && !cl->dashing && sprites[PLAYER_GUN].anim == gunGrenadeBoost && heat.count <= 0) {
        sprites[PLAYER_GUN].anim = gunGrenadeRetract;
        sprites[PLAYER_GUN].frame = anim_data[gunGrenadeRetract].len-1;
        playSoundChannel(rocketBoostEnd, CHAN_WEAPON_ALT);
        sprites[PLAYER_CROSSHAIR].frame = 0;
    }
    if (cl && !cl->keyboardAiming) {
        sprites[PLAYER_CROSSHAIR].pos = cl->aim_pixel_pos.to_float();
    }
    else {
        vec2f crosshairDir = angleToVector(sprites[PLAYER_GUN].rotation);
        sprites[PLAYER_CROSSHAIR].pos = crosshairDir*RSIZE*4;
    }
    sprites[PLAYER_FLAMES].rotation = sprites[PLAYER_GUN].rotation;
    sprites[PLAYER_FLAMES].pos = angleToVector(sprites[PLAYER_GUN].rotation) * (-RSIZE*3/4);
    sprites[PLAYER_FLAMES_EXTRA].rotation = sprites[PLAYER_FLAMES].rotation;
    sprites[PLAYER_FLAMES_EXTRA].pos = sprites[PLAYER_FLAMES].pos * 1.8;
    //vec2f p = pos + HW;
    if (cl && cl->interacting) {
        nearbyEntInteractionBidirectional((struct ent_basics*)this, playerInteract);
    }
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
    health = 1;
    num_sprites = 1;
    sprites[0].anim = grenade01Blink;
    sprites[0].flags |= LOOPING;
    flags = NOFRICTION | NOCOLLISION;
    lifetime = 100;
    isExploding = 0;
}
void projectileHitNearby(struct ent_basics* attacker, struct ent_basics* victim) {
    if (victim && victim->type == zombie_type && victim->health > 0) {
        float victimDistance = attacker->pos.dist(victim->pos);
        if (victimDistance > RSIZE*0.8) {
            return;
        }
        victim->health -= 1;
        ((struct ent_projectile*)attacker)->lifetime = 0;
    }
}

void ent_projectile::think() {
    lifetime -= 1;
    struct tile* curTile = 0;
    vec2f p = pos + HW;
    p = p - vec2f{RSIZE,RSIZE};
    nearbyEntInteractionBidirectional((struct ent_basics*)this, projectileHitNearby);
    curTile = main_world->tileFromPos(pos + HW);
    if (lifetime <= 0 && !isExploding) {
        isExploding = 1;
        playSound(explosion01);
        sprites[0].anim = grenade01Explode;
        sprites[0].frame = 0;
        sprites[0].flags &= ~LOOPING;
        //vel = vec2f {0,0};
        lifetime = 500;
    }
    if (curTile != 0 && curTile->wall_height > 0 && !isExploding) {
        isExploding = 1;
        playSoundChannel(explosion01, CHAN_EXPLOSION);
        //playSound(chow);
        sprites[0].anim = grenade01Explode;
        sprites[0].frame = 3;
        sprites[0].flags &= ~LOOPING;
        vel = vec2f {0,0};
        lifetime = 500;
        if (curTile != 0 && curTile->wall_height > 0) {
            curTile->wall_height = 0;
            curTile->floor_anim = tileGold01;
        }
        return;
    }
    if (lifetime <= 0 || sprites[0].frame >= anim_data[grenade01Explode].len-1) {
        despawn_ent((ent_basics*)this);
    }
}

void ent_rabbit::init() {                               // RABBIT
    health = 1;
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
    health = 1;
    wanderDir = vec2f{1,0};
    wanderWait = 100;
    num_sprites = 1;
    sprites[0].flags |= LOOPING;
    sprites[0].anim = zombie;
    target = 1; // first entity handle should be the player TODO add a player-finding function for reliability! TODO
    targetPos = {0,0};
    walkDelay.interval = 5;
}
void pushNearbyEnts(struct ent_basics* me, struct ent_basics* them) {
    if (them->type != zombie_type)
        return;
    vec2f posDelta = me->pos - them->pos;
    float d = (posDelta).vlen();
    if (d < RSIZE) {
        me->vel = me->vel + posDelta*500*dt;
        them->vel = them->vel - posDelta*500*dt;
    }
}
#define MSIZE 1024
char message[MSIZE] = "Example message.... Greetings! Hello world! Goodbye world! Farewell world? Nice to meet you world? Oh well, see ya world!";
void ent_zombie::think() {
    if (health <= 0) {
        playSound(zombieDie01);
        int numGibs = anim_data[zombieGibs].len;
        for (int i=0; i<numGibs; i++) {
            struct ent_basics* newGib = (struct ent_basics*)spawn_ent(gib_type, main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
            if (newGib) {
                ((struct ent_gib*)newGib)->pos = pos;
                ((struct ent_gib*)newGib)->sprites[0].rotation = sprites[0].rotation;
                ((struct ent_gib*)newGib)->sprites[0].anim = zombieGibs;
                ((struct ent_gib*)newGib)->sprites[0].frame = i;
                ((struct ent_gib*)newGib)->vel = vec2f{randfn()*10000,randfn()*10000};
            }
        }
        despawn_ent((ent_basics*)this);
        return;
    }
    nearbyEntInteractionBidirectional((struct ent_basics*)this, pushNearbyEnts);
    struct ent_basics* e = get_ent(target);
    if (e != 0 && e->pos.dist(pos) < RSIZE/2 && playerClient.dialogVisible == 0) {
        playerClient.loadDialog((char*)"assets/worlds/testWorld/hello.txt");
        playerClient.startDialog(playerClient.loadedDialog);
    }
    wanderWait -= 1;
    counterInc(&walkDelay);
    if (walkDelay.count > 0) { //wanderWait <= 0 && e != 0) {
        walkDelay.count = 0;
        wanderWait = 10;
        //playSound(tik);
        //wanderDir = vec2f{ (float)(rand()) / (float)(RAND_MAX) - 0.5f, (float)(rand()) / (float)RAND_MAX - 0.5f };
        wanderDir = targetPos - this->pos;
        vel = wanderDir.normalized() * 400.f;
        
        
        sprites[0].rotation = vectorToAngle(wanderDir) + 270;//atan2(wanderDir.y, wanderDir.x) * 180. / M_PI + 270.0;
        sprites[0].rotation = (float)((int)sprites[0].rotation % 360);
        targetPos = e->pos;
    }
}
void ent_gib::init() {
    num_sprites = 1;
    sprites[0].anim = zombieGibs;
    sprites[0].flags |= PAUSED;
    lifetime = 10000 * randf();
}
void ent_gib::think() {
    lifetime--;
    if (lifetime < 0)
        despawn_ent((ent_basics*)this);
}
//======================================================================================================================//
//=====================================================// Entity management functions. (spawn, despawn, get_next, ect.) //
// X macro for ENTITY_TYPES_LIST:
char entity_type_names[NUM_ENT_TYPES][MAX_ENTITY_TYPE_NAME_LEN] = {
    ENTITY_TYPES_LIST(TO_STRING)
};
// Return the name for a given entity type.
char* get_type_name(int type) {
    return entity_type_names[type];
}
// Return the size of the current entity type (in segments).
int get_ent_size(int type) {
    int size = -1;
    switch (type) {
    // X macro for ENTITY_TYPES_LIST:
    #define GET_ENT_SIZES(name) case name##_type:  size = sizeof(struct ent_##name); break; 
    ENTITY_TYPES_LIST(GET_ENT_SIZES)
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
        #define ENT_INIT_CASES(name) case name##_type:  ((struct ent_##name *)(&array[i]))->init(); break; //--- Init the entity.
        ENTITY_TYPES_LIST(ENT_INIT_CASES)
        default:
            printf("*** spawn_ent() error: invalid entity type: %d", type);
            exit(-1);
    }
    return &array[i];
}
void* spawn(int type, vec2f pos) { // Spawn an ent in the default entity array.
    char* array = main_world->entity_bytes_array;
    int array_len = ENTITY_BYTES_ARRAY_LEN;
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
    new_entity->pos = pos;
    // Initialize the entity.
    switch (type) {
        #define ENT_INIT_CASES(name) case name##_type:  ((struct ent_##name *)(&array[i]))->init(); break; //--- Init the entity.
        ENTITY_TYPES_LIST(ENT_INIT_CASES)
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
            #define ENT_THINK_CASES(name) case name##_type:  ((struct ent_##name *)(&array[i])) -> think(); break; 
            ENTITY_TYPES_LIST(ENT_THINK_CASES)
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

float PLAYER_WIDTH = RSIZE;
float MIN_SQUARE_DISTANCE = PLAYER_WIDTH/2 + RSIZE/2;
void collide_wall(struct ent_basics* e) {
    vec2f* position = &e->pos;
    vec2f centered_position = e->pos + vec2f{RSIZE/2, RSIZE/2};
    vec2f nearest_corner = centered_position;
    nearest_corner = nearest_corner / RSIZE;
    nearest_corner = vec2f{std::floor(nearest_corner.x + 0.5f),std::floor(nearest_corner.y + 0.5f)}; //--- Nearest corner.
    nearest_corner = nearest_corner * RSIZE;
    vec2f tile_pos;//------------------------------------------------------------------------------------- Adjacent tiles.
    vec2i tile_index;
    float sign_x = -1;
    float sign_y = -1;
    bool collisions[2][2];
    int num_collisions = 0;
     for (int i=0; i<2; i++) {
        for (int j=0; j<2; j++) {
            tile_pos = nearest_corner + vec2f{RSIZE/2*sign_x, RSIZE/2*sign_y};
            tile_index = (tile_pos / RSIZE).to_int();  //vec2i{int(tile_pos.x/RSIZE), int(tile_pos.y/RSIZE)} % CHUNK_WIDTH;
            tile* cur_tile = main_world->get_tile(tile_index);
            if (cur_tile != nullptr && cur_tile->wall_height >= 1)
                { collisions[i][j] = true; num_collisions++; }
            else
                { collisions[i][j] = false; }
            sign_x *= -1;
        }
        sign_y *= -1;
    }
    bool is_vertical_pair = (num_collisions == 2) && (collisions[0][0] == collisions[1][0]);
    bool is_horizontal_pair = (num_collisions == 2) && (collisions[0][0] == collisions[0][1]);
    if (num_collisions == 0) return;
    for (int i=0; i<2; i++) { // -------------------------------------------------------------- Apply the collisions.
        for (int j=0; j<2; j++) {
            tile_pos = nearest_corner + vec2f{RSIZE/2*sign_x, RSIZE/2*sign_y};
            sign_x *= -1;
            tile_pos = tile_pos / RSIZE;
            tile_pos = vec2f{std::floor(tile_pos.x), std::floor(tile_pos.y)} * RSIZE;
            tile_index = vec2i{int(tile_pos.x/RSIZE), int(tile_pos.y/RSIZE)};
            tile* cur_tile = main_world->get_tile(tile_index);
            if (cur_tile == nullptr || cur_tile->wall_height <=0) { continue; } // Skip the tile.
            vec2f delta = *position - tile_pos;
            bool in_square = abs(delta.x) < MIN_SQUARE_DISTANCE && abs(delta.y) < MIN_SQUARE_DISTANCE;
            bool in_diamond = abs(delta.x) + abs(delta.y) > RSIZE*1.2;
            if (in_square) {
                float x_delta_sign = 1;
                float y_delta_sign = 1;
                if (delta.x < 0) x_delta_sign = -1;
                if (delta.y < 0) y_delta_sign = -1;
                if (in_diamond && num_collisions == 1) { //------------------------------------- Circle-style collision on tile corners.
                    if (delta.vlen() < RSIZE) { *position = *position + delta.normalized()*(RSIZE-delta.vlen()); }
                }
                else if (abs(delta.x) > abs(delta.y) && !is_horizontal_pair) //----------------- Square-style collision on tile sides.
                    { *position = *position + vec2f{x_delta_sign*MIN_SQUARE_DISTANCE-delta.x, 0}; }
                else if (abs(delta.x) < abs(delta.y) && !is_vertical_pair)
                    { *position = *position + vec2f{0, y_delta_sign*MIN_SQUARE_DISTANCE-delta.y}; }
            }
        }
        sign_y *= -1;
    }
}
void wallCollision(char* array, int array_len) {
    for (int i=get_first_ent(array, array_len); i != -1; i=get_next_ent(i, array, array_len)) {
        if (array[i] != HEADER_BYTE) {
            if (DEBUG_ENTS) { printf("*** Invalid index given by get_next_ent() in think_all_ents()\n"); }
            break;
        }
        struct ent_basics* e = ((struct ent_basics*)&array[i]);
        if ((e->flags & NOCOLLISION) != NOCOLLISION)
            collide_wall(e);
    }
}
