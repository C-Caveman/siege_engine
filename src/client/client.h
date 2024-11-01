// store all player-related information
#ifndef CLIENT
#define CLIENT

#include <stdio.h>
#include "../ent/ent.h"

#define MAX_DIALOG_LEN 2048
#define MAX_ANNOTATION_LEN 64

#define dialogAnnotationTypesList(f) \
    f(invalidAnnotation) \
    f(setActor) \
    f(setFaceAnim) \
    f(setVoice) \
    f(clearDialog) \
    f(waitDialog)
enum dialogAnnotationTypes {
    dialogAnnotationTypesList(TO_ENUM)
    NUM_DIALOG_ANNOTATION_TYPES
};

#define MAX_ACTOR_NAME_LEN 128
#define MAX_ACTOR_VOICES 128
#define MAX_ACTOR_ANIMS 256
struct dialogActor { // Set by annotations in the dialog strings.
    char name[MAX_ACTOR_NAME_LEN];
    int voices[MAX_ACTOR_VOICES];
    int anim[MAX_ACTOR_ANIMS];
};
extern struct dialogActor actors[];

struct client {
    void update_player_entity();
    void startDialog(char* message);
    void updateDialogue();
    void showDialog();
    void changeActor();
    // Player entity:
    struct ent_player* player;
    //
    // Current input state:
    //
    bool keyboardAiming;
    bool attacking;
    bool building;
    bool interacting;
    bool sprinting; // Desired movement speed.
    bool quitting; // Quit button.
    vec2f accel_dir; // Desired move direction.
    float jerk; // Rate of acceleration.
    float aim_dir_rotation; // Desired auto rotation for aim.
    float aim_dir; // Desired aim direction.
    float aimSpeed; // Keyboard aim sensitivity.
    vec2i aim_pixel_pos; // Current pixel the cursor is on.
    // Current camera position:
    vec2f camera_pos; // Top-left corner of the camera.
    vec2f camera_center; // Center of the camera.
    int lastAttackTime; // Last time the player attacked.
    int lastBuildTime; // Last time the player built.
    int  dialogWaitTimer; // Pause time set by the <wait> annotation.
    int  dialogTick;
    int  dialogVisible;
    int  dialogCharsPrinted;
    int  dialogStringPos;
    int  dialogAnnotationLen;
    int  dialogAnnotationType;
    char dialogString[MAX_DIALOG_LEN];      // Dialog with <annotations> included.
    char dialogPrintString[MAX_DIALOG_LEN]; // Printed text with <annotations> removed.
    char dialogAnnotation[MAX_ANNOTATION_LEN]; // Current <annotation> being read in from the dialogString.
    int  dialogActorIndex;      // Array index of the current actor who is speaking.
    int  dialogActorFaceIndex;  // Actor's current face animation.
    int  dialogActorVoiceIndex; // Actor's current talk sound.
    int  dialogActorFrame;      // Frame offset for the face animation.
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
