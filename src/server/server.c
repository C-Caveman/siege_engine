// update the world using messages from the clients,
// update the clients on what has happened (if multiplayer)

#include "server.h"
#include <unistd.h> // testing memory leaks with sleep()

// entity and client data are in these arrays
int num_entities = 0;
int num_clients = 0;

struct client playerClient;

// each entity gets a unique ID number
int id = 0;
int new_id() {return ++id;}
uint8_t anim_tick = 0;

vec2i getTileAtCursor(struct client* c) {
    if (c == 0) { printf("*** null client in getTileAtCursor!\n"); exit(-1); }
    return v2fToIRoundUp(v2fScalarDiv(v2fAdd(c->camera_center,v2iToF(c->aim_pixel_pos)), RSIZE));
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
    ent_basics* e;
    for (int i=get_first_ent(array, array_len); i != -1; i=get_next_ent(i, array, array_len)) {
        if (array[i] != HEADER_BYTE) { printf("*** Invalid index given by get_next_ent() in move_all_ents()\n"); exit(-1); }
                                                                                    //- move the entity, record its position in the chunk
        e = (ent_basics*)&array[i];
        vec2i old_tile = e->tile;                                                   //- Old tile.
        vec2i old_chunk = e->chunk;                                                 //- Old chunk.
        move_ent(e);
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
            for (int i=0; i<MAX_ENTS_PER_TILE; i++) {                               //- Remove handle from old tile.
                if (old_chunk_was_valid && old_tile_ptr->ents[i] == e->h) { old_tile_ptr->ents[i] = 0; /* old_tile_ptr->floor_anim = stonedk; */ }
            }
            for (int i=0; i<MAX_ENTS_PER_TILE; i++) { //------------------------------------------------------------ Store handle in new tile.
                if (new_chunk_was_valid && new_tile_ptr->ents[i] == 0) { new_tile_ptr->ents[i] = e->h; break; }
            } //----- NOTE: copy_handle() isn't used on e->h here. Use it for sharing e->h with other ents.
        }
    }
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
int main() {
    applyConfig((char*)"config/config.txt");                                                           //===========// Initialize server. //
    running = 1;
    init_graphics();
    init_audio();
    struct world test_world;
    mainWorld = &test_world;
    initMainWorld();
                                                                               //==============// Place tiles. //
    struct chunk* chunk_0 = &test_world.chunks[0][0];
    test_world.chunks[1][0].tiles[4][4].wall_height = 16;
    //chunk_0->set_floors(floor_test);
    chunkSetFloors(chunk_0, tileGold01);
    for (int y=0; y<WORLD_WIDTH; y++) {
        for (int x=0; x<WORLD_WIDTH; x++)
            chunkSetFloors(&test_world.chunks[y][x], tileMetal04);
    }
    for (int y=0; y<CHUNK_WIDTH; y++) {
        chunkSetWall(chunk_0, 0,y, wall_steel,wall_steel_side,16);
        chunkSetWall(chunk_0, CHUNK_WIDTH-1,y, wall_steel,wall_steel_side,16);
    }
    for (int x=0; x<CHUNK_WIDTH; x++) {
        chunkSetWall(chunk_0, x,0, wall_steel,wall_steel_side,16);
        chunkSetWall(chunk_0, x,CHUNK_WIDTH-1, wall_steel,wall_steel_side,16);
    }                                                                          //==============// Spawn entities. //
    struct ent_player* p = (struct ent_player*)spawn(player_type, (vec2f){0,0});
    p->pos = (vec2f){RSIZE,RSIZE};
    playerClient.player = (struct ent_player*)p;
    ((struct ent_player*)p)->cl = &playerClient;
    struct ent_scenery* s = (struct ent_scenery*)spawn(scenery_type, (vec2f){0,0});
    s->pos = (vec2f){(float)(RSIZE*1.5), RSIZE*CHUNK_WIDTH/2};
    s->fren = p->h;
    //printf("*Type name: '%s'\n", get_type_name(s->type));
    ent_basics* bunny = (ent_basics*)spawn(rabbit_type, (vec2f){0,0});
    bunny->pos = (vec2f){RSIZE*2, RSIZE*2};
    ent_basics* zombieGuy = (ent_basics*)spawn(zombie_type, (vec2f){0,0});
    zombieGuy->pos = (vec2f){RSIZE*2, RSIZE*4};
    
    
    //playSoundChannel(arcLamp1, 6);
    playMusicLoop(campfire01); //TODO todon't do this
    //playMusicLoop(dorian01);
    
    //char message[] = "Example message.... Greetings! Hello world! Goodbye world! Farewell world? Nice to meet you world? Oh well, see ya world!";
    //playerClient.startDialog(message);
    //print_vars();
    
    
    while (running) {                                                           //======================// GAME LOOP //
        cur_frame_start = SDL_GetTicks();
        dt = (cur_frame_start - last_frame_end) / 1000;
        if (dt > 0.1f) // Cap the delta time.
            dt = 0.05f;
        anim_tick = SDL_GetTicks() % 256;                                       //- 8-bit timestamp for animations.
        last_frame_end = SDL_GetTicks();
        track_fps();
                                                                                //==================// Client inputs and movement. //
        client_input(&playerClient);
        if (playerClient.paused) {
            //memcpy(playerClient.menuText, pauseMenuText, sizeof(playerClient.menuText));
            SDL_RenderClear(renderer);
            renderMenu(&playerClient);
            //renderText(playerClient.menuText[0]);
            present_frame();
            continue;
        }
        clientUpdatePlayerEntity();                                   //- Client_Inputs -> Player_Entity.
                                                                                //=================// Update the player's camera position. //
        playerClient.camera_pos =
            (vec2f) {p->pos.x - window_x/2*(RSIZE/tileWidth) + RSIZE/2, p->pos.y - window_y/2*(RSIZE/tileWidth) + RSIZE/2};
        playerClient.camera_center =
            (vec2f) {p->pos.x, p->pos.y};
                                                                                //================// Building/Destroying tiles. //
        if (playerClient.attacking && (cur_frame_start - playerClient.lastAttackTime) > 300 && playerClient.player->heat.count < 1) {
            //playerClient.player->heat.count = (uint8_t)iclamp(20+(int)playerClient.player->heat.count, 0, 200);
            playerClient.lastAttackTime = cur_frame_start;
            playerClient.player->sprites[PLAYER_GUN].frame = 0;;
            playerClient.player->sprites[PLAYER_GUN].flags &= ~PAUSED;
            //playSound(bam02);
            playSoundChannel(bam02, CHAN_WEAPON);
            void* e = spawn(projectile_type, (vec2f){0,0});
            vec2f aimDir = angleToVector(playerClient.aim_dir);
            ((ent_basics*)e)->pos = v2fAdd(playerClient.player->pos, v2fScale(aimDir, RSIZE/2));
            ((ent_basics*)e)->tile = v2iScalarDiv(v2fToI(((ent_basics*)e)->pos), RSIZE);
            ((ent_basics*)e)->vel = v2fScale(aimDir, 800);
        }
        /*
        if (playerClient.attacking && (cur_frame_start - playerClient.lastAttackTime) > 200) {
            //destroy_wall(playerClient.camera_center, playerClient.aim_pixel_pos, chunk_0);
            playerClient.lastAttackTime = cur_frame_start;
            tile* timmy = raycast_into_selected_tile(p->pos,
                                                   (vec2f){cos(playerClient.aim_dir/180*(float)F_PI),
                                                         sin(playerClient.aim_dir/180*(float)F_PI)},
                                                   getTileAtCursor(&playerClient));
            if (timmy != 0 && timmy->wall_height != 0) { timmy->wall_height = 0; playSound(thud); }
        }
        */
        if (playerClient.building && (cur_frame_start - playerClient.lastBuildTime) > 50) {
            //build_wall(playerClient.camera_center, playerClient.aim_pixel_pos, chunk_0);
            
            //int isClear = isPathToTileClear(p->pos,
            //                                (vec2f){cos(playerClient.aim_dir/180*(float)F_PI),
            //                                sin(playerClient.aim_dir/180*(float)F_PI)},
            //                                getTileAtCursor(&playerClient));
            struct tile* timmy = worldGetTile(getTileAtCursor(&playerClient));
            for (int i=0; i<MAX_ENTS_PER_TILE; i++) {
                if (timmy == 0)
                    break;
                if (timmy->ents[i] != 0) {
                    ent_basics* e = get_ent(timmy->ents[i]);
                    if (e->type == gib_type)
                        despawn_ent(e);
                    else
                        timmy = 0;
                }
            }
            if (timmy != 0 && timmy->wall_height <= 0) {
                playerClient.lastBuildTime = cur_frame_start;
                timmy->wall_height = 8;
                timmy->floor_anim = grass1Floor;
                timmy->wall_side_anim = grass1Side;
                timmy->wall_top_anim = grass1Side;
                playSound(thud);
            }
        }
        SDL_RenderClear(renderer);                                              //===============// Draw the environment. //
    #define OFFSETS 9
        vec2i order[OFFSETS] = {
            {-1,1}, {1,1}, {-1,-1}, {1,-1}, //- Corners.
            {-1,0}, {0,1}, {1,0}, {0,-1}, //--- Sides.
            {0,0} //--------------------------- Middle.
        };
        //printf("In chunk %s.\n", v2iToString(p->chunk));
        for (int i=0; i<OFFSETS; i++) {
            vec2i next_chunk = v2iAdd(p->chunk, order[i]);
            if ( v2iInBounds(next_chunk, 0, WORLD_WIDTH-1) ) {                        //- Floor pass.
                //printf("Drawing floor of chunk %s.\n", v2iToString(next_chunk));
                draw_chunk_floor(
                    playerClient.camera_pos, playerClient.camera_center,
                    &test_world.chunks[next_chunk.y][next_chunk.x],
                    (vec2i){next_chunk.x, next_chunk.y});
            }
        }
        for (int i=0; i<OFFSETS; i++) {
            vec2i next_chunk = v2iAdd(p->chunk, order[i]);
            if ( v2iInBounds(next_chunk, 0,WORLD_WIDTH-1) ) {                        //- Wall/entity pass.
                draw_chunk_ents(
                    playerClient.camera_pos, playerClient.camera_center,
                    &test_world.chunks[next_chunk.y][next_chunk.x],
                    (vec2i){next_chunk.x, next_chunk.y});
            }
        }
        for (int i=0; i<OFFSETS; i++) {
            vec2i next_chunk = v2iAdd(p->chunk, order[i]);
            if ( v2iInBounds(next_chunk,0,WORLD_WIDTH-1) ) {                        //- Wall/entity pass.
                draw_chunk_walls(
                    playerClient.camera_pos, playerClient.camera_center,
                    &test_world.chunks[next_chunk.y][next_chunk.x],
                    (vec2i){next_chunk.x, next_chunk.y});
            }
        }
        // DRAW A HUD!   
        drawFps(fps);
        drawHeat(playerClient.player->heat.count);
        
        //char message[] = "Example message.... Greetings! Hello world! Goodbye world! Farewell world? Nice to meet you world? Oh well, see ya world!";
        clientShowDialog();
        //draw_ent_sprites(playerClient.camera_pos, (ent_basics*)p);
        think_all_ents(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN); //==========// Update/draw the entities. //
        move_all_ents(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
        wallCollision(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
        defragEntArray();
        //draw_all_ents(playerClient.camera_pos, mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
        present_frame(); // Put the frame on the screen:
    }
    printf("Server was running for %d seconds.\n", SDL_GetTicks() / 1000);
    cleanup_graphics();
    cleanup_audio();
    return 0;
}
