// store all player-related information

#include "client.h"
extern float dt;

float PLAYER_ACCELERATION = 4000;
void client::update_player_entity() {
    // Player movement:
    player->vel = player->vel + (accel_dir.normalized() * (PLAYER_ACCELERATION + sprinting*PLAYER_ACCELERATION*1.25)*dt);
    float dotProd = accel_dir.dot(player->vel);
    if (dotProd < 0) {
        //printf("reverse!\n");
        //player->vel = player->vel - (accel_dir * dotProd);
    }
    //std::cout << "pos: " << player->pos << "\n";
    if (accel_dir.vlen() == 0)
        player->vel = player->vel * (1 - dt*25); // Add friction when no direction is held.
    // Gun direction:
    player->sprites[PLAYER_GUN].rotation = aim_dir;
}
