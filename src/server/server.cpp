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
    if (chonk->tiles[selected_y][selected_x].wall_height > 254)
        return;
    // Play a sound.
    Mix_PlayChannel(-1, sound, 0);
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

int main() {
    server_config();
    
    
    //
    // start the game
    //
    running = 1;
    init_graphics();
    init_audio();
    
    //
    // build an example world of entities
    //
    world test_world;
    chunk* chunk_0 = test_world.get_chunk(0,0);
    chunk_0->set_floors(floor_test);
    /*
    chunk_0->set_floor(0,0, tiledark);
    chunk_0->set_wall(1,4, wall_steel,wall_steel,8);
    chunk_0->set_wall(3,4, wall_steel,wall_steel,8);
    chunk_0->set_wall(7,7, wall_steel,wall_steel,64);
    chunk_0->set_wall(8,7, wall_steel,wall_steel,16);
    */
    for (int y=0; y<CHUNK_WIDTH; y++) {
        chunk_0->set_wall(0,y, wall_steel,wall_steel_side,16);
        chunk_0->set_wall(CHUNK_WIDTH-1,y, wall_steel,wall_steel_side,16);
    }
    for (int x=0; x<CHUNK_WIDTH; x++) {
        chunk_0->set_wall(x,0, wall_steel,wall_steel_side,16);
        chunk_0->set_wall(x,CHUNK_WIDTH-1, wall_steel,wall_steel_side,16);
    }
    
    
    
    
    constexpr int SEGMENT_ARRAY_SIZE = 4096;
    segment entity_segment_array[SEGMENT_ARRAY_SIZE];
    ent_PLAYER* p = (ent_PLAYER*)spawn_ent(PLAYER, entity_segment_array, SEGMENT_ARRAY_SIZE);
    player_client.player = (segment*)p;
    
    
    ent_SCENERY* s = (ent_SCENERY*)spawn_ent(SCENERY, entity_segment_array, SEGMENT_ARRAY_SIZE);
    s->data[pos].pos.pos = vec2f{RSIZE*1.5,RSIZE*1.5};
    printf("*Type name: %s\n", get_type_name(s->data[head].head.type));
    
    //segment* test_ent = spawn_ent(SCENERY, entity_segment_array, SEGMENT_ARRAY_SIZE);
    
    while (running) {
        dt = (SDL_GetTicks() - last_frame_end) / 1000;
        anim_tick = SDL_GetTicks() % 256; // 8-bit timestamp for sprites to know when to go to the next frame.
        last_frame_end = SDL_GetTicks();
        track_fps();
        //
        // handle client inputs and movement
        //
        client_input(&player_client);
        player_client.update_player_entity(); // Apply client inputs to the player entity.
        //
        // update the player's camera position
        //
        player_client.camera_pos = vec2f {p->data[pos].pos.pos.x - window_x/2 + RSIZE/2, p->data[pos].pos.pos.y - window_y/2 + RSIZE/2};
        player_client.camera_center = vec2f {p->data[pos].pos.pos.x, p->data[pos].pos.pos.y};

        if (player_client.attacking)
            destroy_wall(player_client.camera_center, player_client.aim_pixel_pos, chunk_0);
        if (player_client.building)
            build_wall(player_client.camera_center, player_client.aim_pixel_pos, chunk_0);

        //
        // Draw the environment:
        //
        SDL_RenderClear(renderer);
        draw_chunk(player_client.camera_pos, player_client.camera_center, test_world.get_chunk(0,0));
        //
        // Update and draw the entities:
        //
        think_all_ents(entity_segment_array, SEGMENT_ARRAY_SIZE);
        draw_all_ents(player_client.camera_pos, entity_segment_array, SEGMENT_ARRAY_SIZE);
        move_ent((segment*) p);

        present_frame(); // Put the frame on the screen:
    }
    printf("Server was running for %d seconds.\n", SDL_GetTicks() / 1000);
    cleanup_graphics();
    cleanup_audio();
    return 0;
}
