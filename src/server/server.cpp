// update the world using messages from the clients,
// update the clients on what has happened (if multiplayer)

#include "server.h"
#include <unistd.h> // testing memory leaks with sleep()

extern bool m1_held;

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
    entities[num_entities] = ent (new_id(), type, rocket_tank, 0, 0, 2, vec2f{0,0}, vec2f{0,0}, vec2f{0,0});
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
    e_displayer->move_ent(e->get_pos() );//+ vec2f(mouse_x,mouse_y));
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
    chunk* chunk_0 = test_world.get_chunk(0,0);
    chunk_0->set_floors(floor_test);
    chunk_0->set_floor(0,0, tiledark);
    chunk_0->set_wall(1,4, wall_steel,wall_steel_side,8);
    chunk_0->set_wall(3,4, wall_steel,wall_steel_side,8);
    chunk_0->set_wall(7,7, wall_steel,wall_steel_side,8);
    chunk_0->set_wall(8,7, wall_steel,wall_steel_side,16);
    // spawn a rock
    ent* rock_ent =  add_ent(ENT_SCENERY);
    rock_ent->set_anim(stone);
    rock_ent->slide_ent(vec2f{RSIZE*3, RSIZE*1});
    // spawn the gun
    ent* gun_ent = add_ent(ENT_GUN);
    gun_ent->set_anim(gun_grenade);
    gun_ent->slide_ent(vec2f{RSIZE*5, RSIZE*1});
    // spawn a firepit
    ent* fire_ent = add_ent(ENT_SCENERY);
    fire_ent->set_anim(firepit);
    fire_ent->slide_ent(vec2f{RSIZE*7, RSIZE*1});
    // spawn a pile of sand
    ent* sand_ent = add_ent(ENT_SCENERY);
    sand_ent->set_anim(sand);
    sand_ent->slide_ent(vec2f{RSIZE*10, RSIZE*1});
    
    /*
    printf("Each segment is %ld bits.\n", sizeof(segment) * 8);
    printf("The player entity is %d segments long.\n", get_ent_size(PLAYER));
    constexpr int SEGMENT_ARRAY_SIZE = 4096;
    segment entity_segment_array[SEGMENT_ARRAY_SIZE];
    ent_PLAYER* p = (ent_PLAYER*)spawn_ent(PLAYER, entity_segment_array, SEGMENT_ARRAY_SIZE);
    printf("Ent type: %d, VEL: %f\n", p->data[head].head.type, p->data[vel].vel.vel.x);
    ent_SCENERY* s = (ent_SCENERY*)spawn_ent(SCENERY, entity_segment_array, SEGMENT_ARRAY_SIZE);
    despawn_ent((segment*)s);
    ent_SCENERY* S = (ent_SCENERY*)spawn_ent(SCENERY, entity_segment_array, SEGMENT_ARRAY_SIZE);
    */
    constexpr int SEGMENT_ARRAY_SIZE = 4096;
    segment entity_segment_array[SEGMENT_ARRAY_SIZE];
    ent_PLAYER* p = (ent_PLAYER*)spawn_ent(PLAYER, entity_segment_array, SEGMENT_ARRAY_SIZE);
    ent_SCENERY* s = (ent_SCENERY*)spawn_ent(SCENERY, entity_segment_array, SEGMENT_ARRAY_SIZE);
    printf("*Type name: %s\n", get_type_name(p->data[0].head.type));
    int second_ent_index = get_next_ent(0, entity_segment_array, SEGMENT_ARRAY_SIZE);
    printf("*Type name: %s\n", get_type_name(entity_segment_array[second_ent_index].head.type));
    draw_ent_sprites((segment*) p);
    draw_ent_sprites((segment*) &entity_segment_array[second_ent_index]);
    
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
        
        if (m1_held)
            place_wall(chunk_0);
        
        //
        // draw the world
        //
        draw_chunk(test_world.get_chunk(0,0), vec2f{view_x, view_y});
        draw_ents(entities, num_entities);
    }
    printf("Server was running for %d seconds.\n", SDL_GetTicks() / 1000);
    cleanup_graphics();
    cleanup_audio();
    return 0;
}
