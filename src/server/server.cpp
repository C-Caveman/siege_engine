// update the world using messages from the clients,
// update the clients on what has happened (if multiplayer)

#include "server.h"
#include <unistd.h> // testing memory leaks with sleep()

// entity and client data are in these arrays
int num_entities = 0;
int num_clients = 0;

client_data player_client;

// each entity gets a unique ID number
int id = 0;
int new_id() {return ++id;}
uint8_t anim_tick = 0;

void server_config() {
    //char varlist_fname[] = "config/vars_list.txt";
    char config_fname[] = "config/config.txt";
    // debug print the config variable indicies are in their table
    //print_hashes(varlist_fname);
    // initialize the config variable hash table
    set_var_ptrs(); // TODO migrate to new config system <--------------- TODO
    // load the config file data
    vars_from_file(config_fname);
}

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
        ( (int(camera_center.x/RSIZE) == selected_x) && (int(camera_center.y/RSIZE) == selected_y) )
    )
        return;
    // Play a sound.
    if (chonk->tiles[selected_y][selected_x].wall_height == 0)
        { Mix_PlayChannel(-1, sound, 0); }
    chonk->set_wall(selected_x,
                    selected_y,
                    wall_steel,wall_steel_side,chonk->tiles[selected_y][selected_x].wall_height + 1);
}
void destroy_wall(vec2f camera_center, vec2i aim_pixel, chunk* chonk) {
    // No longer the start of a click. TODO rename and move to input.cpp TODO
    int selected_x = (camera_center.x + aim_pixel.x) / RSIZE + 0.5;
    int selected_y = (camera_center.y + aim_pixel.y) / RSIZE + 0.5;
    if (selected_x < 0) selected_x = 0;
    if (selected_y < 0) selected_y = 0;
    if (selected_x > (CHUNK_WIDTH-1)) selected_x = (CHUNK_WIDTH-1);
    if (selected_y > (CHUNK_WIDTH-1)) selected_y = (CHUNK_WIDTH-1);
    if (chonk->tiles[selected_y][selected_x].wall_height <= 0)
        return;
    // Play a sound.
    if (chonk->tiles[selected_y][selected_x].wall_height == 1)
        Mix_PlayChannel(-1, sound, 0);
    chonk->set_wall(selected_x,
                    selected_y,
                    wall_steel,wall_steel_side, chonk->tiles[selected_y][selected_x].wall_height - 1 );
}

