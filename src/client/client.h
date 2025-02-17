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

//////////////////////////////////////////////////// Menus ;;
#define PAUSE_MENU_LIST(f) \
    f(Resume) \
    f(Settings) \
    f(Quit)
#define SETTINGS_MENU_LIST(f) \
    f(MusicVolumeUp) \
    f(MusicVolumeDown) \
    f(SfxVolume) \
    f(voiceVolume) \
    f(Fullscreen)
#define MENU_PAGES_LIST(f) \
    f(PAUSE_MENU) \
    f(SETTINGS_MENU)
    
#define TO_MENU_PREFIXED_ENUM(name) menu##name, 
enum pauseMenuEnum {
    PAUSE_MENU_LIST(TO_MENU_PREFIXED_ENUM)
    NUM_PAUSE_MENU_ITEMS
};
enum settingsMenuEnum {
    SETTINGS_MENU_LIST(TO_MENU_PREFIXED_ENUM)
    NUM_SETTINGS_MENU_ITEMS
};
enum menuPagesEnum {
    MENU_PAGES_LIST(TO_ENUM)
    NUM_MENU_PAGES
};
#define TO_MENU_SIZE_INTS(name) NUM_##name##_ITEMS, 
#define MAX_MENU_ITEM_LEN 256
#define MAX_MENU_ITEMS 64
extern int menuSizes[NUM_MENU_PAGES];
extern char (*menuPages[NUM_MENU_PAGES])[MAX_MENU_ITEMS][MAX_MENU_ITEM_LEN];
extern char menuPageNames[NUM_MENU_PAGES][MAX_MENU_ITEM_LEN];
extern char PAUSE_MENU_ITEMS[MAX_MENU_ITEMS][MAX_MENU_ITEM_LEN];
extern char SETTINGS_MENU_ITEMS[MAX_MENU_ITEMS][MAX_MENU_ITEM_LEN];

struct client {
    // Player entity:
    struct ent_player* player;
    //
    // Current input state:
    //
    bool paused;
    bool keyboardAiming;
    bool attacking;
    bool building;
    bool interacting;
    bool sprinting; // Desired movement speed.
    bool dashing;
    bool quitting; // Quit button.
    bool zombieSpawning;
    bool explodingEverything;
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
    //
    // Current dialog state:
    //
    int  dialogWaitTimer; // Pause time set by the <wait> annotation.
    int  dialogTick;
    int  dialogVisible;
    int  dialogCharsPrinted;
    int  dialogStringPos;
    int  dialogAnnotationLen;
    int  dialogAnnotationType;
    char loadedDialog[MAX_DIALOG_LEN];      // Dialog loaded by loadDialog().
    char dialogString[MAX_DIALOG_LEN];      // Dialog with <annotations> included.
    char dialogPrintString[MAX_DIALOG_LEN]; // Printed text with <annotations> removed.
    char dialogAnnotation[MAX_ANNOTATION_LEN]; // Current <annotation> being read in from the dialogString.
    int  dialogActorIndex;      // Array index of the current actor who is speaking.
    int  dialogActorFaceIndex;  // Actor's current face animation.
    int  dialogActorVoiceIndex; // Actor's current talk sound.
    int  dialogActorFrame;      // Frame offset for the face animation.
    //
    // Current menu state:
    //
    char (*menuText)[MAX_MENU_ITEMS][MAX_MENU_ITEM_LEN];
    int menuPage;
    int menuSelection[NUM_MENU_PAGES]; // Current selected menu item for each menu page.
};
void clientUpdatePlayerEntity();
void clientClearDialog();
void clientStartDialog(char* message);
void clientUpdateDialogue();
void clientShowDialog();
void clientChangeActor();
void clientLoadDialog(char* fName);
void clientSelectMenuItem();
void clientMenuMoveUp();
void clientMenuMoveDown();

#endif
