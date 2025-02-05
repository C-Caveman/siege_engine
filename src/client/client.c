// store all player-related information

#include "client.h"
#include "../graphics/graphics.h"
#include "../audio/audio.h"
extern float dt;
struct eventsBuffer clientEvents;


struct dialogActor actors[] = {
    {".", {typewriterA01, voiceThudA3}, {black, black} },
    {"pig", {voiceJolly02, voiceMetalB1}, {facePigTalk01, facePig01} },
    {"book", {voiceTickerTape02, tik}, {faceBookTalk01, faceBook01} },
    {"goat", {chuh02, chuh01}, {faceSkullGoat01, faceSkullGoat01} },
    {"robot", {voiceRobot02bb, voiceRobot01b}, {faceRobot01, faceRobot01} },
    {"", {0}, {0}} // null terminator
};
char dialogAnnotationTypeNames[][MAX_ANNOTATION_LEN] = {
    dialogAnnotationTypesList(TO_STRING)
};
char* nameOfAnnotationType(int t) {
    if (t >= 0 && t < NUM_DIALOG_ANNOTATION_TYPES)
        return (char*)&dialogAnnotationTypeNames[t];
    else
        return (char*)&dialogAnnotationTypeNames[invalidAnnotation];
}

///////////////////////////////////////////////////////////////////////////////////////////// Menus ;;
int menuSizes[NUM_MENU_PAGES] = {
    MENU_PAGES_LIST(TO_MENU_SIZE_INTS)
};
char menuPageNames[NUM_MENU_PAGES][MAX_MENU_ITEM_LEN] = {
    MENU_PAGES_LIST(TO_STRING)
};
char PAUSE_MENU_ITEMS[MAX_MENU_ITEMS][MAX_MENU_ITEM_LEN] = {
    PAUSE_MENU_LIST(TO_STRING)
};
char SETTINGS_MENU_ITEMS[MAX_MENU_ITEMS][MAX_MENU_ITEM_LEN] = {
    SETTINGS_MENU_LIST(TO_STRING)
};
#define TO_MENU_LISTING_ADDRESS(name) &(name##_ITEMS), 
char (*menuPages[NUM_MENU_PAGES])[MAX_MENU_ITEMS][MAX_MENU_ITEM_LEN] = {
    MENU_PAGES_LIST(TO_MENU_LISTING_ADDRESS)
};

vec2i getTileAtCursor(struct client* c) {
    if (c == 0) { printf("*** null client in getTileAtCursor!\n"); exit(-1); }
    return v2fToIRoundUp(v2fScalarDiv(v2fAdd(c->camera_center,v2iToF(c->aim_pixel_pos)), RSIZE));
}

#define PLAYER_ACCELERATION 4000
float DASH_ACCELERATION = PLAYER_ACCELERATION*3;
float BONUS_DASH_ACCELERATION = PLAYER_ACCELERATION*2;
void clientUpdatePlayerEntity() {
    // Player movement:
    if (playerClient.dashing && v2fLen(playerClient.accel_dir) > 0.1) {
        playerClient.aim_dir = vectorToAngle(playerClient.accel_dir);
        playerClient.keyboardAiming = true;
    }
    
    if (playerClient.dashing) {
        playerClient.player->vel = 
            v2fAdd(
                playerClient.player->vel, 
                v2fScale(angleToVector(playerClient.aim_dir), ((DASH_ACCELERATION+BONUS_DASH_ACCELERATION*(playerClient.player->heatTracker == HEAT_MAX))*dt))
            );
    }
    else {
        playerClient.player->vel = v2fAdd(
            playerClient.player->vel, 
            v2fScale(v2fNormalized(playerClient.accel_dir), ((PLAYER_ACCELERATION + playerClient.sprinting*PLAYER_ACCELERATION*1.25)*dt))
        );
    }
    if (v2fLen(playerClient.accel_dir) == 0 && !playerClient.dashing)
        playerClient.player->vel = v2fScale(playerClient.player->vel, (1 - dt*25)); // Add friction when no direction is held.
    // Gun direction:
    playerClient.player->sprites[PLAYER_GUN].rotation = playerClient.aim_dir;
    
    if (playerClient.attacking && (curFrameStart - playerClient.lastAttackTime) > 300 && playerClient.player->heatTracker < 1) {
        // Tell the server we are shooting:
        CE(PlayerShoot, playerClient.player->h, playerClient.player->pos, playerClient.aim_dir);
        // Show the shooting without waiting for the server:
        //evPlayerShoot(&(struct dPlayerShoot) {eventPlayerShoot, playerClient.player->h, playerClient.player->pos, playerClient.aim_dir});
    }
    if (playerClient.building && (curFrameStart - playerClient.lastBuildTime) > 50) {
        struct tile* timmy = worldGetTile(getTileAtCursor(&playerClient));
        for (int i=0; i<MAX_ENTS_PER_TILE; i++) {
            if (timmy == 0)
                break;
            if (timmy->ents[i] != 0) {
                entBasics* e = getEnt(timmy->ents[i], 0);
                if (e->type == gib_type)
                    despawnEnt(e);
                else
                    timmy = 0;
            }
        }
        if (timmy != 0 && timmy->wall_height <= 0) {
            playerClient.lastBuildTime = curFrameStart;
            CE(ChangeTile, .tileNumber=tileIndexToNumber(getTileAtCursor(&playerClient)), .floor=grass1Floor, .height=8, .wall=grass1Side, .wallSide=grass1Side);
            playSound(thud);
        }
    }
}

