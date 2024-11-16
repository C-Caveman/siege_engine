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
    {0,1,true}, // Handle 0 is the Null handle.
};
handle claim_handle(ent_basics* e) { //-------- Bind a handle to an entity.
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
ent_basics*  get_ent(handle i) { //------------ Get an entity by its handle.
    if (handles[i].claimed == 1)
        { return handles[i].ent; }
    else
        { handles[i].copies--; return 0; }
}//===============================================================================// ENTITY FUNCTIONS. //;;
void nearbyEntInteraction(vec2f position, void (*fn)(ent_basics*)) {
    vec2f p = v2fSub(v2fAdd(position, HW), (vec2f){RSIZE,RSIZE}); // Top left corner of the 3x3.
    for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
            for (int k=0; k<MAX_ENTS_PER_TILE; k++) {
                struct tile* curTile = worldTileFromPos(v2fAdd(v2fAdd(p, v2fScale((vec2f){0,RSIZE}, (float)(i))), v2fScale((vec2f){RSIZE,0}, (float)(j))));
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
void nearbyEntInteractionBidirectional(ent_basics* user, void (*fn)(ent_basics*, ent_basics*)) {
    vec2f p = v2fSub(v2fAdd(user->pos, HW), (vec2f){RSIZE,RSIZE}); // Top left corner of the 3x3.
    for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
            for (int k=0; k<MAX_ENTS_PER_TILE; k++) {
                struct tile* curTile = worldTileFromPos(v2fAdd(v2fAdd(p, v2fScale((vec2f){0,RSIZE}, (float)(i))), v2fScale((vec2f){RSIZE,0}, (float)(j))));
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
struct eventsBuffer events;
#define TO_EVENT_CASE(name, detailsUnused) case event##name: ev##name(&ev->details.d##name); break;
void applyEvent(struct event* ev) {
    switch(ev->type) {
        EVENT_LIST(TO_EVENT_CASE)
    };
}
void makeEvent(struct event e) {
    if (events.count >= EVENT_BUFFER_SIZE-1)
        return;
    int newEventIndex = (events.index+events.count) % EVENT_BUFFER_SIZE;
    events.buffer[newEventIndex].time = 0;
    events.buffer[newEventIndex].sequenceNumber = events.sequenceNumber++;
    //events.buffer[newEventIndex].e = e;
    memcpy((void*)&events.buffer[newEventIndex].e, (void*)&e, sizeof(struct event));
    events.count++;
}
void takeEvent() {
    if (events.count <= 0)
        return;
    applyEvent(&events.buffer[events.index].e);
    memset((void*)&events.buffer[events.index], 0, sizeof(struct packet));
    events.count--;
    events.index++;
    if (events.index >= EVENT_BUFFER_SIZE)
        events.index = 0;
}
void evPlayerMove(struct detailsPlayerMove* d) {
    printf("Player moves!\n");
    struct ent_player* p = (struct ent_player*)get_ent(d->p);
    if (p == 0)
        return;
    p->pos = d->pos;
    p->vel = d->vel;
}
void evPlayerShoot(struct detailsPlayerShoot* d) {
    printf("Player shoots!\n");
}
void evEntMove(struct detailsEntMove* d) {
    ent_basics* e = get_ent(d->e);
    if (!e)
        return;
    e->pos = d->pos;
    e->vel = d->vel;
}
void playerInit(struct ent_player* e) {
    if (DEBUG_ENTS) { printf("Player entity initializing!\n"); }
    e->health = 1;
    e->heat.interval = 4;
    e->pos = (vec2f){0,0};
    // Init the sprites:
    e->num_sprites = NUM_PLAYER_SPRITES;
    e->sprites[PLAYER_BODY].anim = vTankBody03;
    e->sprites[PLAYER_BODY].flags |= PAUSED;
    e->sprites[PLAYER_FLAMES].anim = vRocketFire01;
    e->sprites[PLAYER_FLAMES].flags |= INVISIBLE | LOOPING;
    e->sprites[PLAYER_FLAMES_EXTRA].anim = vRocketFire02;
    e->sprites[PLAYER_FLAMES_EXTRA].flags |= INVISIBLE | LOOPING;
    e->sprites[PLAYER_GUN].anim = gunGrenadeRetract;
    e->sprites[PLAYER_GUN].frame = anim_data[gunGrenadeRetract].len-1;
    e->sprites[PLAYER_GUN].flags |= PAUSED;
    e->sprites[PLAYER_CROSSHAIR].anim = crosshair01;
    // Sprint by default:
    e-> movetype = MOVE_SPRINT;
}
int numRabbitPets = 0;
#define INTERACT_DIALOG_LEN 256
char rabbitPetDialog[INTERACT_DIALOG_LEN];
void playerInteract(ent_basics* player, ent_basics* useTarget) {
    struct ent_player* user = (struct ent_player*)player;
    if (useTarget && useTarget->type == scenery_type && user->cl && user->cl->interacting) {
        printf("Used a scenery ent!\n");
        user->cl->interacting = false;
        //cl->interacting = false;
        //playerClient.loadDialog((char*)"assets/worlds/testWorld/hello.txt");
        strcpy(playerClient.loadedDialog, (char*)"<apig>Hello world!\n");
        clientStartDialog(playerClient.loadedDialog);
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
            clientChangeActor();
            clientStartDialog(playerClient.dialogPrintString);
        }
        else if ((size_t)playerClient.dialogStringPos >= strlen(playerClient.dialogPrintString)-20) {
            playerClient.dialogWaitTimer = 5000;
        }
    }
}
void playerThink(struct ent_player* e) {                              // PLAYER
    if (playerClient.zombieSpawning)
        spawn(zombie_type, v2fAdd(playerClient.camera_center, v2iToF(playerClient.aim_pixel_pos)));
    if (playerClient.dashing) {
        e->sprites[PLAYER_FLAMES].flags &= ~INVISIBLE;
    }
    else {
        e->sprites[PLAYER_FLAMES].flags |= INVISIBLE;
        e->sprites[PLAYER_FLAMES_EXTRA].flags |= INVISIBLE;
    }
    if (playerClient.dashing && e->heat.count < HEAT_MAX)
        counterInc(&e->heat);
    else if (e->heat.count > 0 && !playerClient.dashing)
        counterDec(&e->heat);
    if (playerClient.dashing && e->heat.count < HEAT_MAX && !isChannelPlaying(CHAN_ENGINE)) {
        playSoundChannel(rocketEngineLoop, CHAN_ENGINE);
    }
    else if (playerClient.dashing && e->heat.count == HEAT_MAX && !isChannelPlaying(CHAN_ENGINE)) {
        playSoundChannel(rocketEngineLoopFast, CHAN_ENGINE);
    }
    if (e->heat.count > 0) {
        if (e->heat.count == HEAT_MAX && e->sprites[PLAYER_GUN].frame < anim_data[gunGrenadeBoost].len-1) {
            playSoundChannel(rocketBoostEngage, CHAN_WEAPON_ALT);
            playSoundChannel(rocketEngineLoopFast, CHAN_ENGINE);
            e->sprites[PLAYER_FLAMES_EXTRA].flags &= ~INVISIBLE;
        }
        e->sprites[PLAYER_GUN].anim = gunGrenadeBoost;
        e->sprites[PLAYER_GUN].flags |= PAUSED;
        e->sprites[PLAYER_GUN].frame = (int)((float)e->heat.count/(float)(HEAT_MAX)*(float)(anim_data[gunGrenadeBoost].len-1));
        e->sprites[PLAYER_BODY].frame = (int)((float)e->heat.count/(float)(HEAT_MAX)*(float)(anim_data[vTankBody03].len-1));
        e->sprites[PLAYER_CROSSHAIR].frame = (int)((float)e->heat.count/(float)(HEAT_MAX)*(float)(anim_data[crosshair01].len-1));
    }
    else if (!playerClient.dashing && e->sprites[PLAYER_GUN].anim == gunGrenadeBoost && e->heat.count <= 0) {
        e->sprites[PLAYER_GUN].anim = gunGrenadeRetract;
        e->sprites[PLAYER_GUN].frame = anim_data[gunGrenadeRetract].len-1;
        playSoundChannel(rocketBoostEnd, CHAN_WEAPON_ALT);
        e->sprites[PLAYER_CROSSHAIR].frame = 0;
    }
    if (!playerClient.keyboardAiming) {
        e->sprites[PLAYER_CROSSHAIR].pos = v2iToF(playerClient.aim_pixel_pos);
    }
    else {
        vec2f crosshairDir = angleToVector(e->sprites[PLAYER_GUN].rotation);
        e->sprites[PLAYER_CROSSHAIR].pos = v2fScale(crosshairDir, RSIZE*4);
    }
    e->sprites[PLAYER_FLAMES].rotation = e->sprites[PLAYER_GUN].rotation;
    e->sprites[PLAYER_FLAMES].pos = v2fScale(angleToVector(e->sprites[PLAYER_GUN].rotation), (-RSIZE*3/4));
    e->sprites[PLAYER_FLAMES_EXTRA].rotation = e->sprites[PLAYER_FLAMES].rotation;
    e->sprites[PLAYER_FLAMES_EXTRA].pos = v2fScale(e->sprites[PLAYER_FLAMES].pos, 1.8);
    //vec2f p = pos + HW;
    if (playerClient.interacting) {
        nearbyEntInteractionBidirectional((ent_basics*)e, playerInteract);
    }
}

void sceneryInit(struct ent_scenery* e) {                              // SCENERY
    if (DEBUG_ENTS)
        printf("Scenery ent initializing!\n");
    e->num_sprites = NUM_SCENERY_SPRITES;
    e->sprites[SCENERY_SPRITE_1].anim = rocket_tank;
}
void sceneryThink(struct ent_scenery* e) {
    //ent_basics* e = get_ent(fren);
    //if (e != nullptr) { vel = vel + ( e->pos-pos ).normalized() * 15; } //- Follow the player.
    //vel = vel * 1.1; //---------------------------------------------------- Slip around.
}

void projectileInit(struct ent_projectile* e) {                           // PROJECTILE
    e->health = 1;
    e->num_sprites = 1;
    e->sprites[0].anim = grenade01Blink;
    e->sprites[0].flags |= LOOPING;
    e->flags = NOFRICTION | NOCOLLISION;
    e->lifetime = 100;
    e->isExploding = 0;
}
void projectileHitNearby(ent_basics* attacker, ent_basics* victim) {
    if (victim && victim->type == zombie_type && victim->health > 0) {
        float victimDistance = v2fDist(attacker->pos, victim->pos);
        if (victimDistance > RSIZE*0.8) {
            return;
        }
        victim->health -= 1;
        ((struct ent_projectile*)attacker)->lifetime = 0;
    }
}

void projectileThink(struct ent_projectile* e) {
    e->lifetime -= 1;
    struct tile* curTile = 0;
    vec2f p = v2fAdd(e->pos, HW);
    p = v2fSub(p, (vec2f){RSIZE,RSIZE});
    nearbyEntInteractionBidirectional((ent_basics*)e, projectileHitNearby);
    curTile = worldTileFromPos(v2fAdd(e->pos, HW));
    if (e->lifetime <= 0 && !e->isExploding) {
        e->isExploding = 1;
        playSound(explosion01);
        e->sprites[0].anim = grenade01Explode;
        e->sprites[0].frame = 0;
        e->sprites[0].flags &= ~LOOPING;
        //vel = (vec2f) {0,0};
        e->lifetime = 500;
    }
    if (curTile != 0 && curTile->wall_height > 0 && !e->isExploding) {
        e->isExploding = 1;
        playSoundChannel(explosion01, CHAN_EXPLOSION);
        //playSound(chow);
        e->sprites[0].anim = grenade01Explode;
        e->sprites[0].frame = 3;
        e->sprites[0].flags &= ~LOOPING;
        e->vel = (vec2f) {0,0};
        e->lifetime = 500;
        if (curTile != 0 && curTile->wall_height > 0) {
            curTile->wall_height = 0;
            curTile->floor_anim = tileGold01;
        }
        return;
    }
    if (e->lifetime <= 0 || e->sprites[0].frame >= anim_data[grenade01Explode].len-1) {
        despawn_ent((ent_basics*)e);
    }
}

void rabbitInit(struct ent_rabbit* e) {                               // RABBIT
    e->health = 1;
    e->wanderDir = (vec2f){1,0};
    e->wanderWait = 100;
    e->num_sprites = 1;
    //sprites[0].flags |= LOOPING;
    e->sprites[0].anim = rabbitHop01;
    e->sprites[0].flags |= PAUSED;
    e->sprites[0].frame = 2;
}
void rabbitThink(struct ent_rabbit* e) {
    e->wanderWait -= 1;
    if (e->wanderWait <= 0) {
        e->wanderWait = 100;
        e->wanderDir = (vec2f){ (float)(rand()) / (float)(RAND_MAX) - 0.5f, (float)(rand()) / (float)RAND_MAX - 0.5f };
        e->vel = v2fScale(v2fNormalized(e->wanderDir), 800.f);
        e->sprites[0].rotation = atan2(e->wanderDir.y, e->wanderDir.x) * 180. / F_PI + 90;
        e->sprites[0].rotation = (float)((int)e->sprites[0].rotation % 360);
        e->sprites[0].frame = 0;
        e->sprites[0].flags &= ~PAUSED;
    }
}

void zombieInit(struct ent_zombie* e) {                               // ZOMBIE
    e->health = 1;
    e->wanderDir = (vec2f){1,0};
    e->wanderWait = 100;
    e->num_sprites = 1;
    e->sprites[0].flags |= LOOPING;
    e->sprites[0].anim = zombie;
    e->target = 1; // first entity handle should be the player TODO add a player-finding function for reliability! TODO
    e->targetPos = (vec2f) {0,0};
    e->walkDelay.interval = 20;
}
#define PUSH_FORCE 50
void pushNearbyEnts(ent_basics* me, ent_basics* them) {
    if (them->type != zombie_type)
        return;
    vec2f posDelta = v2fSub(me->pos, them->pos);
    float d = v2fLen(posDelta);
    if (d < RSIZE) {
        me->vel = v2fAdd(me->vel, v2fScale(posDelta, PUSH_FORCE*dt));
        them->vel = v2fSub(them->vel, v2fScale(posDelta, PUSH_FORCE*dt));
    }
}
#define DIVERSION_STRENGTH 64
void divertNearbyZombies(ent_basics* me, ent_basics* them) {
    if (them->type != zombie_type)
        return;
    struct ent_zombie* thisZombie = (struct ent_zombie*)me;
    struct ent_zombie* thatZombie = (struct ent_zombie*)them;
    vec2f posDelta = v2fSub(thisZombie->pos, thatZombie->pos);
    float d = v2fLen(posDelta)*2;
    if (d < RSIZE*2.5) {
        thisZombie->wanderDir = v2fAdd(thisZombie->wanderDir, v2fScale(posDelta, (DIVERSION_STRENGTH/(d+1))*DIVERSION_STRENGTH*dt));
        thatZombie->wanderDir = v2fSub(thatZombie->wanderDir, v2fScale(posDelta, (DIVERSION_STRENGTH/(d+1))*DIVERSION_STRENGTH*dt));
    }
}
#define MSIZE 1024
char message[MSIZE] = "Example message.... Greetings! Hello world! Goodbye world! Farewell world? Nice to meet you world? Oh well, see ya world!";
void zombieThink(struct ent_zombie* e) {
    if (e->health <= 0) {
        playSoundChannel(zombieDie01, CHAN_MONSTER);
        int numGibs = anim_data[zombieGibs].len;
        for (int i=0; i<numGibs; i++) {
            ent_basics* newGib = (ent_basics*)spawn(gib_type, e->pos);
            if (newGib) {
                ((struct ent_gib*)newGib)->sprites[0].rotation = e->sprites[0].rotation;
                ((struct ent_gib*)newGib)->sprites[0].anim = zombieGibs;
                ((struct ent_gib*)newGib)->sprites[0].frame = i;
                ((struct ent_gib*)newGib)->vel = (vec2f){randfn()*randfn()*16000,randfn()*randfn()*16000};
            }
        }
        despawn_ent((ent_basics*)e);
        return;
    }
    ent_basics* t = get_ent(e->target);
    if (t != 0 && v2fDist(t->pos, e->pos) < RSIZE/2 && playerClient.dialogVisible == 0) {
        clientLoadDialog((char*)"assets/worlds/testWorld/hello.txt");
        clientStartDialog(playerClient.loadedDialog);
    }
    e->wanderWait -= 1;
    counterInc(&e->walkDelay);
    if (e->walkDelay.count > 0) { //wanderWait <= 0 && e != 0) {
        e->walkDelay.count = 0;
        e->wanderWait = 10;
        //playSound(tik);
        //wanderDir = (vec2f){ (float)(rand()) / (float)(RAND_MAX) - 0.5f, (float)(rand()) / (float)RAND_MAX - 0.5f };
        vec2f targetVector = v2fNormalized(v2fSub(v2fAdd(e->targetPos, v2fScale(t->vel, 0.15f)), e->pos));
        e->wanderDir = targetVector;
        nearbyEntInteractionBidirectional((ent_basics*)e, divertNearbyZombies);
        e->vel = v2fAdd(e->vel, v2fScale(v2fNormalized(e->wanderDir), 130.f));
        //EVENT(EntMove, .pos=pos, .vel=wanderDir.normalized() * 400.f); ;;
        
        e->sprites[0].rotation = vectorToAngle(targetVector) + 270;//atan2(wanderDir.y, wanderDir.x) * 180. / F_PI + 270.0;
        e->sprites[0].rotation = (float)((int)e->sprites[0].rotation % 360);
        e->targetPos = t->pos;
    }
}
void gibInit(struct ent_gib* e) {
    e->num_sprites = 1;
    if (e->h % 8 == 0)
        e->flags |= NOFRICTION;
    e->sprites[0].anim = zombieGibs;
    e->sprites[0].flags |= PAUSED;
    e->spinRate.interval = 8;
    e->spinRate.count = GIB_SPIN_SPEED * randf()*randf();
}
void gibThink(struct ent_gib* e) {
    counterDec(&e->spinRate);
    e->sprites[0].rotation += (float)(e->spinRate.count*8*dt * (1-2*((e->h & 1) == 0)));
    if (e->spinRate.count < GIB_SPIN_SPEED*3/8)
        e->flags &= ~NOFRICTION;
    /*
    e->lifetime--;
    if (e->lifetime < 0)
        despawn_ent((ent_basics*)e);
    */
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
        int ent_size = ((ent_basics*)&array[i])->size;
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
void* spawn(int type, vec2f pos) { // Spawn an ent in the default entity array.
    if (mainWorld->entArraySpace < ENTITY_BYTES_ARRAY_LEN/8)
        printf("*** Warning! 7/8 of entity bytes array is full!\n");
    char* array = mainWorld->entity_bytes_array;
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
            int skip_bytes = ((ent_basics*)&array[i])->size;
            if (DEBUG_ENT_SPAWNING) { printf("Slots [%d, %d] already taken.\n", i, i+skip_bytes-1); }
            empty_space_len = 0;
            i += skip_bytes;
        }
        // Got enough space to store the ent.
        if (empty_space_len == required_space) {
            if (DEBUG_ENT_SPAWNING) { printf("Found enough space for ent in [%d, %d]\n", i-required_space, i-1); }
            i = i-required_space;
            mainWorld->entArraySpace += required_space;
            break;
        }
    }
    if (i >= array_len-1) {
        printf("***\n*** No space left in the entity array!!!\n***\n");
        //exit(-1);
    }
    // Initialize the entity's header info:
    ent_basics* new_entity = (ent_basics*)&array[i];
    new_entity->header_byte = HEADER_BYTE;
    new_entity->type = type;
    new_entity->size = required_space;
    new_entity->h = claim_handle((ent_basics*)&array[i]);
    new_entity->pos = pos;
    // Initialize the entity.
    switch (type) {
        #define ENT_INIT_CASES(name) case name##_type:  name##Init((struct ent_##name *)(&array[i])); break; //--- Init the entity.
        ENTITY_TYPES_LIST(ENT_INIT_CASES)
        default:
            printf("*** spawn_ent() error: invalid entity type: %d", type);
            exit(-1);
    }
    //printf("Spawning a '%s' at index %d.\n", get_type_name(type), i);
    if (type == gib_type)
        mainWorld->numGibs++;
    return &array[i];
}
// Remove an entity from an entity segment array. TODO ent-specific cleanup TODO
void despawn_ent(ent_basics* e) {
    struct tile* old_tile = &mainWorld->chunks[e->chunk.y][e->chunk.x].tiles[e->tile.y][e->tile.x];
    bool old_tile_was_valid = (e->tile.x > -1 && e->tile.x < CHUNK_WIDTH && e->tile.y > -1 && e->tile.y < CHUNK_WIDTH);
    for (int i=0; i<MAX_ENTS_PER_TILE; i++) {
        if (old_tile_was_valid && old_tile->ents[i] == e->h) { old_tile->ents[i] = 0; } //---- Remove handle from old tile.
    }
    unclaim_handle(e->h);
    int size = e->size;
    mainWorld->entArraySpace -= size;
    if (DEBUG_ENTS) { printf("Despawning ent of size %d\n", size); }
    if (e->type == gib_type)
        mainWorld->numGibs--;
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
        type = ((ent_basics*)&array[i])->type;
        //printf("Thinking entity type: '%s' at index %d.\n", get_type_name(type), i);
        switch (type) {
            // X macro for ENTITY_TYPES_LIST:
            #define ENT_THINK_CASES(name) case name##_type:  name##Think((struct ent_##name *)(&array[i])); break; 
            ENTITY_TYPES_LIST(ENT_THINK_CASES)
            default:
                printf("*** entity type '%d' at %d not recognized in think_all_ents().\n", type,  i);
                exit(-1);
        }
    }
}
void move_ent(ent_basics* e) { //------------ Update an ent's position based on its velocity:
    e->pos = v2fAdd(e->pos, v2fScale(e->vel, dt));
    // Apply friction:
    float speed = v2fLen(e->vel);
    float friction = speed*8*dt;
    int hasFriction = (e->flags & NOFRICTION) == 0;
    e->vel = v2fSub(e->vel, v2fScale(v2fNormalized(e->vel), friction*hasFriction));
    if (v2fLen(e->vel) < 5) { e->vel = (vec2f){0,0}; } // Minimum vel.
}

float MIN_SQUARE_DISTANCE = RSIZE;
void collide_wall(ent_basics* e) {
    vec2f* position = &e->pos;
    vec2f centered_position = v2fAdd(e->pos, (vec2f){RSIZE/2, RSIZE/2});
    vec2f nearest_corner = centered_position;
    nearest_corner = v2fScalarDiv(nearest_corner, RSIZE);
    nearest_corner = (vec2f){floor(nearest_corner.x + 0.5f), floor(nearest_corner.y + 0.5f)}; //--- Nearest corner.
    nearest_corner = v2fScale(nearest_corner, RSIZE);
    vec2f tile_pos;//------------------------------------------------------------------------------------- Adjacent tiles.
    vec2i tile_index;
    float sign_x = -1;
    float sign_y = -1;
    bool collisions[2][2];
    int num_collisions = 0;
     for (int i=0; i<2; i++) {
        for (int j=0; j<2; j++) {
            tile_pos = v2fAdd(nearest_corner, (vec2f){RSIZE/2*sign_x, RSIZE/2*sign_y});
            tile_index = v2fToI(v2fScalarDiv(tile_pos, RSIZE));  //vec2i{int(tile_pos.x/RSIZE), int(tile_pos.y/RSIZE)} % CHUNK_WIDTH;
            struct tile* cur_tile = worldGetTile(tile_index);
            if (cur_tile != 0 && cur_tile->wall_height >= 1)
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
            tile_pos = v2fAdd(nearest_corner, (vec2f){RSIZE/2*sign_x, RSIZE/2*sign_y});
            sign_x *= -1;
            tile_pos = v2fScalarDiv(tile_pos, RSIZE);
            tile_pos = v2fScale((vec2f){floor(tile_pos.x), floor(tile_pos.y)}, RSIZE);
            tile_index = (vec2i){(int)(tile_pos.x/RSIZE), (int)(tile_pos.y/RSIZE)};
            struct tile* cur_tile = worldGetTile(tile_index);
            if (cur_tile == 0 || cur_tile->wall_height <=0) { continue; } // Skip the tile.
            vec2f delta = v2fSub(*position, tile_pos);
            bool in_square = abs(delta.x) < MIN_SQUARE_DISTANCE && abs(delta.y) < MIN_SQUARE_DISTANCE;
            bool in_diamond = abs(delta.x) + abs(delta.y) > RSIZE*1.2;
            if (in_square) {
                float x_delta_sign = 1;
                float y_delta_sign = 1;
                if (delta.x < 0) x_delta_sign = -1;
                if (delta.y < 0) y_delta_sign = -1;
                if (in_diamond && num_collisions == 1) { //------------------------------------- Circle-style collision on tile corners.
                    if (v2fLen(delta) < RSIZE)
                        *position = v2fAdd(*position, v2fScale(v2fNormalized(delta), (RSIZE-v2fLen(delta))));
                }
                else if (abs(delta.x) > abs(delta.y) && !is_horizontal_pair) { //----------------- Square-style collision on tile sides.
                    *position = v2fAdd(*position, (vec2f){x_delta_sign*MIN_SQUARE_DISTANCE-delta.x, 0});
                }
                else if (abs(delta.x) < abs(delta.y) && !is_vertical_pair) {
                    *position = v2fAdd(*position, (vec2f){0, y_delta_sign*MIN_SQUARE_DISTANCE-delta.y});
                }
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
        ent_basics* e = ((ent_basics*)&array[i]);
        if ((e->flags & NOCOLLISION) != NOCOLLISION)
            collide_wall(e);
    }
}
void defragEntArray() {
    if (mainWorld->numGibs < MAX_GIBS) //(mainWorld->entArraySpace < ENTITY_BYTES_ARRAY_LEN/2)
            return;
    char* array = mainWorld->entity_bytes_array;
    int array_len = ENTITY_BYTES_ARRAY_LEN;
    for (int i=get_first_ent(array, array_len); i != -1; i=get_next_ent(i, array, array_len)) {
        ent_basics* e = ((ent_basics*)&array[i]);
        if (e && e->type == gib_type && rand() < RAND_MAX/32){ // Delete random gibs (not just the newest ones).
            despawn_ent(e);
        }
        if (mainWorld->numGibs < MAX_GIBS)
            break;;
    }
    //printf("Back to %d gibs.\n", mainWorld->numGibs);
}
