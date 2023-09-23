// update the world using messages from the clients,
// update the clients on what has happened (if multiplayer)

#include "../server.h"
#include <unistd.h> // testing memory leaks with sleep()

// entity and client data are in these arrays
int num_entities = 0;
struct ent entities[MAX_ENTS];
int num_clients = 0;
struct client clients[MAX_CLIENTS];

// each entity gets a unique ID number
int id = 0;
int new_id() {return ++id;}

// add an entity to "entities"
struct ent* add_ent(int type) {
    //TODO add an insert_ent() method here, so new ents fill gaps in the ent_array TODO
    entities[num_entities] = ent (new_id(), type, rocket_tank, 0, 0, 2, vec2(0,0), vec2(0,0), vec2(0,0));
    struct ent* e = &entities[num_entities];
    //printf("New ent id %d\n", e->get_id());
    num_entities++;
    return e;
}

// add a client to "clients"
struct client* add_client() {
    struct ent* client_ent  = add_ent(ENT_PLAYER);
    client_init(&clients[num_clients], client_ent);
    struct client* c = &clients[num_clients];
    num_clients++;
    return c;
}

void gun_movement(ent* e, ent* e_displayer) {
    e_displayer->move_ent(e->get_pos() );//+ vec2(mouse_x,mouse_y));
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

void wall_parallax(struct ent* wall_slices[], 
                   int num_slices, 
                   ent* player_entity, 
                   vec2 parent_position) {
    vec2 offset = player_entity->get_pos() - parent_position;
    offset = offset / vec2(window_x, window_y);
    //offset = offset / 128;
    //offset = offset +vec2(0, 128);
    for (int i=0; i<num_slices; i++) {
        //offset = offset * (i+1);
        wall_slices[i]->move_ent(parent_position - (offset * i * 10));
    }
}


int main() {
    server_config();
    // make the player_entity entity
    struct client* player_entity_client = add_client();
    //player_entity_client->get_ent()->set_anim(rocket_tank);
    ent* player_entity = player_entity_client->get_ent();
    player_entity->set_state(state_player_speed,8); // walk instead of sneak
    
    
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
    test_world.get_chunks()->set_blocks(stonedk);
    test_world.get_chunks()->set_block(1,0, tiledark,1);
    // spawn a rock
    ent* rock_ent =  add_ent(ENT_SCENERY);
    rock_ent->set_anim(stone);
    rock_ent->slide_ent(vec2(200, 200));
    // spawn the gun
    ent* gun_ent = add_ent(ENT_GUN);
    gun_ent->set_anim(gun_grenade);
    gun_ent->slide_ent(vec2(200+128, 200));
    // spawn a firepit
    ent* fire_ent = add_ent(ENT_SCENERY);
    fire_ent->set_anim(firepit);
    fire_ent->slide_ent(vec2(200+128, 200+128));
    // spawn a pile of sand
    ent* sand_ent = add_ent(ENT_SCENERY);
    sand_ent->set_anim(sand);
    sand_ent->slide_ent(vec2(200, 200+128));
#define NUM_WALL_BITS 8
    ent* wall_bits[NUM_WALL_BITS];
    // spawn lower layers of the wall
    for (int i=0; i<NUM_WALL_BITS-1; i++) {
        wall_bits[i] = add_ent(ENT_SCENERY);
        wall_bits[i]->set_anim(wall_steel_2);
        wall_bits[i]->slide_ent(vec2(128*5, 128*5));
    }
    
    
#define NUM_WALL_BITS_2 16
    ent* wall_bits_2[NUM_WALL_BITS_2];
    // spawn lower layers of the wall
    for (int i=0; i<NUM_WALL_BITS_2-1; i++) {
        wall_bits_2[i] = add_ent(ENT_SCENERY);
        wall_bits_2[i]->set_anim(wall_steel_2);
        wall_bits_2[i]->slide_ent(vec2(128*6, 128*5));
    }
#define NUM_WALL_BITS_3 32
    ent* wall_bits_3[NUM_WALL_BITS_3];
    // spawn lower layers of the wall
    for (int i=0; i<NUM_WALL_BITS_3-1; i++) {
        wall_bits_3[i] = add_ent(ENT_SCENERY);
        wall_bits_3[i]->set_anim(wall_steel_2);
        wall_bits_3[i]->slide_ent(vec2(128*7, 128*5));
    }
    // spawn top of the wall
    wall_bits_3[NUM_WALL_BITS_3-1] = add_ent(ENT_SCENERY);
    wall_bits_3[NUM_WALL_BITS_3-1]->set_anim(wall_steel);
    wall_bits_3[NUM_WALL_BITS_3-1]->slide_ent(vec2(128*7, 128*5));
    
    // spawn top of the wall
    wall_bits_2[NUM_WALL_BITS_2-1] = add_ent(ENT_SCENERY);
    wall_bits_2[NUM_WALL_BITS_2-1]->set_anim(wall_steel);
    wall_bits_2[NUM_WALL_BITS_2-1]->slide_ent(vec2(128*6, 128*5));
    
    // spawn top of the wall
    wall_bits[NUM_WALL_BITS-1] = add_ent(ENT_SCENERY);
    wall_bits[NUM_WALL_BITS-1]->set_anim(wall_steel);
    wall_bits[NUM_WALL_BITS-1]->slide_ent(vec2(128*5, 128*5));
    
    while (running) {
        dt = (SDL_GetTicks() - last_frame_end) / 1000;
        last_frame_end = SDL_GetTicks();
        track_fps();
        //
        // handle client inputs and movement
        //
        client_input(player_entity);
        player_accel(player_entity);
        //
        // move things according to their velocity
        //
        move(player_entity);
        
        wall_parallax(wall_bits, NUM_WALL_BITS, player_entity, vec2(128*5, 128*5));
        wall_parallax(wall_bits_2, NUM_WALL_BITS_2, player_entity, vec2(128*6, 128*5));
        wall_parallax(wall_bits_3, NUM_WALL_BITS_3, player_entity, vec2(128*7, 128*5));
        
        //
        // do collisions
        //
        player_entity->collide_ent_cs(rock_ent);
        gun_movement(player_entity, gun_ent);
        //
        // update the player's camera position
        //
        view_x = player_entity->get_pos().get_x();
        view_y = player_entity->get_pos().get_y();
        //
        // draw the world
        //
        draw_chunk(test_world.get_chunks());
        draw_ents(entities, num_entities);
    }
    printf("Server was running for %d seconds.\n", SDL_GetTicks() / 1000);
    cleanup_graphics();
    cleanup_audio();
    return 0;
}
