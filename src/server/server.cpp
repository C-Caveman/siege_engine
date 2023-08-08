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
    struct ent* client_ent  = add_ent(type_player);
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
    ent* rock_ent =  add_ent(type_scenery);
    rock_ent->set_anim(stone);
    rock_ent->slide_ent(vec2(200, 200));
    // spawn the gun
    ent* gun_ent = add_ent(type_gun);
    gun_ent->set_anim(gun_grenade);
    gun_ent->slide_ent(vec2(200+128, 200));
    // spawn a firepit
    ent* fire_ent = add_ent(type_scenery);
    fire_ent->set_anim(firepit);
    fire_ent->slide_ent(vec2(200+128, 200+128));
    // spawn a pile of sand
    ent* sand_ent = add_ent(type_scenery);
    sand_ent->set_anim(sand);
    sand_ent->slide_ent(vec2(200, 200+128));
    
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
