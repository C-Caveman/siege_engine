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
int countRemainingHandles() {
    int numAvailable = 0;
    for (int i=0; i<NUM_HANDLES; i++) {
        if (handles[i].copies == 0)
            numAvailable++;
    }
    return numAvailable;
}
handle claim_handle(entBasics* e, uint16_t entType) { //-------- Bind a handle to an entity.
    handle h = -1;
    for (int i=0; i<NUM_HANDLES; i++) {
        if (handles[i].copies == 0) {h=i; break;}
    }
    if (h == -1) { printf("\n*** Ran out of handles!!!\n"); exit(-1); }
    handles[h].copies = 1;
    handles[h].ent = e;
    handles[h].claimed = 1;
    handles[h].entType = entType;
    //printf("Handle %d claimed by a '%s' ent.\n", h, entTypeName(handles[h].ent->type));
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
entBasics*  getEnt(handle i, uint16_t entType) { //------------ Get an entity by its handle.
    if (handles[i].claimed == 1 && (handles[i].entType == entType || entType == 0))
        { return handles[i].ent; }
    else
        { handles[i].copies--; return 0; }
}//===============================================================================// ENTITY FUNCTIONS. //;;
void nearbyEntInteraction(vec2f position, void (*fn)(entBasics*)) {
    vec2f p = v2fSub(v2fAdd(position, HW), (vec2f){RSIZE,RSIZE}); // Top left corner of the 3x3.
    for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
            for (int k=0; k<MAX_ENTS_PER_TILE; k++) {
                struct tile* curTile = worldTileFromPos(v2fAdd(v2fAdd(p, v2fScale((vec2f){0,RSIZE}, (float)(i))), v2fScale((vec2f){RSIZE,0}, (float)(j))));
                if (curTile) {
                    entBasics* useTarget = getEnt(curTile->ents[k], 0);
                    if (useTarget) {
                        fn(useTarget);
                    }
                }
            }
        }
    }
}
void nearbyEntInteractionBidirectional(entBasics* user, void (*fn)(entBasics*, entBasics*)) {
    vec2f p = v2fSub(v2fAdd(user->pos, HW), (vec2f){RSIZE,RSIZE}); // Top left corner of the 3x3.
    for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
            for (int k=0; k<MAX_ENTS_PER_TILE; k++) {
                struct tile* curTile = worldTileFromPos(v2fAdd(v2fAdd(p, v2fScale((vec2f){0,RSIZE}, (float)(i))), v2fScale((vec2f){RSIZE,0}, (float)(j))));
                if (curTile) {
                    entBasics* useTarget = getEnt(curTile->ents[k], 0);
                    if (useTarget) {
                        fn(user, useTarget);
                    }
                }
            }
        }
    }
}
struct eventsBuffer events;
#define TO_EVENT_NAMES(name, detailsUnused) #name,
char eventNames[NUM_EVENTS][64] = {
    EVENT_LIST(TO_EVENT_NAMES)
};
char* eventName(int eventType) {
    if (eventType > 0 && eventType < NUM_EVENTS)
        return &eventNames[eventType][0];
    else
        return "Invalid event!";
}
#define TO_EVENT_CASE(name, detailsUnused) case event##name: ev##name(&ev->data.det##name); break;
void applyEvent(struct event* ev) {
    switch(ev->type) {
        EVENT_LIST(TO_EVENT_CASE)
    };
}
void takeEvent() {
    if (events.count <= 0)
        return;
    //printf("Event: %d '%s'\n", events.buffer[events.count-1].type, eventName(events.buffer[events.count-1].type));
    applyEvent(&events.buffer[events.count-1]);
    memset((void*)&events.buffer[events.count-1], 0, sizeof(events.buffer[0]));
    events.count--;
}
void sendEvents(struct eventsBuffer* eBuff) {
    //TODO send the events!!!!
}
void evPlayerMove(struct dPlayerMove* d) {
    struct ent_player* p = (struct ent_player*)getEnt(d->p, player_type);
    if (p == 0)
        return;
    p->pos = d->pos;
    p->vel = d->vel;
}
void evPlayerShoot(struct dPlayerShoot* d) {
    struct ent_player* p = (struct ent_player*)getEnt(d->p, player_type);
    // Are we missing a player/client?
    if (!p || !p->cl)
        return;
    p->cl->lastAttackTime = curFrameStart;
    p->cl->player->sprites[PLAYER_GUN].frame = 0;;
    p->cl->player->sprites[PLAYER_GUN].flags &= ~PAUSED;
    //playSound(bam02);
    playSoundChannel(bam02, CHAN_WEAPON);
    void* e = spawn(projectile_type, (vec2f){0,0});
    vec2f aimDir = angleToVector(d->shootDir);
    ((entBasics*)e)->pos = v2fAdd(d->shootPos, v2fScale(aimDir, RSIZE/2));
    ((entBasics*)e)->tile = v2iScalarDiv(v2fToI(((entBasics*)e)->pos), RSIZE);
    ((entBasics*)e)->vel = v2fScale(aimDir, 800);
}
void evEntMove(struct dEntMove* d) {
    entBasics* e = getEnt(d->h, 0);
    if (!e)
        return;
    e->pos = d->pos;
    e->vel = d->vel;
}
void evTriggerDialog(struct dTriggerDialog* d) {
    struct ent_player* e = (struct ent_player*)getEnt(d->p, player_type);
    if (!e)
        return;
    #define DIALOG_FILE_NAME_SIZE 64
    char dialogFileName[DIALOG_FILE_NAME_SIZE] = {0};
    snprintf(dialogFileName, DIALOG_FILE_NAME_SIZE, "assets/worlds/%.*s.txt", (int)sizeof(d->fileName), d->fileName);
    clientLoadDialog(dialogFileName);
    clientStartDialog(playerClient.loadedDialog);
}
void evSpriteRotate(struct dSpriteRotate* d) {
    entBasics* e = getEnt(d->h, 0);
    if (!e || e->num_sprites <= d->index || d->index < 0)
        return;
    struct sprite* sprites = (struct sprite*)( (char*)e+sizeof(entBasics) );
    sprites[d->index].rotation = d->angle;
}
void evEntSpawn(struct dEntSpawn* d) {
    spawn(d->entType, d->pos);
}
handle findPlayer() { // first entity handle should be the player TODO add a player_type ent search for reliability! TODO
    return 1;
}
void playerInit(struct ent_player* e) {
    if (DEBUG_ENTS) { printf("Player entity initializing!\n"); }
    e->health = 1;
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
void playerInteract(entBasics* player, entBasics* useTarget) {
    struct ent_player* user = (struct ent_player*)player;
    if (useTarget && useTarget->type == scenery_type && user->cl && user->cl->interacting) {
        printf("Used a scenery ent!\n");
        user->cl->interacting = false;
        E(TriggerDialog, .p=user->h, .fileName="test/piggy");
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
void windShieldSplatter(entBasics* attacker, entBasics* victim) {
    if (victim == 0 || attacker == 0)
        return;
    if (victim->type == zombie_type && v2fDist(victim->pos, attacker->pos) < RSIZE*2)
        E(ZombieWindShieldSplatter, victim->h);
}

#define HEAT_UPDATE_DELAY_MILLIS 10
void playerThink(struct ent_player* e) {                              // PLAYER
    // Debug commands:
    if (playerClient.zombieSpawning && mainWorld->entArraySpace > ENTITY_BYTES_ARRAY_LEN/8 && countRemainingHandles() > 10) {
        vec2f spawnPos = v2fAdd(playerClient.camera_center, v2iToF(playerClient.aim_pixel_pos));
        E(EntSpawn, zombie_type, spawnPos);
        //spawn(zombie_type, v2fAdd(playerClient.camera_center, v2iToF(playerClient.aim_pixel_pos)));
    }
    if (playerClient.explodingEverything) {
        playerClient.explodingEverything = false;
        for (int i=getFirstEnt(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN); i != -1; i=getNextEnt(i, mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN)) {
            if (mainWorld->entity_bytes_array[i] != HEADER_BYTE) {
                if (DEBUG_ENTS) { printf("*** Invalid index given by getNextEnt() in thinkAllEnts()\n"); }
                break;
            }
            // Run the correct think function for this entity:
            entBasics* e = (entBasics*)&mainWorld->entity_bytes_array[i];
            if (e->type == zombie_type && v2fDist(playerClient.player->pos, e->pos) < RSIZE*10)
                E(ZombieDie, e->h);
        }
    }
    // Get the heat value:
    timerUpdate(&e->heatTimer, HEAT_UPDATE_DELAY_MILLIS);
    int heat = e->heat;
    if (playerClient.dashing)
        heat = e->heat + e->heatTimer.count;
    else
        heat = e->heat - e->heatTimer.count;
    heat = iclamp(heat, 0, HEAT_MAX);
    e->heatTracker = heat; // When the boost key is pressed/released, e->heatTracker becomes the new e->heat value.
    // Heat value obtained, now do stuff with it:
    if (heat == HEAT_MAX) {
        nearbyEntInteractionBidirectional((entBasics*)e, windShieldSplatter);
    }
    if (playerClient.dashing) {
        e->sprites[PLAYER_FLAMES].flags &= ~INVISIBLE;
    }
    else {
        e->sprites[PLAYER_FLAMES].flags |= INVISIBLE;
        e->sprites[PLAYER_FLAMES_EXTRA].flags |= INVISIBLE;
    }
    if (playerClient.dashing && heat < HEAT_MAX && !isChannelPlaying(CHAN_ENGINE)) {
        playSoundChannel(rocketEngineLoop, CHAN_ENGINE);
    }
    else if (playerClient.dashing && heat == HEAT_MAX && !isChannelPlaying(CHAN_ENGINE)) {
        playSoundChannel(rocketEngineLoopFast, CHAN_ENGINE);
    }
    if (heat > 0) {
        if (heat == HEAT_MAX && e->sprites[PLAYER_GUN].frame < anim_data[gunGrenadeBoost].len-1) {
            playSoundChannel(rocketBoostEngage, CHAN_WEAPON_ALT);
            playSoundChannel(rocketEngineLoopFast, CHAN_ENGINE);
            e->sprites[PLAYER_FLAMES_EXTRA].flags &= ~INVISIBLE;
        }
        e->sprites[PLAYER_GUN].anim = gunGrenadeBoost;
        e->sprites[PLAYER_GUN].flags |= PAUSED;
        e->sprites[PLAYER_GUN].frame = (int)((float)heat/(float)(HEAT_MAX)*(float)(anim_data[gunGrenadeBoost].len-1));
        e->sprites[PLAYER_BODY].frame = (int)((float)heat/(float)(HEAT_MAX)*(float)(anim_data[vTankBody03].len-1));
        e->sprites[PLAYER_CROSSHAIR].frame = (int)((float)heat/(float)(HEAT_MAX)*(float)(anim_data[crosshair01].len-1));
    }
    else if (!playerClient.dashing && e->sprites[PLAYER_GUN].anim == gunGrenadeBoost && heat <= 0) {
        e->sprites[PLAYER_GUN].anim = gunGrenadeRetract;
        e->sprites[PLAYER_GUN].frame = anim_data[gunGrenadeRetract].len-1;
        playSoundChannel(rocketBoostEnd, CHAN_WEAPON_ALT);
        e->sprites[PLAYER_CROSSHAIR].frame = 0;
    }
    // Crosshair things:
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
    // Interacting with the 'e' key:
    if (playerClient.interacting) {
        nearbyEntInteractionBidirectional((entBasics*)e, playerInteract);
    }
    
    
    
}
// Client-side animation:
void playerAnim(struct ent_player* e) {}

void sceneryInit(struct ent_scenery* e) {                              // SCENERY
    if (DEBUG_ENTS)
        printf("Scenery ent initializing!\n");
    e->flags |= NOTHINK;
    e->num_sprites = NUM_SCENERY_SPRITES;
    e->sprites[SCENERY_SPRITE_1].anim = rocket_tank;
}
void sceneryThink(struct ent_scenery* e) {
    //entBasics* e = getEnt(fren);
    //if (e != nullptr) { vel = vel + ( e->pos-pos ).normalized() * 15; } //- Follow the player.
    //vel = vel * 1.1; //---------------------------------------------------- Slip around.
}
void sceneryAnim(struct ent_scenery* e) {}

void projectileInit(struct ent_projectile* e) {                           // PROJECTILE
    e->num_sprites = 1;
    e->health = 1;
    e->sprites[0].anim = grenade01Blink;
    e->sprites[0].flags |= LOOPING;
    e->flags = NOFRICTION | NOCOLLISION;
    e->timeOut = curFrameStart + 2000;
    e->isExploding = 0;
}
void projectileHitNearby(entBasics* attacker, entBasics* victim) {
    if (victim && victim->type == zombie_type && victim->health > 0) {
        float victimDistance = v2fDist(attacker->pos, victim->pos);
        if (victimDistance > RSIZE*0.8) {
            return;
        }
        victim->health -= 1;
        ((struct ent_projectile*)attacker)->timeOut = curFrameStart;
    }
}
void evChangeTile(struct dChangeTile* d) {
    struct tile* t = worldGetTile(tileNumberToIndex(d->tileNumber));
    if (t) {
        t->wall_height = d->height;
        t->wall_top_anim = d->wall;
        t->wall_side_anim = d->wallSide;
        t->floor_anim = d->floor;
    }
}
void projectileThink(struct ent_projectile* e) {
    struct tile* curTile = 0;
    vec2f p = v2fAdd(e->pos, HW);
    nearbyEntInteractionBidirectional((entBasics*)e, projectileHitNearby);
    curTile = worldTileFromPos(p);
    if (passedTimestamp(e->timeOut) && !e->isExploding) {
        e->isExploding = 1;
        playSound(explosion01);
        e->sprites[0].anim = grenade01Explode;
        e->sprites[0].frame = 0;
        e->sprites[0].flags &= ~LOOPING;
        //vel = (vec2f) {0,0};
        e->timeOut = curFrameStart + 500;
    }
    if (curTile != 0 && curTile->wall_height > 0 && !e->isExploding) {
        e->isExploding = 1;
        playSoundChannel(explosion01, CHAN_EXPLOSION);
        //playSound(chow);
        e->sprites[0].anim = grenade01Explode;
        e->sprites[0].frame = 3;
        e->sprites[0].flags &= ~LOOPING;
        e->vel = (vec2f) {0,0};
        e->timeOut = curFrameStart + 500;
        E(ChangeTile, .tileNumber=tileIndexToNumber(v2fToI(v2fScalarDiv(p, RSIZE))), .floor=tileGold01, .height=0, .wall=tiledark, .wallSide=tiledark);
        return;
    }
    if (passedTimestamp(e->timeOut) || e->sprites[0].frame >= anim_data[grenade01Explode].len-1) {
        despawnEnt((entBasics*)e);
    }
}
void projectileAnim(struct ent_projectile* e) {}

void rabbitInit(struct ent_rabbit* e) {                               // RABBIT
    e->num_sprites = 1;
    e->health = 1;
    e->wanderDir = (vec2f){1,0};
    //sprites[0].flags |= LOOPING;
    e->sprites[0].anim = rabbitHop01;
    e->sprites[0].flags |= PAUSED;
    e->sprites[0].frame = 2;
    e->target = findPlayer();
}
#define RABBIT_IGNORE_DIST RSIZE*10
void rabbitThink(struct ent_rabbit* e) {
    e->nextThink = curFrameStart + 1500;
    vec2f targetPos = e->pos;
    entBasics* t = getEnt(e->target, player_type);
    if (t)
        targetPos = t->pos;
    if (t && v2fDist(targetPos, e->pos) < RABBIT_IGNORE_DIST) {
        playSoundChannel(fft, CHAN_MONSTER);
        e->wanderDir = v2fSub(targetPos, e->pos);
    }
    else {
        e->wanderDir = v2fAdd(v2fNormalized(v2fSub(targetPos, e->pos)), (vec2f){randfn(), randfn()});
    }
    e->vel = v2fScale(v2fNormalized(e->wanderDir), 800.f);
    e->sprites[0].rotation = vectorToAngle(e->wanderDir) + 90;//atan2(e->wanderDir.y, e->wanderDir.x) * 180. / F_PI + 90;
    e->sprites[0].rotation = (float)((int)e->sprites[0].rotation % 360);
    e->sprites[0].frame = 0;
    e->sprites[0].flags &= ~PAUSED;
}
float closestAngleDelta(float a, float b) {
    float angleDelta = b - a;
    float wrapDelta = b - (a+360);
    if (abs(wrapDelta) < abs(angleDelta))
        angleDelta = wrapDelta;
    return angleDelta;
}
float lerpAngle(float a, float b, float t) {
    t = fclamp(t, 0, 1);
    return a + closestAngleDelta(a,b)*t;
}
void rabbitAnim(struct ent_rabbit* e) {
    vec2f targetPos = {0,0};
    entBasics* t = getEnt(e->target, player_type);
    if (t)
        targetPos = t->pos;
    bool aboutToHop = (e->nextThink-curFrameStart) < 1000; // 1/3 of a second before hop.
    // Wiggle before hop:
    if (aboutToHop)
        e->sprites[0].rotation += sin(curFrameStart*0.02f)*150.f*dt;
    // Look at player:
    float a = vectorToAngle(v2fSub(targetPos, e->pos)) + 90;
    float b = e->sprites[0].rotation;
    if (t && v2fDist(targetPos, e->pos) < RABBIT_IGNORE_DIST) {
        e->sprites[0].rotation -= closestAngleDelta(a,b)*4.f*dt;
        //e->sprites[0].rotation = lerpAngle(a, b, (float)(e->nextThink-curFrameStart)/1500.f);
    }
    if (e->sprites[0].rotation > 360)
        e->sprites[0].rotation -= 360;
    if (e->sprites[0].rotation < 0)
        e->sprites[0].rotation += 360;
}

void zombieInit(struct ent_zombie* e) {                               // ZOMBIE
    e->health = 1;
    e->wanderDir = (vec2f){1,0};
    e->speed = 150.f + 200.f*randf();
    e->num_sprites = 1;
    e->nextWalk = curFrameStart;
    e->sprites[0].flags |= LOOPING;
    e->sprites[0].anim = zombie;
    e->target = findPlayer(); 
    e->targetPos = (vec2f) {0,0};
}
#define PUSH_FORCE 50
void pushNearbyEnts(entBasics* me, entBasics* them) {
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
void divertNearbyZombies(entBasics* me, entBasics* them) {
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
// A blank event (type zero):
void evInvalid(struct dInvalid* d) {}
// Markers for the start/end of a server frame:
void evFrameStart(struct dFrameStart* d) {}
void evFrameEnd(struct dFrameEnd* d) {}
#define GIB_SPEED 4000
void evZombieDie(struct dZombieDie* d) {
    struct ent_zombie* e = (struct ent_zombie*)getEnt(d->h, zombie_type);
    if (!e)
        return;
    playSoundChannel(zombieDie01, CHAN_MONSTER);
    int numGibs = anim_data[zombieGibs].len;
    for (int i=0; i<numGibs; i++) {
        entBasics* newGib = (entBasics*)spawn(gib_type, e->pos);
        if (newGib) {
            ((struct ent_gib*)newGib)->sprites[0].rotation = e->sprites[0].rotation;
            ((struct ent_gib*)newGib)->sprites[0].anim = zombieGibs;
            ((struct ent_gib*)newGib)->sprites[0].frame = i;
            ((struct ent_gib*)newGib)->vel = (vec2f){randfn()*randfn()*GIB_SPEED,randfn()*randfn()*GIB_SPEED}; //TODO ensure handles are not desynced in client/server!
        }
    }
    despawnEnt((entBasics*)e);
}
#define SPLATTER_FORCE 6000
#define SCATTER_FORCE 0.8
void evZombieWindShieldSplatter(struct dZombieWindShieldSplatter* d) {
    struct ent_zombie* e = (struct ent_zombie*)getEnt(d->h, zombie_type);
    if (!e)
        return;
    playSoundChannel(zombieDie01, CHAN_MONSTER);
    int numGibs = anim_data[zombieGibs].len;
    vec2f splatterDir = v2fNormalized(v2fAdd(v2fScale(v2fNormalized(playerClient.player->vel),-1), (vec2f){randfn()*SCATTER_FORCE,randfn()*SCATTER_FORCE}));
    for (int i=0; i<numGibs; i++) {
        entBasics* newGib = (entBasics*)spawn(gib_type, e->pos);
        if (newGib) {
            ((struct ent_gib*)newGib)->sprites[0].rotation = e->sprites[0].rotation;
            ((struct ent_gib*)newGib)->sprites[0].anim = zombieGibs;
            ((struct ent_gib*)newGib)->sprites[0].frame = i;
            ((struct ent_gib*)newGib)->vel = v2fAdd(v2fScale(splatterDir, SPLATTER_FORCE*(randfns()+0.25f)), playerClient.player->vel);
        }
    }
    despawnEnt((entBasics*)e);
}
void zombieThink(struct ent_zombie* e) {
    e->nextThink = curFrameStart + 40;
    if (e->health <= 0) {
        E(ZombieDie, e->h);
        return;
    }
    entBasics* t = getEnt(e->target, player_type);
    bool attacking = (t != 0 && v2fDist(t->pos, e->pos) < RSIZE/2);
    if (attacking) {
        playSoundChannel(slice001, CHAN_MONSTER);
        e->nextThink = curFrameStart + 500;
    }
    vec2f targetVector = v2fNormalized(v2fSub(v2fAdd(e->targetPos, v2fScale(t->vel, 0.15f)), e->pos));
    if (passedTimestamp(e->nextWalk)) { //e->walkDelay.count > 0) {
        e->nextWalk = curFrameStart + 40;
        e->wanderDir = targetVector;
        nearbyEntInteractionBidirectional((entBasics*)e, divertNearbyZombies);
        vec2f persuitVelocity = v2fAdd(e->vel, v2fScale(v2fNormalized(e->wanderDir), e->speed));
        E(EntMove, .h=e->h, .pos=e->pos, .vel=persuitVelocity);;;
        e->targetPos = t->pos;
    }
}
void zombieAnim(struct ent_zombie* e) {
    // Face the player:
    entBasics* t = getEnt(e->target, player_type);
    vec2f targetVector = v2fNormalized(v2fSub(v2fAdd(e->targetPos, v2fScale(t->vel, 0.15f)), e->pos));
    float angleToPlayer = vectorToAngle(targetVector) + 270;
    if (angleToPlayer > 360.f)
        angleToPlayer -= 360.f;
    e->sprites[ZOMBIE_SPRITE_1].rotation = angleToPlayer;
}

void gibInit(struct ent_gib* e) {
    e->num_sprites = 1;
    e->flags |= NOTHINK;
    if (e->h % 8 == 0)
        e->vel = v2fScale(e->vel, 4.f); // 1/8 chance to quadruple velocity!
    e->spinMultiplier = randf() * randfn() * 20;
    e->sprites[0].anim = zombieGibs;
    e->sprites[0].flags |= PAUSED;
}
void gibThink(struct ent_gib* e) {}
void gibAnim(struct ent_gib* e) {
    float spinRate = v2fLen(e->vel) * e->spinMultiplier;
    //float spinDir = 1 - 2*((e->h & 1) == 0);
    e->sprites[0].rotation += (float)(spinRate*dt);
    if (v2fLen(e->vel) < 10.f) {
        e->flags |= NO_ANIMATION;
    }
}
#define SPAWN_INTERVAL 1000
#define MAX_SPAWNS 10
void spawnerInit(struct ent_spawner* e) {
    e->num_sprites = 1;
    e->sprites[0].anim = spawner001;
    e->sprites[0].flags |= LOOPING;
    e->nextThink = curFrameStart + SPAWN_INTERVAL;
}
void spawnerThink(struct ent_spawner* e) {
    e->nextThink = curFrameStart + SPAWN_INTERVAL;
    E(EntSpawn, zombie_type, e->pos);
    e->numSpawns += 1;
    if (e->numSpawns >= MAX_SPAWNS) {
        e->flags |= NOTHINK;
        e->sprites[0].flags &= ~LOOPING;
    }
}
void spawnerAnim(struct ent_spawner* e) {}

//======================================================================================================================//
//=====================================================// Entity management functions. (spawn, despawn, get_next, ect.) //
// X macro for ENTITY_TYPES_LIST:
char entity_type_names[NUM_ENT_TYPES][MAX_ENTITY_TYPE_NAME_LEN] = {
    ENTITY_TYPES_LIST(TO_STRING)
};
// Return the name for a given entity type.
char* entTypeName(int entType) {
    if (entType < 0 || entType > NUM_ENT_TYPES-1)
        return "*** invalid entType";
    return entity_type_names[entType];
}
// Return the size of the current entity type (in segments).
int getEntSize(int type) {
    int size = -1;
    switch (type) {
    // X macro for ENTITY_TYPES_LIST:
    #define GET_ENT_SIZES(name) case name##_type:  size = sizeof(struct ent_##name); break; 
    ENTITY_TYPES_LIST(GET_ENT_SIZES)
    default:
        printf("*** Unknown entity type in getEntSize()\n");
        exit(-1);
    }
    return size;
}
// Return index of the next entity's first byte. Returns -1 if there are no more entities.
int getNextEnt(int i, char* array, int array_len) {
    if (array[i] == HEADER_BYTE) {
        int ent_size = ((entBasics*)&array[i])->size;
        i += ent_size; // Skip past this entity.
        if (DEBUG_ENTS) { printf("getNextEnt() entity at %d. Size is %d.\n", i, ent_size); }
    }
    while (array[i] != HEADER_BYTE && i < array_len) { // Increment i until reaching the next entity.
        i += 1;
        if (DEBUG_ENTS) { printf("getNextEnt() skipping past %d.\n", i); }
    }
    if (i >= array_len-1) // Out of bounds.
        i = -1;
    return i;
}
// Return index of the first entity in a segment array. Return -1 if no entities are found.
int getFirstEnt(char* array, int array_len) {
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
handle reserveEntHandle(uint16_t entType) {
    handle h = 0;
    // Make sure there are handles left:
    int remainingHandles = countRemainingHandles();
    if (remainingHandles == 0) {
        printf("***\n*** No entity handles left!!!\n***\n");
        return 0;
    }
    else if (remainingHandles < 10 && entType == gib_type) {
        printf("!!! Only %d handles left!!! Skipping gib spawn.\n", remainingHandles);
        return 0;
    }
    // Locate memory for the handle to map to:
    if (mainWorld->entArraySpace < ENTITY_BYTES_ARRAY_LEN/8)
        printf("*** Warning! 7/8 of entity bytes array are full!\n");
    char* array = mainWorld->entity_bytes_array;
    int required_space = getEntSize(entType);
    int empty_space_len = 0;
    int i = 0;
    while (i<ENTITY_BYTES_ARRAY_LEN) {
        // Empty slot?
        if (array[i] != HEADER_BYTE) {
            if (DEBUG_ENT_SPAWNING) { printf("Found an open slot at %d.\n", i); }
            empty_space_len += 1;
            i += 1;
        }
        // Slot occupied.
        else {
            int skip_bytes = ((entBasics*)&array[i])->size;
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
    if (i >= ENTITY_BYTES_ARRAY_LEN-1) {
        printf("***\n*** No space left in the entity array!!!\n***\n");
        return 0;
    }
    // A handle is available and we have enough memory for its entity type.
    h = claim_handle((entBasics*)&array[i], entType);
    return h;
}
void updateEntCount(uint16_t entType, int n) {
    switch (entType) {
        case gib_type:
            mainWorld->numGibs += n;
            break;
        case zombie_type:
            mainWorld->numZombies += n;
            break;
    }
}
//TODO make a forceSpawn() function for the client to delete anything occupying the assigned byte array space
entBasics* spawnEnt(int entType, vec2f pos, handle h) {
    // Reserve a handle first (so the server can tell the clients where to store their entity):
    if (!h || h >= NUM_HANDLES-1) {
        printf("*** Null handle to spawn '%s' in spawnEnt()\n", entTypeName(entType));
        return 0;
    }
    if (handles[h].claimed && handles[h].entType != entType) {
        printf("*** Handle %d for spawning '%s' already claimed by a '%s' in spawnEnt()\n", h, entTypeName(entType), entTypeName(handles[h].entType));
        return 0;
    }
    // Initialize the entity in the reserved memory location:
    entBasics* new_entity = (entBasics*)handles[h].ent;
    new_entity->header_byte = HEADER_BYTE;
    new_entity->type = entType;
    new_entity->size = getEntSize(entType);
    new_entity->h = h;
    new_entity->pos = pos;
    // Call the associated init function for entType:
    switch (entType) {
        #define ENT_INITIALIZERS(name) case name##_type:  name##Init((struct ent_##name *)(new_entity)); break; //--- Init the entity.
        ENTITY_TYPES_LIST(ENT_INITIALIZERS)
        default:
            printf("*** spawnEnt() error: invalid entity type: %d", entType);
            exit(-1);
    }
    //printf("Spawning a '%s' at index %d.\n", entTypeName(entType), i);
    updateEntCount(entType, +1);
    return new_entity;
}
entBasics* findEntSpace(uint16_t entType) {
    char* array = mainWorld->entity_bytes_array;
    int required_space = getEntSize(entType);
    int empty_space_len = 0;
    int i = 0;
    if (mainWorld->entArraySpace < ENTITY_BYTES_ARRAY_LEN/8)
        printf("*** Warning! 7/8 of entity bytes array are full!\n");
    while (i<ENTITY_BYTES_ARRAY_LEN) {
        // Empty slot?
        if (array[i] != HEADER_BYTE) {
            if (DEBUG_ENT_SPAWNING) { printf("Found an open slot at %d.\n", i); }
            empty_space_len += 1;
            i += 1;
        }
        // Slot occupied.
        else {
            int skip_bytes = ((entBasics*)&array[i])->size;
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
    if (i >= ENTITY_BYTES_ARRAY_LEN-1) {
        printf("***\n*** No space left in the entity array!!!\n***\n");
        return 0;
        //exit(-1);
    }
    return (entBasics*)&array[i];
}
//  ;;;
// Clientside ent spawning:
void forceSpawn(uint16_t entType, vec2f pos, handle h) {
    // If this handle is taken by another type of ent, kill it.
    if (handles[h].claimed && handles[h].entType != entType) {
        despawnEnt(handles[h].ent);
        handles[h].claimed = true;
        handles[h].entType = entType;
        spawnEnt(entType, pos, h);
    }
}
void* spawn(int type, vec2f pos) { // Spawn an ent in the default entity array.
    entBasics* entData = findEntSpace(type);
    int remainingHandles = countRemainingHandles();
    if (remainingHandles == 0) {
        printf("***\n*** No entity handles left!!!\n***\n");
        return 0;
    }
    else if (remainingHandles < 10 && type == gib_type) {
        printf("!!! Only %d handles left!!! Skipping gib spawn.\n", remainingHandles);
        return 0;
    }
    // Initialize the entity's header info:
    entBasics* new_entity = entData;
    new_entity->header_byte = HEADER_BYTE;
    new_entity->type = type;
    new_entity->size = getEntSize(type);
    new_entity->h = claim_handle(entData, (uint16_t)type);
    new_entity->pos = pos;
    // Initialize the entity.
    switch (type) {
        #define ENT_INIT_CASES(name) case name##_type:  name##Init((struct ent_##name *)(entData)); break; //--- Init the entity.
        ENTITY_TYPES_LIST(ENT_INIT_CASES)
        default:
            printf("*** spawn_ent() error: invalid entity type: %d", type);
            exit(-1);
    }
    //printf("Spawning a '%s' at index %d.\n", entTypeName(type), i);
    updateEntCount(type, +1);
    return entData;
}
// Remove an entity from an entity segment array. TODO ent-specific cleanup TODO
void despawnEnt(entBasics* e) {
    struct tile* old_tile = &mainWorld->chunks[e->chunk.y][e->chunk.x].tiles[e->tile.y][e->tile.x];
    // Make sure the old_tile is in bounds:
    bool old_tile_was_valid = v2iInBounds(e->tile, 0, CHUNK_WIDTH) && v2iInBounds(e->chunk, 0, WORLD_WIDTH);
    for (int i=0; i<MAX_ENTS_PER_TILE; i++) {
        if (old_tile_was_valid && old_tile->ents[i] == e->h) { old_tile->ents[i] = 0; } //---- Remove handle from old tile.
    }
    unclaim_handle(e->h);
    int size = e->size;
    mainWorld->entArraySpace -= size;
    if (DEBUG_ENTS) { printf("Despawning ent of size %d\n", size); }
    updateEntCount(e->type, -1);
    memset((void*)e, 0, size*sizeof(char));
}
// Run the think() function for each entitiy in a segment array.
void thinkAllEnts(char* array, int array_len) {
    for (int i=getFirstEnt(array, array_len); i != -1; i=getNextEnt(i, array, array_len)) {
        if (array[i] != HEADER_BYTE) {
            if (DEBUG_ENTS) { printf("*** Invalid index given by getNextEnt() in thinkAllEnts()\n"); }
            break;
        }
        // Run the correct think function for this entity:
        entBasics* e = (entBasics*)&array[i];
        if (curFrameStart < e->nextThink || e->flags & NOTHINK)
            continue;
        //printf("Thinking entity type: '%s' at index %d.\n", entTypeName(type), i);
        switch (e->type) {
            // X macro for ENTITY_TYPES_LIST:
            #define ENT_THINK_CASES(name) case name##_type:  name##Think((struct ent_##name *)(&array[i])); break; 
            ENTITY_TYPES_LIST(ENT_THINK_CASES)
            default:
                printf("*** entity type '%d' at %d not recognized in thinkAllEnts().\n", e->type,  i);
                exit(-1);
        }
    }
}
void animateAllEnts(char* array, int array_len) {
    for (int i=getFirstEnt(array, array_len); i != -1; i=getNextEnt(i, array, array_len)) {
        if (array[i] != HEADER_BYTE) {
            if (DEBUG_ENTS) { printf("*** Invalid index given by getNextEnt() in animateAllEnts()\n"); }
            break;
        }
        // Run the correct animation function for this entity:
        entBasics* e = (entBasics*)&array[i];
        if (e->flags & NO_ANIMATION)
            continue;
        switch (e->type) {
            // X macro for ENTITY_TYPES_LIST:
            #define ENT_ANIM_CASES(name) case name##_type:  name##Anim((struct ent_##name *)(&array[i])); break; 
            ENTITY_TYPES_LIST(ENT_ANIM_CASES)
            default:
                printf("*** entity type '%d' at %d not recognized in animateAllEnts().\n", e->type,  i);
                exit(-1);
        }
    }
}
/*
> Save the current pos.
> Update the position.
> Check if we entered a new tile.
    > Move our handle from the old to the new tile.
> Check if we entered a new chunk.
    > Update our chunk.
*/
void move_all_ents(char* array, int array_len) {
    entBasics* e;
    for (int i=getFirstEnt(array, array_len); i != -1; i=getNextEnt(i, array, array_len)) {
        if (array[i] != HEADER_BYTE) { printf("*** Invalid index given by getNextEnt() in move_all_ents()\n"); exit(-1); }
                                                                                    //- move the entity, record its position in the chunk
        e = (entBasics*)&array[i];
        vec2i old_tile = e->tile;                                                   //- Old tile.
        vec2i old_chunk = e->chunk;                                                 //- Old chunk.
        moveEnt(e);
        e->chunk = v2fToI(v2fScalarDiv( v2fAdd(e->pos,(vec2f){RSIZE/2,RSIZE/2}), (RSIZE*CHUNK_WIDTH) ));
        vec2f floored = v2fSub(e->pos, v2iToF(v2iScale(e->chunk, RSIZE*CHUNK_WIDTH)));
        e->tile = v2fToI(v2fAdd(v2fScalarDiv(floored, RSIZE), (vec2f){0.5,0.5}));
        bool changed_tile = !v2iIsEq(e->tile, old_tile);                                  //- New tile?
        bool old_tile_was_valid = v2iInBounds(old_tile, 0, CHUNK_WIDTH);
        bool new_tile_was_valid = v2iInBounds(e->tile, 0, CHUNK_WIDTH);
        bool old_chunk_was_valid = v2iInBounds(old_chunk, 0, WORLD_WIDTH) && old_tile_was_valid;
        bool new_chunk_was_valid = v2iInBounds(e->chunk, 0, WORLD_WIDTH) && new_tile_was_valid;
        struct tile* old_tile_ptr = &mainWorld->chunks[old_chunk.y][old_chunk.x].tiles[old_tile.y][old_tile.x];
        struct tile* new_tile_ptr = &mainWorld->chunks[e->chunk.y][e->chunk.x].tiles[e->tile.y][e->tile.x];
        if (changed_tile) {
            if (old_chunk_was_valid)
            for (int i=0; i<MAX_ENTS_PER_TILE; i++) {                               //- Remove handle from old tile.
                if (old_tile_ptr->ents[i] == e->h)
                    old_tile_ptr->ents[i] = 0; /* old_tile_ptr->floor_anim = stonedk; */
            }
            int numGibsInTile = 0;
            if (new_chunk_was_valid)
                for (int i=0; i<MAX_ENTS_PER_TILE; i++) {
                    entBasics* tileEnt = getEnt(new_tile_ptr->ents[i], 0);
                    numGibsInTile += (tileEnt && tileEnt->type == gib_type);
                }
            bool tooManyGibs = (numGibsInTile > MAX_ENTS_PER_TILE*3/4);
            if (new_chunk_was_valid) {
                entBasics* firstTileEnt = getEnt(new_tile_ptr->ents[0], 0);
                if (e->type != gib_type && firstTileEnt && firstTileEnt->type == gib_type && numGibsInTile > 0) {
                    new_tile_ptr->ents[0] = e->h;
                }
            }
            if (new_chunk_was_valid)
                for (int i=0; i<MAX_ENTS_PER_TILE; i++) { //------------------------------------------------------------ Store handle in new tile.
                    entBasics* tileEnt = getEnt(new_tile_ptr->ents[i], 0);
                    if (tileEnt == 0 || (i == MAX_ENTS_PER_TILE-1 && tileEnt && e->type != gib_type && tileEnt->type == gib_type && tooManyGibs)) {
                        new_tile_ptr->ents[i] = e->h;
                        break;
                    }
                } //----- NOTE: copy_handle() isn't used on e->h here. Use it for sharing e->h with other ents.
        }
    }
}
void moveEnt(entBasics* e) { //------------ Update an ent's position based on its velocity:
    e->pos = v2fAdd(e->pos, v2fScale(e->vel, dt));
    // Apply friction:
    float speed = v2fLen(e->vel);
    float friction = speed*8*dt;
    int hasFriction = (e->flags & NOFRICTION) == 0;
    e->vel = v2fSub(e->vel, v2fScale(v2fNormalized(e->vel), friction*hasFriction));
    if (v2fLen(e->vel) < 5) { e->vel = (vec2f){0,0}; } // Minimum vel.
}

float MIN_SQUARE_DISTANCE = RSIZE;
void collide_wall(entBasics* e) {
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
    for (int i=getFirstEnt(array, array_len); i != -1; i=getNextEnt(i, array, array_len)) {
        if (array[i] != HEADER_BYTE) {
            if (DEBUG_ENTS) { printf("*** Invalid index given by getNextEnt() in thinkAllEnts()\n"); }
            break;
        }
        entBasics* e = ((entBasics*)&array[i]);
        if ((e->flags & NOCOLLISION) != NOCOLLISION)
            collide_wall(e);
    }
}
void defragEntArray() {
    if (mainWorld->numGibs < MAX_GIBS) //(mainWorld->entArraySpace < ENTITY_BYTES_ARRAY_LEN/2)
            return;
    char* array = mainWorld->entity_bytes_array;
    int array_len = ENTITY_BYTES_ARRAY_LEN;
    for (int i=getFirstEnt(array, array_len); i != -1; i=getNextEnt(i, array, array_len)) {
        entBasics* e = ((entBasics*)&array[i]);
        if (e && e->type == gib_type && rand() < RAND_MAX/32){ // Delete random gibs (not just the newest ones).
            despawnEnt(e);
        }
        if (mainWorld->numGibs < MAX_GIBS)
            break;
    }
    //printf("Back to %d gibs.\n", mainWorld->numGibs);
}

#define MAX_RAYCAST_DISTANCE 128
struct tile* raycast_upto_selected_tile(vec2f pos, vec2f dir, vec2i sel) {
    struct tile* cur_tile = 0;
    struct tile* prev_tile = 0;
    vec2i tile_index;
    dir = v2fNormalized(dir);
    for (int i=0; i<MAX_RAYCAST_DISTANCE; i++) {
        pos = v2fAdd(pos, v2fScale(dir, RSIZE/4)); //------------- Step forward.
        tile_index = v2fToIRoundUp(v2fScalarDiv(pos, RSIZE));
        cur_tile = worldGetTile(tile_index);
        if (cur_tile == 0 || cur_tile->wall_height >= 1) { break; }
        prev_tile = cur_tile;
        if (v2iIsEq(tile_index, sel)) { break; }
    }
    return prev_tile;
}
struct tile* raycast_into_selected_tile(vec2f pos, vec2f dir, vec2i sel) {
    struct tile* cur_tile = 0;
    vec2i tile_index;
    dir = v2fNormalized(dir);
    for (int i=0; i<MAX_RAYCAST_DISTANCE; i++) {
        pos = v2fAdd(pos, v2fScale(dir, RSIZE/4)); //------------- Step forward.
        tile_index = v2fToI(v2fScalarDiv(pos, RSIZE));
        cur_tile = worldGetTile(tile_index);
        if (cur_tile == 0 || cur_tile->wall_height >= 1 || v2iIsEq(tile_index, sel)) { break; }
    }
    return cur_tile;
}
int isPathToTileClear(vec2f pos, vec2f dir, vec2i sel) {
    struct tile* cur_tile = 0;
    vec2i tile_index;
    dir = v2fNormalized(dir);
    for (int i=0; i<MAX_RAYCAST_DISTANCE; i++) {
        pos = v2fAdd(pos, v2fScale(dir, RSIZE/4)); //------------- Step forward.
        tile_index = v2fToIRoundUp(v2fScalarDiv(pos, RSIZE));
        cur_tile = worldGetTile(tile_index);
        if (cur_tile == 0 || cur_tile->wall_height >= 1)
            return 0;
        if (v2iIsEq(tile_index, sel))
            break;
    }
    return 1;
}
