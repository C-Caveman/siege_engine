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
    //
    // Current input state:
    //
    bool attacking;
    vec2f accel_dir; // Desired move direction.
    bool sprinting; // Desired movement speed.
    float aim_dir_rotation; // Desired auto rotation for aim.
    float aim_dir; // Desired aim direction.
    vec2i aim_pixel_pos; // Current pixel the cursor is on.
    bool quitting; // Quit button.
    // Current camera position:
    vec2f camera_pos; // Top-left corner of the camera.
    vec2f camera_center; // Center of the camera.
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