void clientStartDialog(char* message) {
    playerClient.dialogTick = anim_tick;
    strncpy(playerClient.dialogString, message, sizeof(playerClient.dialogString)-1);
    memset(playerClient.dialogPrintString, 0, sizeof(playerClient.dialogPrintString)-1);
    playerClient.dialogVisible = 1;
    playerClient.dialogCharsPrinted = 0;
    playerClient.dialogStringPos = 0;
    playerClient.dialogWaitTimer = 0;
    playerClient.dialogActorFrame = 0;
}

void clientChangeActor() {
    playerClient.dialogActorIndex = 0; // Set to default actor.
    playerClient.dialogActorFaceIndex = 0;
    playerClient.dialogActorVoiceIndex = 0;
    playerClient.dialogActorFrame = 0;
    int foundMatch = 0;
    for (int i=0; actors[i].name[0] != 0; i++) {
        int isMatch = strncmp(playerClient.dialogAnnotation, actors[i].name, MAX_ACTOR_NAME_LEN);
        if (isMatch == 0) {
            playerClient.dialogActorIndex = i;
            foundMatch = 1;
            break;
        }
    }
    if (foundMatch == 1) {
        //printf("[dialogActor = '%s']\n", actors[playerClient.dialogActorIndex].name);
    }
    else {
        printf("*** changeActor didn't find '%s'\n", playerClient.dialogAnnotation);
    }
}

void clientUpdateDialogue() { // Animate the dialog box.
    if (playerClient.dialogVisible == 0)
        return;
    int msSinceTextBoxUpdate = anim_tick - playerClient.dialogTick + (anim_tick < playerClient.dialogTick)*256;
    if (playerClient.dialogWaitTimer > 0) {
        playerClient.dialogWaitTimer -= msSinceTextBoxUpdate;
        playerClient.dialogTick = anim_tick;
        msSinceTextBoxUpdate = 0;
        return;
    }
    int numTextBoxChars = strlen(playerClient.dialogString);
    //char prevChar = playerClient.dialogPrintString[(playerClient.dialogCharsPrinted > 0) ? playerClient.dialogCharsPrinted-1 : 0];
    char c = playerClient.dialogString[playerClient.dialogStringPos];
    if (c == '<') {
        playerClient.dialogStringPos++;
        playerClient.dialogAnnotationLen = 0;
        switch (playerClient.dialogString[playerClient.dialogStringPos]) {
            case 'a':
                playerClient.dialogAnnotationType = setActor;
                break;
            case 'f':
                playerClient.dialogAnnotationType = setFaceAnim;
                break;
            case 'v':
                playerClient.dialogAnnotationType = setVoice;
                break;
            case 'c':
                playerClient.dialogAnnotationType = clearDialog;
                break;
            case 'w':
                playerClient.dialogAnnotationType = waitDialog;
                break;
            default:
                playerClient.dialogAnnotationType = invalidAnnotation;
        }
        if (playerClient.dialogStringPos != -1)
            playerClient.dialogStringPos++;
        while (playerClient.dialogString[playerClient.dialogStringPos] != '>' && playerClient.dialogStringPos < MAX_DIALOG_LEN) {
            playerClient.dialogAnnotation[playerClient.dialogAnnotationLen] = playerClient.dialogString[playerClient.dialogStringPos];
            playerClient.dialogStringPos++;
            playerClient.dialogAnnotationLen++;
        }
        playerClient.dialogAnnotation[playerClient.dialogAnnotationLen] = 0;
        //printf("[%s] <%s>\n", nameOfAnnotationType(playerClient.dialogAnnotationType), playerClient.dialogAnnotation);
        switch (playerClient.dialogAnnotationType) {
            case setActor:
                clientChangeActor();
                break;
            case setFaceAnim:
                playerClient.dialogActorFaceIndex = atoi(playerClient.dialogAnnotation);
                //printf("[playerClient.dialogActorFaceIndex = %d]\n", playerClient.dialogActorFaceIndex);
                break;
            case setVoice:
                playerClient.dialogActorVoiceIndex = atoi(playerClient.dialogAnnotation);
                //printf("[playerClient.dialogActorVoiceIndex = %d]\n", playerClient.dialogActorVoiceIndex);
                break;
            case clearDialog:
                playerClient.dialogCharsPrinted = 0;
                memset(playerClient.dialogPrintString, 0, sizeof(playerClient.dialogPrintString)-1);
                break;
            case waitDialog:
                playerClient.dialogWaitTimer = atoi(playerClient.dialogAnnotation) * 100;
                break;
            default:
                break;
        }
        playerClient.dialogStringPos++;
        c = playerClient.dialogString[playerClient.dialogStringPos];
    }
    int isPunctuation = (c == '.' || c == '!' || c == '?');
    int isSpace = isspace(c);
    int timeToAddChar = (msSinceTextBoxUpdate > 70) && (playerClient.dialogWaitTimer <= 0);
    int waitedForPunct = !(isSpace && msSinceTextBoxUpdate < 140) && !(isPunctuation && msSinceTextBoxUpdate < 200);
    if (timeToAddChar && waitedForPunct && playerClient.dialogCharsPrinted < numTextBoxChars) {
        playerClient.dialogTick = anim_tick;
        playerClient.dialogPrintString[playerClient.dialogCharsPrinted] = c;
        playerClient.dialogPrintString[playerClient.dialogCharsPrinted+1] = 0;
        playerClient.dialogCharsPrinted++;
        playerClient.dialogStringPos++;
        if (!isSpace) {
            playSoundChannel(actors[playerClient.dialogActorIndex].voices[playerClient.dialogActorVoiceIndex], CHAN_VOICE);
            playerClient.dialogActorFrame++;
        }
        /*
        if (isPunctuation && rand() > RAND_MAX/2)
            playSoundChannel(typewriterAPunct2, 5);
        else if (isPunctuation && rand() > RAND_MAX/4)
            playSoundChannel(typewriterAPunct3, 5);
        else if (isPunctuation)
            playSoundChannel(typewriterAPunct1, 5);
        else if (c != ' ' && rand() > RAND_MAX/2)
            playSoundChannel(typewriterARattle1, rand() % 4);
        else if (c != ' ' && prevChar == ' ')
            playSoundChannel(typewriterARattle2, 4);
        else if (c != ' ' && rand() < RAND_MAX/1.1 && prevChar == ' ')
            playSoundChannel(typewriterARattle1, rand() % 4);
        else if (c != ' ')
            playSoundChannel(typewriterARattle2, rand() % 4);
        */
    }
    if (playerClient.dialogStringPos == numTextBoxChars) {
        playerClient.dialogCharsPrinted = 0;
        playerClient.dialogVisible = 0;
        return;
    }
}

