// update the world using messages from the clients,
// update the clients on what has happened (if multiplayer)

#include "server.h"
#include <unistd.h> // testing memory leaks with sleep()

// entity and client data are in these arrays
int num_entities = 0;
int num_clients = 0;

client playerClient;

// each entity gets a unique ID number
int id = 0;
int new_id() {return ++id;}
uint8_t anim_tick = 0;

void build_wall(vec2f camera_center, vec2i aim_pixel, chunk* chonk) {
    // No longer the start of a click. TODO rename and move to input.cpp TODO
    int selected_x = (camera_center.x + aim_pixel.x) / RSIZE + 0.5;
    int selected_y = (camera_center.y + aim_pixel.y) / RSIZE + 0.5;
    if (selected_x < 0) selected_x = 0;
    if (selected_y < 0) selected_y = 0;
    if (selected_x > (CHUNK_WIDTH-1)) selected_x = (CHUNK_WIDTH-1);
    if (selected_y > (CHUNK_WIDTH-1)) selected_y = (CHUNK_WIDTH-1);
    //printf("Tile: (%d, %d)\n", selected_x, selected_y);
    //printf("Pos:  (%f, %f)\n", camera_center.x, camera_center.y);
    if (chonk->tiles[selected_y][selected_x].wall_height > 254 ||
        ( (int(camera_center.x/RSIZE) == selected_x) && (int(camera_center.y/RSIZE) == selected_y) ))
        return;
    struct tile* t = &chonk->tiles[selected_y][selected_x];
    // Play a sound.
    if (t->wall_height == 0) {
        playSound(thud);
    }
    chonk->set_wall(selected_x,
                    selected_y,
                    wall_steel,solid_grey,t->wall_height + 16);
    //t->wall_side_anim = solid_grey;
}
void destroy_wall(vec2f camera_center, vec2i aim_pixel, chunk* chonk) {
    //destroy_wall(playerClient.camera_center, playerClient.aim_pixel_pos, chunk_0);
    vec2i sel = ((camera_center + aim_pixel.to_float()) / RSIZE).to_int_round_up();
    if (sel.x < 0) sel.x = 0;
    if (sel.y < 0) sel.y = 0;
    if (sel.x > (CHUNK_WIDTH-1)) sel.x = (CHUNK_WIDTH-1);
    if (sel.y > (CHUNK_WIDTH-1)) sel.y = (CHUNK_WIDTH-1);
    if (chonk->tiles[sel.y][sel.x].wall_height <= 0)
        return;
    // Play a sound.
    if (chonk->tiles[sel.y][sel.x].wall_height == 1)
        playSound(thud);
    chonk->set_wall(sel.x,
                    sel.y,
                    wall_steel,wall_steel_side, chonk->tiles[sel.y][sel.x].wall_height - 1 );
}

