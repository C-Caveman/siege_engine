// how entities do things 

#include "../ent/actions.h"
extern float dt;

// accel the player entity using the desired direction and move_mode
void player_accel(ent* e) {
    // get desired movement direction
    vec2f player_desired_dir = e->get_dir();
    player_desired_dir = player_desired_dir.normalized();
    // get current velocity
    vec2f player_velocity = e->get_vel();
    //
    if (player_desired_dir.dot(player_velocity) < 0) { // switching direction
        player_desired_dir = player_desired_dir * 2;   // give a boost
    }
    //
    // check whether the player is walking
    int move_mode = abs(e->get_state(state_player_speed)) + 1;
    // accelerate in the desired direction
    player_velocity = player_velocity 
                      + player_desired_dir
                      * (move_mode*P_ACCEL)
                      * dt;
    // get the player's manhattan speed
    float man_speed = abs(player_velocity.get_x()) + abs(player_velocity.get_y());
    // TODO implement walking/running using the move_mode variable
    // apply friction
    if (man_speed < MIN_SPEED) {
        player_velocity = vec2f{0,0};
    }
    else 
        //player_velocity = player_velocity * (1-0.08);
        player_velocity = player_velocity - (player_velocity * FRICTION * dt);
    // update player's velocity
    e->set_vel(player_velocity);
}

// ent-ent collisions
void push(ent* e) {}

// use velocity to move entities
void move(ent* e) {
    e->slide_ent(e->get_vel() * dt);
}
