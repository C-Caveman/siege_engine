// update the world using messages from the clients,
// update the clients on what has happened (if multiplayer)

#include "server.h"
#include <unistd.h> // testing memory leaks with sleep()

extern bool m1_held;
extern int mouse_angle;

// entity and client data are in these arrays
int num_entities = 0;
int num_clients = 0;

//TODO replace old client system TODO
client_data player_client;

// each entity gets a unique ID number
int id = 0;
int new_id() {return ++id;}

void server_config() {
    char varlist_fname[] = "config/vars_list.txt";
    char config_fname[] = "config/config.txt";
    // debug print the config variable indicies are in their table
    //print_hashes(varlist_fname);
    // initialize the config variable hash table
    set_var_ptrs(); // TODO migrate to new config system <--------------- TODO
    // load the config file data
    vars_from_file(config_fname);
}

void place_wall(chunk* chonk) {
    // No longer the start of a click. TODO rename and move to input.cpp TODO
    m1_held = false;
    // Adjusted player position (to get the center).
    int camera_offset_x = (int)(view_x) + RSIZE/2;
    int camera_offset_y = (int)(view_y) + RSIZE/2;
    int selected_x = (camera_offset_x + mouse_x) / RSIZE;
    int selected_y = (camera_offset_y + mouse_y) / RSIZE;
    if (selected_x < 0) selected_x = 0;
    if (selected_y < 0) selected_y = 0;
    if (selected_x > (CHUNK_WIDTH-1)) selected_x = (CHUNK_WIDTH-1);
    if (selected_y > (CHUNK_WIDTH-1)) selected_y = (CHUNK_WIDTH-1);
    printf("Tile: (%d, %d)\n", selected_x, selected_y);
    chonk->set_wall(selected_x,
                    selected_y,
                    wall_steel,wall_steel_side,16);
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
    chunk_0->set_floor(0,0, tiledark);
    chunk_0->set_wall(1,4, wall_steel,wall_steel_side,8);
    chunk_0->set_wall(3,4, wall_steel,wall_steel_side,8);
    chunk_0->set_wall(7,7, wall_steel,wall_steel_side,8);
    chunk_0->set_wall(8,7, wall_steel,wall_steel_side,16);
    
    
    
    
    constexpr int SEGMENT_ARRAY_SIZE = 4096;
    segment entity_segment_array[SEGMENT_ARRAY_SIZE];
    ent_PLAYER* p = (ent_PLAYER*)spawn_ent(PLAYER, entity_segment_array, SEGMENT_ARRAY_SIZE);
    player_client.player = (segment*)p;
    
    
    //ent_SCENERY* s = (ent_SCENERY*)spawn_ent(SCENERY, entity_segment_array, SEGMENT_ARRAY_SIZE);
    //printf("*Type name: %s\n", get_type_name(p->data[0].head.type));
    
    //segment* test_ent = spawn_ent(SCENERY, entity_segment_array, SEGMENT_ARRAY_SIZE);
    
    while (running) {
        dt = (SDL_GetTicks() - last_frame_end) / 1000;
        last_frame_end = SDL_GetTicks();
        track_fps();
        //
        // handle client inputs and movement
        //
        client_input(&player_client);
        player_client.update_player_entity(); // <- new client system
        //
        // update the player's camera position
        //
        view_x = p->data[pos].pos.pos.x - window_x/2 + RSIZE/2;
        view_y = p->data[pos].pos.pos.y - window_y/2 + RSIZE/2;
        
        if (m1_held)
            place_wall(chunk_0);
        
        //
        // draw the world
        //
        draw_chunk(test_world.get_chunk(0,0), vec2f{view_x, view_y});
        
        think_all_ents(entity_segment_array, SEGMENT_ARRAY_SIZE);
        draw_all_ents(entity_segment_array, SEGMENT_ARRAY_SIZE);
        move_ent((segment*) p);
        
        present_frame();
    }
    printf("Server was running for %d seconds.\n", SDL_GetTicks() / 1000);
    cleanup_graphics();
    cleanup_audio();
    return 0;
}
