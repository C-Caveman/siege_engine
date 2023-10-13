// store all player-related information
#ifndef CLIENT
#define CLIENT

#include <stdio.h>
#include "../ent/ent.h"


// TODO replace the old client system TODO
struct client_data {
    void update_player_entity();
    // Player entity:
    segment* player;
    // Desired move direction:
    vec2f accel_dir;
    // Desired movement speed:
    bool sprinting;
    // Desired auto rotation for aim:
    float aim_dir_rotation;
    // Desired aim direction:
    float aim_dir;
    // Desired aim position:
    vec2f aim_pos;
    // Misc commands:
    bool quitting;
};

// TODO use these for packets TODO
enum command_types {
    MOVE,
    SHOOT
};
struct command {
    uint32_t type;
    uint32_t sequence_number;
    union {
        vec2f vec2f_data;
        vec2i vec2i_data;
        uint32_t int_data;
        float float_data;
    };
};


#endif
