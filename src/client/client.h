// store all player-related information
#ifndef CLIENT
#define CLIENT

#include <stdio.h>
#include "../ent/ent.h"


struct client {
    void update_player_entity();
    void startDialog(char* message);
    void updateDialogue();
    void showDialog();
    // Player entity:
    struct ent_player* player;
    //
    // Current input state:
    //
    bool attacking;
    bool building;
    bool sprinting; // Desired movement speed.
    bool quitting; // Quit button.
    vec2f accel_dir; // Desired move direction.
    float jerk; // Rate of acceleration.
    float aim_dir_rotation; // Desired auto rotation for aim.
    float aim_dir; // Desired aim direction.
    vec2i aim_pixel_pos; // Current pixel the cursor is on.
    // Current camera position:
    vec2f camera_pos; // Top-left corner of the camera.
    vec2f camera_center; // Center of the camera.
    int lastAttackTime; // Last time the player attacked.
    int lastBuildTime; // Last time the player built.
    int  dialogTick;
    int  dialogVisible;
    int  dialogCharsPrinted;
    int  dialogStringPos;
    char dialogString[1024];
    char dialogPrintString[1024];
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
