// store all player-related information

#include "client.h"


float PLAYER_ACCELARATION = 10;
// TODO remove the old client system entirely
void client_data::update_player_entity() {
    player[vel].vel.vel = player[vel].vel.vel + (accel_dir.normalized() * (PLAYER_ACCELARATION + sprinting*PLAYER_ACCELARATION*2));
    if (accel_dir.vlen() == 0)
        player[vel].vel.vel = player[vel].vel.vel * 0.95; // Add friction when no direction is held.
}