//TODO move this to ent.cpp
constexpr float PLAYER_WIDTH = RSIZE;
constexpr float MIN_SQUARE_DISTANCE = PLAYER_WIDTH/2 + RSIZE/2;
void collide_wall(struct ent_basics* e, vec2i chunk_index) {
    chunk* c = &main_world->chunks[chunk_index.y][chunk_index.x];
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
            if (tile_index.in_bounds(0, CHUNK_WIDTH-1) && c->tiles[tile_index.y][tile_index.x].wall_height > 0)
                 { collisions[i][j] = true; num_collisions++; }
            else { collisions[i][j] = false; }
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
            tile_index = vec2i{int(tile_pos.x/RSIZE), int(tile_pos.y/RSIZE)} % CHUNK_WIDTH;
            if (c->tiles[tile_index.y][tile_index.x].wall_height <= 0) { continue; } // Skip the tile.
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
                if (anim_tick > 100 && num_collisions > 0 && e->type == player_type)
                    {std::cout << "H > V = " << (abs(delta.x) > abs(delta.y)) << " at " << tile_index << "\n";}
            }
        }
        sign_y *= -1;
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
    struct ent_basics* e;
    for (int i=get_first_ent(array, array_len); i != -1; i=get_next_ent(i, array, array_len)) {
        if (array[i] != HEADER_BYTE) { printf("*** Invalid index given by get_next_ent() in move_all_ents()\n"); exit(-1); }
        //---------------------------------- move the entity, record its position in the chunk
        e = (struct ent_basics*)&array[i];
        vec2i old_tile = e->tile; //--------------------------------------------------------------------------------- Old tile.
        vec2i old_chunk = e->chunk; //------------------------------------------------------------------------------- Old chunk.
        move_ent(e);
        e->chunk = ( (e->pos+vec2f{RSIZE/2,RSIZE/2}) / RSIZE / CHUNK_WIDTH ).to_int();
        vec2f floored = e->pos - (e->chunk * RSIZE * CHUNK_WIDTH).to_float();
        e->tile = (floored / RSIZE + vec2f{0.5,0.5}).to_int();
        bool changed_tile = (e->tile != old_tile); //--------------------------------------------------------------- New tile?
        bool old_tile_was_valid = old_tile.in_bounds(0, CHUNK_WIDTH-1);
        bool new_tile_was_valid = e->tile.in_bounds(0, CHUNK_WIDTH-1);
        bool old_chunk_was_valid = old_chunk.in_bounds(0, WORLD_WIDTH-1) && old_tile_was_valid;
        bool new_chunk_was_valid = e->chunk.in_bounds(0, WORLD_WIDTH-1) && new_tile_was_valid;
        tile* old_tile_ptr = &main_world->chunks[old_chunk.y][old_chunk.x].tiles[old_tile.y][old_tile.x];
        tile* new_tile_ptr = &main_world->chunks[e->chunk.y][e->chunk.x].tiles[e->tile.y][e->tile.x];
        if (changed_tile) {
            for (int i=0; i<MAX_ENTS_PER_TILE; i++) { //------------------------------------------------------------- Remove handle from old tile.
                if (old_chunk_was_valid && old_tile_ptr->ents[i] == e->h) { old_tile_ptr->ents[i] = 0; old_tile_ptr->floor_anim = stonedk; }
            }
            for (int i=0; i<MAX_ENTS_PER_TILE; i++) { //------------------------------------------------------------ Store handle in new tile.
                if (new_chunk_was_valid && new_tile_ptr->ents[i] == 0) { new_tile_ptr->ents[i] = e->h; break; }
            } //----- NOTE: copy_handle() isn't used on e->h here. Use it for sharing e->h with other ents.
        }
        //if (changed_tile && e->type == player_type) { std::cout << "Tile: " << e->tile << "\nChunk: " << e->chunk << "\n\n";}
    }
}
//===================================================================// RAYCASTING TODO shortest axis TODO //
#define MAX_RAYCAST_DISTANCE 128
tile* cast_ray(chunk chunks[WORLD_WIDTH][WORLD_WIDTH], vec2f pos, vec2f dir) {
    tile* cur_tile = nullptr;
    vec2i tile_pos;
    vec2i chunk_pos;
    vec2i chunk_tile;
    dir = dir.normalized();
    for (int i=0; i<MAX_RAYCAST_DISTANCE; i++) {
        pos = pos + dir * RSIZE/4; //------------- Step forward.
        tile_pos = vec2i{ (int)std::floor(pos.x/RSIZE + 0.5), (int)std::floor(pos.y/RSIZE + 0.5) }; //-- Cur tile.
        chunk_pos = vec2i{ tile_pos.x/CHUNK_WIDTH, tile_pos.y/CHUNK_WIDTH }; //-- Cur chunk.
        chunk_tile = vec2i{ tile_pos.x%CHUNK_WIDTH, tile_pos.y%CHUNK_WIDTH }; //-- Position in chunk.
        if (!chunk_pos.in_bounds(0, WORLD_WIDTH-1) || !chunk_tile.in_bounds(0, CHUNK_WIDTH-1)) { break; }
        cur_tile = &chunks[chunk_pos.y][chunk_pos.x].tiles[chunk_tile.y][chunk_tile.x];
        bool hit_wall = (cur_tile->wall_height > 0);
        if (hit_wall) { return cur_tile; }
    }
    return nullptr;
}
tile* cast_ray_pre_impact(chunk chunks[WORLD_WIDTH][WORLD_WIDTH], vec2f pos, vec2f dir) {
    tile* cur_tile = nullptr;
    vec2i tile_pos;
    vec2i chunk_pos;
    vec2i chunk_tile;
    dir = dir.normalized();
    for (int i=0; i<MAX_RAYCAST_DISTANCE; i++) {
        pos = pos + dir * RSIZE/4; //------------- Step forward.
        tile_pos = vec2i{ (int)std::floor(pos.x/RSIZE + 0.5), (int)std::floor(pos.y/RSIZE + 0.5) }; //-- Cur tile.
        chunk_pos = vec2i{ tile_pos.x/CHUNK_WIDTH, tile_pos.y/CHUNK_WIDTH }; //-- Cur chunk.
        chunk_tile = vec2i{ tile_pos.x%CHUNK_WIDTH, tile_pos.y%CHUNK_WIDTH }; //-- Position in chunk.
        if (!chunk_pos.in_bounds(0, WORLD_WIDTH-1) || !chunk_tile.in_bounds(0, CHUNK_WIDTH-1) ||
            !tile_pos.in_bounds(0,WORLD_WIDTH*CHUNK_WIDTH-1) ||
            chunks[chunk_pos.y][chunk_pos.x].tiles[chunk_tile.y][chunk_tile.x].wall_height > 0) {
                break;
        }
        cur_tile = &chunks[chunk_pos.y][chunk_pos.x].tiles[chunk_tile.y][chunk_tile.x];
    }
    if (chunk_pos.in_bounds(0, WORLD_WIDTH-1) && chunk_tile.in_bounds(0, CHUNK_WIDTH-1)) { return cur_tile; }
    else { return nullptr; }
}
int main() {
    server_config();
    //===========================================================// Initialize everything. //
    running = 1;
    init_graphics();
    //SDL_RenderSetScale(renderer, 0.5f, 0.5f); //------------------- Zoom in/out TODO properly center/scale everything TODO
    init_audio();
    struct world test_world;
    main_world = &test_world;
    //======================================================================================// Place tiles. //
    chunk* chunk_0 = &test_world.chunks[0][0];
    test_world.chunks[1][0].tiles[4][4].wall_height = 16;
    chunk_0->set_floors(floor_test);
    for (int y=0; y<CHUNK_WIDTH; y++) {
        chunk_0->set_wall(0,y, wall_steel,wall_steel_side,16);
        chunk_0->set_wall(CHUNK_WIDTH-1,y, wall_steel,wall_steel_side,16);
    }
    for (int x=0; x<CHUNK_WIDTH; x++) {
        chunk_0->set_wall(x,0, wall_steel,wall_steel_side,16);
        chunk_0->set_wall(x,CHUNK_WIDTH-1, wall_steel,wall_steel_side,16);
    } //====================================================================================// Spawn entities. //
    ent_player* p = (struct ent_player*)spawn_ent(player_type, main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
    p->pos = vec2f{RSIZE,RSIZE};
    player_client.player = (struct ent_player*)p;
    ent_scenery* s = (ent_scenery*)spawn_ent(scenery_type, main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
    s->pos = vec2f{RSIZE*1.5, RSIZE*CHUNK_WIDTH/2};
    s->fren = p->h;
    printf("*Type name: '%s'\n", get_type_name(s->type));
    while (running) { //=====================================================================================// GAME LOOP //
        dt = (SDL_GetTicks() - last_frame_end) / 1000;
        anim_tick = SDL_GetTicks() % 256; // 8-bit timestamp for sprites to know when to go to the next frame.
        last_frame_end = SDL_GetTicks();
        track_fps();
        //=======================================================================================// Client inputs and movement. //
        client_input(&player_client);
        player_client.update_player_entity(); // Apply client inputs to the player entity.
        collide_wall((struct ent_basics*)p, p->chunk); //--------------------------------- Collision.
        collide_wall((struct ent_basics*)s, s->chunk);
        //if (anim_tick == 0) std::cout << p->pos << "\n";
        //=======================================================================================// Update the player's camera position. //
        player_client.camera_pos =
            vec2f {p->pos.x - window_x/2 + RSIZE/2, p->pos.y - window_y/2 + RSIZE/2};
        player_client.camera_center =
            vec2f {p->pos.x, p->pos.y};
        //=======================================================================================// Building/Destroying tiles. //
        if (player_client.attacking) {
            //destroy_wall(player_client.camera_center, player_client.aim_pixel_pos, chunk_0);
            tile* timmy = cast_ray(test_world.chunks, p->pos,
                                   vec2f{cos(player_client.aim_dir/180*(float)M_PI),
                                         sin(player_client.aim_dir/180*(float)M_PI)});
            if (timmy != nullptr) { timmy->wall_height = 0; player_client.attacking = 0; }
        }
        if (player_client.building) {
            //build_wall(player_client.camera_center, player_client.aim_pixel_pos, chunk_0);
            tile* timmy = cast_ray_pre_impact(test_world.chunks, p->pos,
                                              vec2f{cos(player_client.aim_dir/180*(float)M_PI),
                                                    sin(player_client.aim_dir/180*(float)M_PI)});
            if (timmy != nullptr) { timmy->wall_height = 16; player_client.building = 0; }
        }
        SDL_RenderClear(renderer); //=========================================================================// Draw the environment. //
        vec2i order[9] = {
            {-1,1}, {1,1}, {-1,-1}, {1,-1}, //- Corners.
            {-1,0}, {0,1}, {1,0}, {0,-1}, //--- Sides.
            {0,0} //--------------------------- Middle.
        };
        for (int i=0; i<9; i++) {
            vec2i next_chunk = p->chunk + order[i];
            if ( next_chunk.in_bounds(0,WORLD_WIDTH) ) {
                draw_chunk_floor(
                    player_client.camera_pos, player_client.camera_center,
                    &test_world.chunks[next_chunk.y][next_chunk.x],
                    vec2i{next_chunk.x, next_chunk.y});
            }
        }
        for (int i=0; i<9; i++) {
            vec2i next_chunk = p->chunk + order[i];
            if ( next_chunk.in_bounds(0,WORLD_WIDTH) ) {
                draw_chunk_other(
                    player_client.camera_pos, player_client.camera_center,
                    &test_world.chunks[next_chunk.y][next_chunk.x],
                    vec2i{next_chunk.x, next_chunk.y});
            }
        }
        //===========================================================================// Update and draw the entities. //
        think_all_ents(main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
        //draw_all_ents(player_client.camera_pos, main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN); // NO LONGER USED?
        move_all_ents(main_world->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
        present_frame(); // Put the frame on the screen:
    }
    printf("Server was running for %d seconds.\n", SDL_GetTicks() / 1000);
    cleanup_graphics();
    cleanup_audio();
    return 0;
}