vec2i getTileAtCursor(struct client* c) {
    if (c == nullptr) { printf("*** null client in getTileAtCursor!\n"); exit(-1); }
    return (struct vec2i)(((c->camera_center + c->aim_pixel_pos.to_float()) / RSIZE).to_int_round_up());
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
    struct ent_basics* e;
    for (int i=get_first_ent(array, array_len); i != -1; i=get_next_ent(i, array, array_len)) {
        if (array[i] != HEADER_BYTE) { printf("*** Invalid index given by get_next_ent() in move_all_ents()\n"); exit(-1); }
                                                                                    //- move the entity, record its position in the chunk
        e = (struct ent_basics*)&array[i];
        vec2i old_tile = e->tile;                                                   //- Old tile.
        vec2i old_chunk = e->chunk;                                                 //- Old chunk.
        move_ent(e);
        e->chunk = ( (e->pos+vec2f{RSIZE/2,RSIZE/2}) / RSIZE / CHUNK_WIDTH ).to_int();
        vec2f floored = e->pos - (e->chunk * RSIZE * CHUNK_WIDTH).to_float();
        e->tile = (floored / RSIZE + vec2f{0.5,0.5}).to_int();
        bool changed_tile = (e->tile != old_tile);                                  //- New tile?
        bool old_tile_was_valid = old_tile.in_bounds(0, CHUNK_WIDTH-1);
        bool new_tile_was_valid = e->tile.in_bounds(0, CHUNK_WIDTH-1);
        bool old_chunk_was_valid = old_chunk.in_bounds(0, WORLD_WIDTH-1) && old_tile_was_valid;
        bool new_chunk_was_valid = e->chunk.in_bounds(0, WORLD_WIDTH-1) && new_tile_was_valid;
        tile* old_tile_ptr = &main_world->chunks[old_chunk.y][old_chunk.x].tiles[old_tile.y][old_tile.x];
        tile* new_tile_ptr = &main_world->chunks[e->chunk.y][e->chunk.x].tiles[e->tile.y][e->tile.x];
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
tile* raycast_upto_selected_tile(vec2f pos, vec2f dir, vec2i sel) {
    tile* cur_tile = nullptr;
    tile* prev_tile = nullptr;
    vec2i tile_index;
    dir = dir.normalized();
    for (int i=0; i<MAX_RAYCAST_DISTANCE; i++) {
        pos = pos + dir * RSIZE/4; //------------- Step forward.
        tile_index = (pos / RSIZE).to_int_round_up();
        cur_tile = main_world->get_tile(tile_index);
        if (cur_tile == nullptr || cur_tile->wall_height >= 1) { break; }
        prev_tile = cur_tile;
        if (tile_index == sel) { break; }
    }
    return prev_tile;
}
tile* raycast_into_selected_tile(vec2f pos, vec2f dir, vec2i sel) {
    tile* cur_tile = nullptr;
    vec2i tile_index;
    dir = dir.normalized();
    for (int i=0; i<MAX_RAYCAST_DISTANCE; i++) {
        pos = pos + dir * RSIZE/4; //------------- Step forward.
        tile_index = (pos / RSIZE).to_int_round_up();
        cur_tile = main_world->get_tile(tile_index);
        if (cur_tile == nullptr || cur_tile->wall_height >= 1 || tile_index == sel) { break; }
    }
    return cur_tile;
}
int isPathToTileClear(vec2f pos, vec2f dir, vec2i sel) {
    tile* cur_tile = nullptr;
    vec2i tile_index;
    dir = dir.normalized();
    for (int i=0; i<MAX_RAYCAST_DISTANCE; i++) {
        pos = pos + dir * RSIZE/4; //------------- Step forward.
        tile_index = (pos / RSIZE).to_int_round_up();
        cur_tile = main_world->get_tile(tile_index);
        if (cur_tile == nullptr || cur_tile->wall_height >= 1)
            return 0;
        if (tile_index == sel)
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
    main_world = &test_world;
                                                                               //==============// Place tiles. //
    chunk* chunk_0 = &test_world.chunks[0][0];
    test_world.chunks[1][0].tiles[4][4].wall_height = 16;
    //chunk_0->set_floors(floor_test);
    chunk_0->set_floors(tileGold01);
    for (int y=0; y<WORLD_WIDTH; y++) {
        for (int x=0; x<WORLD_WIDTH; x++)
            test_world.chunks[y][x].set_floors(tileGold01);
    }
    for (int y=0; y<CHUNK_WIDTH; y++) {
        chunk_0->set_wall(0,y, wall_steel,wall_steel_side,16);
        chunk_0->set_wall(CHUNK_WIDTH-1,y, wall_steel,wall_steel_side,16);
    }
    for (int x=0; x<CHUNK_WIDTH; x++) {
        chunk_0->set_wall(x,0, wall_steel,wall_steel_side,16);
        chunk_0->set_wall(x,CHUNK_WIDTH-1, wall_steel,wall_steel_side,16);
    }                                                                          //==============// Spawn entities. //
    ent_player* p = (struct ent_player*)spawn_ent(player_type, main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
    p->pos = vec2f{RSIZE,RSIZE};
    playerClient.player = (struct ent_player*)p;
    ((struct ent_player*)p)->cl = &playerClient;
    ent_scenery* s = (ent_scenery*)spawn_ent(scenery_type, main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
    s->pos = vec2f{(float)(RSIZE*1.5), RSIZE*CHUNK_WIDTH/2};
    s->fren = p->h;
    printf("*Type name: '%s'\n", get_type_name(s->type));
    struct ent_basics* bunny = (struct ent_basics*)spawn_ent(rabbit_type, main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
    bunny->pos = vec2f{RSIZE*2, RSIZE*2};
    struct ent_basics* zombieGuy = (struct ent_basics*)spawn_ent(zombie_type, main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
    zombieGuy->pos = vec2f{RSIZE*2, RSIZE*4};
    
    
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
        playerClient.update_player_entity();                                   //- Client_Inputs -> Player_Entity.
                                                                                //=================// Update the player's camera position. //
        playerClient.camera_pos =
            vec2f {p->pos.x - window_x/2*(RSIZE/tileWidth) + RSIZE/2, p->pos.y - window_y/2*(RSIZE/tileWidth) + RSIZE/2};
        playerClient.camera_center =
            vec2f {p->pos.x, p->pos.y};
                                                                                //================// Building/Destroying tiles. //
        if (playerClient.attacking && (cur_frame_start - playerClient.lastAttackTime) > 200) {
            playerClient.lastAttackTime = cur_frame_start;
            playerClient.player->sprites[PLAYER_GUN].frame = 0;;
            playerClient.player->sprites[PLAYER_GUN].flags &= ~PAUSED;
            //playSound(bam02);
            playSoundChannel(bam02, CHAN_WEAPON);
            void* e = spawn_ent(projectile_type, main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
            vec2f aimDir = vec2f{cos(playerClient.aim_dir/180*(float)M_PI), sin(playerClient.aim_dir/180*(float)M_PI)};
            ((struct ent_basics*)e)->pos = playerClient.player->pos + aimDir*100;
            ((struct ent_basics*)e)->tile = (playerClient.player->pos + aimDir*100).to_int() / RSIZE;
            ((struct ent_basics*)e)->vel = aimDir * 800;
        }
        /*
        if (playerClient.attacking && (cur_frame_start - playerClient.lastAttackTime) > 200) {
            //destroy_wall(playerClient.camera_center, playerClient.aim_pixel_pos, chunk_0);
            playerClient.lastAttackTime = cur_frame_start;
            tile* timmy = raycast_into_selected_tile(p->pos,
                                                   vec2f{cos(playerClient.aim_dir/180*(float)M_PI),
                                                         sin(playerClient.aim_dir/180*(float)M_PI)},
                                                   getTileAtCursor(&playerClient));
            if (timmy != nullptr && timmy->wall_height != 0) { timmy->wall_height = 0; playSound(thud); }
        }
        */
        if (playerClient.building && (cur_frame_start - playerClient.lastBuildTime) > 50) {
            //build_wall(playerClient.camera_center, playerClient.aim_pixel_pos, chunk_0);
            //
            int isClear = isPathToTileClear(p->pos,
                                            vec2f{cos(playerClient.aim_dir/180*(float)M_PI),
                                            sin(playerClient.aim_dir/180*(float)M_PI)},
                                            getTileAtCursor(&playerClient));
            tile* timmy = raycast_upto_selected_tile(p->pos,
                                                   vec2f{cos(playerClient.aim_dir/180*(float)M_PI),
                                                         sin(playerClient.aim_dir/180*(float)M_PI)},
                                                   getTileAtCursor(&playerClient));
            for (int i=0; i<MAX_ENTS_PER_TILE; i++) {
                if (timmy == nullptr)
                    break;
                if (timmy->ents[i] != 0)
                    timmy = nullptr;
            }
            if (isClear && timmy != nullptr) {
                playerClient.lastBuildTime = cur_frame_start;
                timmy->wall_height = 8;
                timmy->floor_anim = grass1Floor;
                timmy->wall_side_anim = grass1Side;
                timmy->wall_top_anim = grass1Side;
                playSound(thud);
            }
            //
        }
        SDL_RenderClear(renderer);                                              //===============// Draw the environment. //
    #define OFFSETS 9
        vec2i order[OFFSETS] = {
            {-1,1}, {1,1}, {-1,-1}, {1,-1}, //- Corners.
            {-1,0}, {0,1}, {1,0}, {0,-1}, //--- Sides.
            {0,0} //--------------------------- Middle.
        };
        for (int i=0; i<OFFSETS; i++) {
            vec2i next_chunk = p->chunk + order[i];
            if ( next_chunk.in_bounds(0,WORLD_WIDTH) ) {                        //- Floor pass.
                draw_chunk_floor(
                    playerClient.camera_pos, playerClient.camera_center,
                    &test_world.chunks[next_chunk.y][next_chunk.x],
                    vec2i{next_chunk.x, next_chunk.y});
            }
        }
        for (int i=0; i<OFFSETS; i++) {
            vec2i next_chunk = p->chunk + order[i];
            if ( next_chunk.in_bounds(0,WORLD_WIDTH) ) {                        //- Wall/entity pass.
                draw_chunk_ents(
                    playerClient.camera_pos, playerClient.camera_center,
                    &test_world.chunks[next_chunk.y][next_chunk.x],
                    vec2i{next_chunk.x, next_chunk.y});
            }
        }
        for (int i=0; i<OFFSETS; i++) {
            vec2i next_chunk = p->chunk + order[i];
            if ( next_chunk.in_bounds(0,WORLD_WIDTH) ) {                        //- Wall/entity pass.
                draw_chunk_walls(
                    playerClient.camera_pos, playerClient.camera_center,
                    &test_world.chunks[next_chunk.y][next_chunk.x],
                    vec2i{next_chunk.x, next_chunk.y});
            }
        }
        // DRAW A HUD!   
        drawFps(fps);
        
        //char message[] = "Example message.... Greetings! Hello world! Goodbye world! Farewell world? Nice to meet you world? Oh well, see ya world!";
        playerClient.showDialog();
        //draw_ent_sprites(playerClient.camera_pos, (struct ent_basics*)p);
        
        think_all_ents(main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN); //==========// Update/draw the entities. //
        move_all_ents(main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
        wallCollision(main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
        //draw_all_ents(playerClient.camera_pos, main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
        present_frame(); // Put the frame on the screen:
    }
    printf("Server was running for %d seconds.\n", SDL_GetTicks() / 1000);
    cleanup_graphics();
    cleanup_audio();
    return 0;
}
