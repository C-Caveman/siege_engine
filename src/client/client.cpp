// store all player-related information

#include "client.h"

float PLAYER_ACCELERATION = 10;
void client::update_player_entity() {
    // Player movement:
    player->vel = player->vel + (accel_dir.normalized() * (PLAYER_ACCELERATION + sprinting*PLAYER_ACCELERATION*2*4));
    if (accel_dir.vlen() == 0)
        player->vel = player->vel * 0.95; // Add friction when no direction is held.
    // Gun direction:
    player->sprites[PLAYER_GUN].rotation = aim_dir;
}
