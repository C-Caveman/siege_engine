// update the world using messages from the clients,
// update the clients on what has happened (if multiplayer)

#include "../server.h"

void sword_movement(ent* e, ent* e_displayer) {
    e_displayer->move_ent(e->get_pos() + vec2(mouse_x,mouse_y));
}

void server_config() {
    char varlist_fname[] = "config/vars_list.txt";
    char config_fname[] = "config/config.txt";
    // debug print the config variable indicies are in their table
    //print_hashes(varlist_fname);
    // initialize the config variable hash table
    set_var_ptrs();
    // load the config file data
    vars_from_file(config_fname);
}

int id = 0;
int new_id() {
    id++;
    return id;
}

int main() {
    server_config();
    // make the player_0 entity
    ent e1(new_id(), type_player, knight_1, 0, 0, 2, vec2(0,0), vec2(0,0), vec2(0,0));
    entities[0] = e1;
    client_init(0, &entities[0]);
    ent* player_0 = &entities[0];
    player_0->set_state(state_player_speed,8); // walk instead of sneak
    
    
    //
    // start the game
    //
    running = 1;
    init_graphics();
    
    
    //
    // build an example world of entities
    //
    // TODO use a function to populate the server's entity array
    world test_world;
    test_world.get_chunks()->set_blocks(stonedk);
    test_world.get_chunks()->set_block(1,0, tiledark,1);
    int num_ents = 5;
    ent* ent_array[MAX_ENTS];
    entities[1]=ent(new_id(), type_scenery, stone, 0, 0, 1, vec2(0,0), vec2(0,0), vec2(0,0));
    entities[2]=ent(new_id(), type_gun, sword, 0, 0, 1, vec2(0,0), vec2(0,0), vec2(0,0));
    entities[3]=ent(new_id(), type_scenery, firepit, 0, 0, 1, vec2(0,0), vec2(0,0), vec2(0,0));
    entities[4]=ent(new_id(), type_scenery, sand, 0, 0, 1, vec2(0,0), vec2(0,0), vec2(0,0));
    ent* rock_ent =  &entities[1];
    ent* sword_ent = &entities[2];
    ent* fire_ent =  &entities[3];
    ent* sand_ent =  &entities[4];
    rock_ent->slide_ent(vec2(200, 200));
    sword_ent->slide_ent(vec2(200+128, 200));
    fire_ent->slide_ent(vec2(200+128, 200+128));
    sand_ent->slide_ent(vec2(200, 200+128));
    
    while (running) {
        dt = (SDL_GetTicks() - last_frame_end) / 1000;
        last_frame_end = SDL_GetTicks();
        track_fps();
        //
        // handle client inputs and movement
        //
        client_input(player_0);
        player_accel(player_0);
        //
        // move things according to their velocity
        //
        move(player_0);
        //
        // do collisions
        //
        player_0->collide_ent_cs(rock_ent);
        sword_movement(player_0, sword_ent);
        //
        // update the player's camera position
        //
        view_x = player_0->get_pos().get_x();
        view_y = player_0->get_pos().get_y();
        //
        // draw the world
        //
        draw_chunk(test_world.get_chunks());
        draw_ents(entities, num_ents);
    }
    printf("Server was running for %d seconds.\n", SDL_GetTicks() / 1000);
    cleanup_graphics();
    return 0;
}
