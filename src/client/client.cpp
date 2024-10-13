// store all player-related information

#include "client.h"
#include "../graphics/graphics.h"
#include "../audio/audio.h"
extern float dt;


struct dialogActor actors[] = {
    {".", {typewriterA01, voiceThudA3}, {black, black} },
    {"pig", {tik, voiceMetalB1}, {facePigTalk01, facePig01} },
    {"book", {tikLower, tik}, {faceBookTalk01, faceBook01} },
    {"", {}, {}} // null terminator
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

void client::startDialog(char* message) {
    dialogTick = anim_tick;
    strncpy(dialogString, message, sizeof(dialogString)-1);
    memset(dialogPrintString, 0, sizeof(dialogPrintString)-1);
    dialogVisible = 1;
    dialogCharsPrinted = 0;
    dialogStringPos = 0;
}

void client::changeActor() {
    dialogActorIndex = 0; // Set to default actor.
    dialogActorFaceIndex = 0;
    dialogActorVoiceIndex = 0;
    int foundMatch = 0;
    for (int i=0; actors[i].name[0] != 0; i++) {
        int isMatch = strncmp(dialogAnnotation, actors[i].name, MAX_ACTOR_NAME_LEN);
        if (isMatch == 0) {
            dialogActorIndex = i;
            foundMatch = 1;
            break;
        }
    }
    if (foundMatch == 1) {
        printf("[dialogActor = '%s']\n", actors[dialogActorIndex].name);
    }
    else {
        printf("*** changeActor didn't find '%s'\n", dialogAnnotation);
    }
}

void client::updateDialogue() { // Animate the dialog box.
    if (dialogVisible == 0)
        return;
    int msSinceTextBoxUpdate = anim_tick - dialogTick + (anim_tick < dialogTick)*256;
    int numTextBoxChars = strlen(dialogString);
    //char prevChar = dialogPrintString[(dialogCharsPrinted > 0) ? dialogCharsPrinted-1 : 0];
    char c = dialogString[dialogStringPos];
    if (c == '<') {
        dialogStringPos++;
        dialogAnnotationLen = 0;
        switch (dialogString[dialogStringPos]) {
            case 'a':
                dialogAnnotationType = setActor;
                break;
            case 'f':
                dialogAnnotationType = setFaceAnim;
                break;
            case 'v':
                dialogAnnotationType = setVoice;
                break;
            case 'c':
                dialogAnnotationType = clearDialog;
                break;
            default:
                dialogAnnotationType = invalidAnnotation;
        }
        if (dialogStringPos != -1)
            dialogStringPos++;
        while (dialogString[dialogStringPos] != '>' && dialogStringPos < MAX_DIALOG_LEN) {
            dialogAnnotation[dialogAnnotationLen] = dialogString[dialogStringPos];
            dialogStringPos++;
            dialogAnnotationLen++;
        }
        dialogAnnotation[dialogAnnotationLen] = 0;
        printf("[%s] <%s>\n", nameOfAnnotationType(dialogAnnotationType), dialogAnnotation);
        switch (dialogAnnotationType) {
            case setActor:
                changeActor();
                break;
            case setFaceAnim:
                dialogActorFaceIndex = std::stoi(dialogAnnotation);
                printf("[dialogActorFaceIndex = %d]\n", dialogActorFaceIndex);
                break;
            case setVoice:
                dialogActorVoiceIndex = std::stoi(dialogAnnotation);
                printf("[dialogActorVoiceIndex = %d]\n", dialogActorVoiceIndex);
                break;
            case clearDialog:
                dialogCharsPrinted = 0;
                memset(dialogPrintString, 0, sizeof(dialogPrintString)-1);
                break;
            default:
                break;
        }
        dialogStringPos++;
        c = dialogString[dialogStringPos];
    }
    int isPunctuation = (c == '.' || c == '!' || c == '?');
    int isSpace = (c == ' ');
    int timeToAddChar = (msSinceTextBoxUpdate > 70);
    int waitedForPunct = !(isSpace && msSinceTextBoxUpdate < 140) && !(isPunctuation && msSinceTextBoxUpdate < 200);
    if (timeToAddChar && waitedForPunct && dialogCharsPrinted < numTextBoxChars) {
        dialogTick = anim_tick;
        dialogPrintString[dialogCharsPrinted] = c;
        dialogPrintString[dialogCharsPrinted+1] = 0;
        dialogCharsPrinted++;
        dialogStringPos++;
        if (!isSpace) {
            playSoundChannel(actors[dialogActorIndex].voices[dialogActorVoiceIndex], 5);
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
    if (dialogStringPos == numTextBoxChars) {
        dialogCharsPrinted = 0;
        dialogVisible = 0;
        return;
    }
}

void client::showDialog() { // Draw an animated dialog string onto the screen.
    if (dialogVisible == 0)
        return;
    updateDialogue();
    drawDialogBox(this);
}
