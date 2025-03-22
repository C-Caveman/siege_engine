// Handle player inputs, draw the screen, and accept server events.

#include "client.h"
#include "../client/graphics.h"
#include "../client/audio.h"
extern volatile float clientDt;
struct eventsBuffer clientCmdEvents = {0};
struct eventsBuffer clientEvents = {0};
#define logDialog(...) if (DEBUG_DIALOG) { printf(__VA_ARGS__); }

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

///////////////////////////////////////////////////////////////////////////////////////////// Loop ;;
#define logThread(...) {\
    if (DEBUG_THREADS) \
        printf( __VA_ARGS__ );\
}
#define logClient(...) {\
    if (DEBUG_CLIENT) \
        printf( __VA_ARGS__ );\
}
void clientInput(struct client* c);
// Client thread (draw the screen, read inputs):
uint32_t clientFrame = 0;
volatile float clientDt = 0;
volatile uint32_t frameStartTime = 0;

void sendCommandsToServer() { //TODO ADD MULTIPLAYER PATH HERE!!! TODO
    int cmdEventsSent = 0;
    while (clientCmdEvents.count > 0 && serverEvents.count <= EVENT_BUFFER_SIZE-1) {
        memcpy(&serverEvents.buffer[serverEvents.writeHead], &clientCmdEvents.buffer[clientCmdEvents.readHead], sizeof(clientCmdEvents.buffer[0]));
        memset(&clientCmdEvents.buffer[clientCmdEvents.readHead], 0, sizeof(clientCmdEvents.buffer[0]));
        clientCmdEvents.readHead++;
        serverEvents.writeHead++;
        if (clientCmdEvents.readHead >= EVENT_BUFFER_SIZE-1)
            clientCmdEvents.readHead = 0;
        if (serverEvents.writeHead >= EVENT_BUFFER_SIZE-1)
            serverEvents.writeHead = 0;
        clientCmdEvents.count--;
        cmdEventsSent++;
    }
    sem_wait(&eventCountMutex);
    serverEvents.count += cmdEventsSent;
    sem_post(&eventCountMutex);
}

void* clientLoop() {
    logThread("Client thread enabled!\n");
    init_graphics();
    init_audio();
    // Connect to the server:
    playerClient.id = 101;
    playerClient.address = 10101010;
    CE(ClientHello, playerClient.id, playerClient.address);
    sendCommandsToServer();
    // Input and rendering loop:
    frameStartTime = SDL_GetTicks();
    while (running) {
        anim_tick = SDL_GetTicks() % 256; //- 8-bit timestamp for animations.
        clientDt = ((float)SDL_GetTicks() - (float)frameStartTime) / 1000.f;
        if (clientDt < 0) {
            printf("clientDt was negative!!!!: %f\n", clientDt);
            exit(0);
        }
        frameStartTime = SDL_GetTicks();
        //
        // Player input:
        //
        clientInput(&playerClient);
        clientUpdatePlayerEntity();
        sendCommandsToServer();
        
        
        /* TODO events go here!!!!!!!!!!!!!!!
        while (clientEvents.count > 0) {
            takeClientEvent();
        }
        // TODO use client data for rendering, not the server data!!
        */
        
        SDL_RenderClear(renderer);
        if (!playerClient.paused) {
            // Clientside animations:
            animateAllEnts(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
            drawWorld(mainWorld);
            // DRAW A HUD!
            drawInfo((char*)"fps", fps, 0);
            drawInfo((char*)"heat", (float)playerClient.player->heatTracker, 1);
            drawInfo((char*)"zombies", (float)mainWorld->numZombies, 2);
            clientShowDialog();
        }
        else {
            renderMenu(&playerClient);
        }
        SDL_RenderPresent(renderer);
        trackFps();
        
        // Screen and inputs updated, now sleep until it's time for the next frame:
        uint32_t frameEndTime = SDL_GetTicks();
        uint32_t frameTimeElapsed = frameEndTime - frameStartTime;
        uint32_t sleepTime = (1000 / fps_cap) - frameTimeElapsed; // millis to sleep
        #define MAX_CLIENT_SLEEP_TIME (1000 / 30)
        sleepTime = fclamp(sleepTime, 0, MAX_CLIENT_SLEEP_TIME);
        SDL_Delay(sleepTime);
        clientFrame++;
    }
    logThread("Client thread exiting.\n");
    return 0;
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
                v2fScale(angleToVector(playerClient.aim_dir), ((DASH_ACCELERATION+BONUS_DASH_ACCELERATION*(playerClient.player->heatTracker == HEAT_MAX))*clientDt))
            );
    }
    else {
        playerClient.player->vel = v2fAdd(
            playerClient.player->vel, 
            v2fScale(v2fNormalized(playerClient.accel_dir), ((PLAYER_ACCELERATION + playerClient.sprinting*PLAYER_ACCELERATION*1.25)*clientDt))
        );
    }
    if (v2fLen(playerClient.accel_dir) == 0 && !playerClient.dashing)
        playerClient.player->vel = v2fScale(playerClient.player->vel, (1 - clientDt*25)); // Add friction when no direction is held.
    // Gun direction:
    playerClient.player->sprites[PLAYER_GUN].rotation = playerClient.aim_dir;
    
    if (playerClient.attacking && (frameStartTime - playerClient.lastAttackTime) > 300 && playerClient.player->heatTracker < 1) {
        // Tell the server we are shooting:
        CE(PlayerShoot, playerClient.player->h, playerClient.player->pos, playerClient.aim_dir);
        playerClient.lastAttackTime = frameStartTime;
        // Show the shooting without waiting for the server:
        //evPlayerShoot(&(struct dPlayerShoot) {eventPlayerShoot, playerClient.player->h, playerClient.player->pos, playerClient.aim_dir});
    }
    if (playerClient.building && (frameStartTime - playerClient.lastBuildTime) > 50) {
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
            playerClient.lastBuildTime = frameStartTime;
            CE(ChangeTile, .tileNumber=tileIndexToNumber(getTileAtCursor(&playerClient)), .floor=grass1Floor, .height=8, .wall=grass1Side, .wallSide=grass1Side);
            playSound(thud);
        }
    }
    // Predict movement:
    if (SINGLEPLAYER_HACK) {
        moveOneEnt((entBasics*)playerClient.player, clientDt);
        collideWall((entBasics*)playerClient.player);
    }
    // Tell the server how we've moved:
    //CE(PlayerMove, .p=playerClient.player->h, .pos=playerClient.player->pos, .vel=playerClient.player->vel);
    //CE(SpriteRotate, .h=playerClient.player->h, .index=PLAYER_GUN, .angle=playerClient.aim_dir);
}

