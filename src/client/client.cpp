// store all player-related information

#include "client.h"
#include "../graphics/graphics.h"
#include "../audio/audio.h"
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

void client::startDialog(char* message) {
    dialogTick = anim_tick;
    strncpy(dialogString, message, sizeof(dialogString)-1);
    dialogVisible = 1;
    dialogCharsPrinted = 0;
}

void client::showDialog() { // Draw an animated dialog string onto the screen.
    if (dialogVisible == 0)
        return;
    int msSinceTextBoxUpdate = anim_tick - dialogTick + (anim_tick < dialogTick)*256;
    int numTextBoxChars = strlen(dialogString);
    char prevChar = dialogString[(dialogCharsPrinted > 0) ? dialogCharsPrinted-1 : 0];
    char c = dialogString[dialogCharsPrinted];
    int isPunctuation = (c == '.' || c == '!' || c == '?');
    int isSpace = (c == ' ');
    if (msSinceTextBoxUpdate > 70 && !(isSpace && msSinceTextBoxUpdate < 140) && !(isPunctuation && msSinceTextBoxUpdate < 200) && dialogCharsPrinted < numTextBoxChars) {
        dialogTick = anim_tick;
        dialogCharsPrinted++;
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
    }
    if (dialogCharsPrinted == numTextBoxChars) {
        dialogCharsPrinted = 0;
        dialogVisible = 0;
        return;
    }
    drawTextBox((char*)&dialogString, dialogCharsPrinted);
}