void clientShowDialog() { // Draw an animated dialog string onto the screen.
    if (playerClient.dialogVisible == 0)
        return;
    clientUpdateDialogue();
    drawDialogBox(&playerClient);
}

void clientLoadDialog(char* fName) {
    FILE* fp = fopen(fName, "r");
    if (!fp) {
        printf("Couldn't open %s\n", fName);
        exit(1);
    }
    for (int i=0; i < MAX_DIALOG_LEN; i++) {
        char c = fgetc(fp);
        if (feof(fp))
            break;
        playerClient.loadedDialog[i] = c;
    } 
}

void clientMenuMoveUp() {
    playerClient.menuSelection[playerClient.menuPage]--;
    playSound(click01);
    if (playerClient.menuSelection[playerClient.menuPage] < 0)
        playerClient.menuSelection[playerClient.menuPage] = 0;
    if (playerClient.menuSelection[playerClient.menuPage] >= menuSizes[playerClient.menuPage])
        playerClient.menuSelection[playerClient.menuPage] = menuSizes[playerClient.menuPage]-1;
}
void clientMenuMoveDown() {
    playerClient.menuSelection[playerClient.menuPage]++;
    playSound(click01);
    if (playerClient.menuSelection[playerClient.menuPage] < 0)
        playerClient.menuSelection[playerClient.menuPage] = 0;
    if (playerClient.menuSelection[playerClient.menuPage] >= menuSizes[playerClient.menuPage])
        playerClient.menuSelection[playerClient.menuPage] = menuSizes[playerClient.menuPage]-1;
}
void clientSelectMenuItem() {
    int selection = playerClient.menuSelection[playerClient.menuPage];
    //printf("Menu '%s' size %d\n", playerClient.menuPageNames[playerClient.menuPage], menuSizes[playerClient.menuPage]);
    switch(playerClient.menuPage) {
        case PAUSE_MENU:
            switch(selection) {
                case menuResume:
                    playerClient.paused = false;
                    break;
                case menuSettings:
                    playerClient.menuPage = SETTINGS_MENU;
                    break;
                case menuQuit:
                    playerClient.quitting = true;
                    running = false;
                    break;
            }
            break;
        
        case SETTINGS_MENU:
            switch(selection) {
                case menuMusicVolumeUp:
                    musicVolume = fclamp(musicVolume+0.1, 0, 1);
                    setMusicVolume(musicVolume);
                    break;
                case menuMusicVolumeDown:
                    musicVolume = fclamp(musicVolume-0.1, 0, 1);
                    setMusicVolume(musicVolume);
                    break;
                case menuSfxVolume:
                    sfxVolume = 0;
                    setSfxVolume(sfxVolume);
                    break;
                case menuvoiceVolume:
                    break;
                case menuFullscreen:
                    if (fullscreen)
                        goWindowed();
                    else
                        goFullscreen();
                    break;
            }
            break;
    }
}
