#include "input.h"
#include "../graphics/graphics.h"
#include "../audio/audio.h"

struct inputKeybindings inputs[NUM_INPUTS] = {
    {}, {SDLK_q},{SDLK_f},{SDLK_w},{SDLK_s},{SDLK_a},{SDLK_d}, {1073742049}
};
// Get the index of the input associated with a given key.
int inputFromKey(int scancode) {
    for (int i=0; i<NUM_INPUTS; i++) {
        for (int j=0; j<MAX_KEYS_PER_BIND; j++) {
            if (scancode == inputs[i].keys[j])
                return i;
        }
    }
    return enum_inputKeyUnbound;
}
#define MAX_INPUT_NAME_LEN 64
char inputNames[NUM_INPUTS][MAX_INPUT_NAME_LEN] = {
    INPUTS_LIST(TO_STRING)
};
void setBinding(int inputIndex, int keyCode) {
    inputs[inputIndex].keys[0] = keyCode;
}
int getBinding(int inputIndex) {
    return inputs[inputIndex].keys[0];
}

// detect the BOGUS x11 keyup event generated on keydown
bool just_keyed_down = false;
bool just_clicked = false;
bool mouse_moved = false;


void client_input(client* client) {
    SDL_Event event;
    
    //
    // check all of the keyboard and mouse events
    //
    while(SDL_PollEvent(&event)) {
        mouse_moved = false;
        just_keyed_down = false;
        just_clicked = false;
        int inputIndex = enum_inputKeyUnbound;
        if (event.type == SDL_QUIT) // window was closed (not a keyboard event)
            running = false;
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            inputIndex = inputFromKey(event.key.keysym.sym);
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.repeat == 1)
                continue;
            //printf("Key pressed: %s, (%d)\n", inputNames[inputIndex], event.key.keysym.sym);
            // used to detect bogus inputs
            just_keyed_down = true;
            if (client->paused) {
                int isBackingOut = false;
                int isSelecting = false;
                int isUsingPredefinedKeys = true;
                switch(event.key.keysym.sym) { // Arrow/enter keys override keybindings in the menu! ;;
                    case 1073741903: // RIGHT
                        isSelecting = true;
                        break;
                    case 1073741904: // LEFT
                        //client->paused = false;
                        isBackingOut = true;
                        break;
                    case 1073741905: // DOWN
                        client->menuMoveDown();
                        break;
                    case 1073741906: // UP
                        client->menuMoveUp();
                        break;
                    case 13: // RETURN
                    case 1073741912: // KP_ENTER
                        isSelecting = true;
                        break;
                    case 27: // ESCAPE
                        //inputIndex = enum_inputQuit;
                        break;
                    case 32: // SPACE
                        isSelecting = true;
                        break;
                    default:
                        isUsingPredefinedKeys = false;
                        break;
                }
                if (!isUsingPredefinedKeys) {
                    switch (inputIndex) {
                        case enum_inputKeyUnbound:
                            break;
                        case enum_inputMoveUp:
                            client->menuMoveUp();
                            break;
                        case enum_inputMoveDown:
                            client->menuMoveDown();
                            break;
                        case enum_inputMoveLeft:
                            //client->paused = false;
                            isBackingOut = true;
                            break;
                        case enum_inputMoveRight:
                        case enum_inputAttack:
                        case enum_inputInteract:
                            isSelecting = true;
                            break;
                            
                        default:
                            break;
                    }
                }
                if (isSelecting) {
                    printf("Page '%s' item '%s'\n", menuPageNames[client->menuPage], menuPages[client->menuPage][0][client->menuSelection[client->menuPage]]);
                    playSound(click02);
                    client->selectMenuItem();
                }
                if (isBackingOut && client->menuPage != PAUSE_MENU) {
                    client->menuPage = PAUSE_MENU;
                    playSound(click02);
                }
                else if (isBackingOut) {
                    client->paused = false;
                }
            }
            
            // Input just activated:
            switch (inputIndex) {
                case enum_inputKeyUnbound:
                    break;
                case enum_inputMoveUp:
                    client->accel_dir.y -= 1;
                    break;
                case enum_inputMoveDown:
                    client->accel_dir.y += 1;
                    break;
                case enum_inputMoveLeft:
                    client->accel_dir.x -= 1;
                    break;
                case enum_inputMoveRight:
                    client->accel_dir.x += 1;
                    break;
                    
                case enum_inputSprint:
                    client->sprinting = !client->sprinting;
                    break;
                case enum_inputSneak:
                    break;
                    
                case enum_inputAttack:
                    client->attacking = true;
                    break;
                    
                case enum_inputAimLeft:
                    client->aim_dir_rotation -= 1;
                    break;
                case enum_inputAimRight:
                    client->aim_dir_rotation += 1;
                    break;
                case enum_inputAimReverse:
                    client->aim_dir += 180;
                    break;
                case enum_inputAimSpeed:
                    client->aimSpeed = aimSpeedB;
                    break;
                
                case enum_inputQuit:
                    running = false;
                    client->quitting = true;
                    break;
                    
                case enum_inputFullscreen:
                    if (fullscreen)
                        goWindowed();
                    else
                        goFullscreen();
                    break;
                
                case enum_inputSpawnZombie:
                    spawn(zombie_type, client->camera_center + client->aim_pixel_pos.to_float());
                    break;
                case enum_inputInteract:
                    client->interacting = true;
                    break;
                case enum_inputSkipDialog:
                    client->dialogVisible = 0;
                    break;
                case enum_inputDash:
                    client->dashing = true;
                    break;
                case enum_inputPause:
                    client->paused = !client->paused;
                    if (client->paused)
                        client->menuPage = PAUSE_MENU;
                    break;
                    
                default:
                    break;
            }
        }
        if (event.type == SDL_KEYUP) {
            // ignore auto repeat
            if (event.key.repeat == 1 || just_keyed_down)
                continue;
            
            // Input just deactivated:
            switch (inputIndex) {
                case enum_inputMoveLeft:
                    client->accel_dir.x += 1;
                    break;
                case enum_inputMoveRight:
                    client->accel_dir.x -= 1;
                    break;
                case enum_inputMoveUp:
                    client->accel_dir.y += 1;
                    break;
                case enum_inputMoveDown:
                    client->accel_dir.y -= 1;
                    break;
                    
                case enum_inputAttack:
                    client->attacking = false;
                    break;
                    
                case enum_inputAimLeft:
                    client->aim_dir_rotation += 1;
                    break;
                case enum_inputAimRight:
                    client->aim_dir_rotation -= 1;
                    break;
                case enum_inputAimSpeed:
                    client->aimSpeed = aimSpeedA;
                    break;
                case enum_inputInteract:
                    client->interacting = false;
                    break;
                case enum_inputDash:
                    client->dashing = false;
                    break;
            }
        }
        if (event.type == SDL_MOUSEMOTION) {
            if (event.type != 769) { // ignore bogus "keyup" move event
                mouse_moved = true;
            }
        }
        if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) { //TODO rebindable mouse buttons! TODO
            
        }
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            just_clicked = true;
            switch(event.button.button) {
                case 1: // left click
                    client->attacking = true;
                    break;
                case 2: // middle click
                    break;
                case 3: // right click
                    client->building = true;
                    break;
                default:
                    break;
            }
        }
        if (event.type == SDL_MOUSEBUTTONUP) {
            // ignore the bogus click up event
            if (just_clicked)
                continue;
            switch(event.button.button) {
                case 1: // left click
                    //printf("Left click up.\n");
                    playerClient.attacking = false;
                    break;
                case 2: // middle click
                    //printf("Middle click up.\n");
                    break;
                case 3: // right click
                    //printf("Right click up.\n");
                    client->building = false;
                    break;
                default:
                    break;
            }
        }
    }
    // Default keyboard aim sensitivity.
    if (aimSpeedA == 0)
        aimSpeedA = 2;
    if (aimSpeedA == 0)
        aimSpeedA = 4;
    if (client->aimSpeed == 0)
        client->aimSpeed = aimSpeedA;
    // send the rotation to the gun
    if (client->aim_dir_rotation < 0)
        client->aim_dir -= client->aimSpeed * dt * ((client->dashing) ? 2 : 1);
    if (client->aim_dir_rotation > 0)
        client->aim_dir += client->aimSpeed * dt * ((client->dashing) ? 2 : 1);
    // only override keyboard aim if mouse is moving
    SDL_GetMouseState(&client->aim_pixel_pos.x, &client->aim_pixel_pos.y);
    client->aim_pixel_pos.x = (client->aim_pixel_pos.x*(RSIZE/tileWidth) - window_x/2*(RSIZE/tileWidth));
    client->aim_pixel_pos.y = (client->aim_pixel_pos.y*(RSIZE/tileWidth) - window_y/2*(RSIZE/tileWidth));
    if (mouse_moved == true) {
        client->keyboardAiming = false;
        client->aim_dir = atan2(client->aim_pixel_pos.y, client->aim_pixel_pos.x) * 180 / M_PI;
    }
    else if (client->aim_dir_rotation != 0) {
        client->keyboardAiming = true;
    }
    if (client->aim_dir < 0)
            client->aim_dir += 360;
}