void clientClearDialog() {
    memset(playerClient.dialogString, 0, sizeof(playerClient.dialogString)-1);
    memset(playerClient.dialogPrintString, 0, sizeof(playerClient.dialogPrintString)-1);
    playerClient.dialogVisible = 0;
    playerClient.dialogCharsPrinted = 0;
    playerClient.dialogStringPos = 0;
    // Set to default actor.
    playerClient.dialogActorIndex = 0;
    playerClient.dialogActorFaceIndex = 0;
    playerClient.dialogActorVoiceIndex = 0;
    playerClient.dialogActorFrame = 0;
}

#define DEFAULT_WAIT_TIME 75
void clientStartDialog(char* message) {
    logDialog("dialog starting!\n");
    playerClient.waitTime = DEFAULT_WAIT_TIME;
    timerStart(&playerClient.waitTimer);
    strncpy(playerClient.dialogString, message, sizeof(playerClient.dialogString)-1);
    memset(playerClient.dialogPrintString, 0, sizeof(playerClient.dialogPrintString)-1);
    playerClient.dialogVisible = 1;
    playerClient.dialogCharsPrinted = 0;
    playerClient.dialogStringPos = 0;
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
    timerUpdate(&playerClient.waitTimer, playerClient.waitTime);
    bool timeToPrint = false;
    if (playerClient.waitTimer.count > 0) {
        timeToPrint = true;
        timerStart(&playerClient.waitTimer);
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
                playerClient.waitTime = atoi(playerClient.dialogAnnotation);
                break;
            default:
                break;
        }
        playerClient.dialogStringPos++;
        c = playerClient.dialogString[playerClient.dialogStringPos];
    }
    //int isPunctuation = (c == '.' || c == '!' || c == '?');
    int isSpace = isspace(c);
    //int waitedForPunct = !(isSpace && !passedTimestamp(playerClient.waitTimer.start + 140)) && !(isPunctuation && !passedTimestamp(playerClient.waitTimer.start + 200));
    if (timeToPrint && playerClient.dialogCharsPrinted < numTextBoxChars) {
        playerClient.dialogPrintString[playerClient.dialogCharsPrinted] = c;
        playerClient.dialogPrintString[playerClient.dialogCharsPrinted+1] = 0;
        playerClient.dialogCharsPrinted++;
        playerClient.dialogStringPos++;
        if (!isSpace) {
            playSoundChannel(actors[playerClient.dialogActorIndex].voices[playerClient.dialogActorVoiceIndex], CHAN_VOICE);
            playerClient.dialogActorFrame++;
        }
    }
    if (playerClient.dialogStringPos == numTextBoxChars) {
        clientClearDialog();
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
