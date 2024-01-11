// store all player-related information

#include "client.h"

float PLAYER_ACCELERATION = 10;
void client_data::update_player_entity() {
    // Player movement:
    player[vel].vel.vel = player[vel].vel.vel + (accel_dir.normalized() * (PLAYER_ACCELERATION + sprinting*PLAYER_ACCELERATION*2));
    if (accel_dir.vlen() == 0)
        player[vel].vel.vel = player[vel].vel.vel * 0.95; // Add friction when no direction is held.
    // Gun direction:
    player[p_sprite_gun_anim].anim.rotation = aim_dir;
}
