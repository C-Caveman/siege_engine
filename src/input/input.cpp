#include "input.h"
#include "../graphics/graphics.h"
#include "../audio/audio.h"

#define ACCEL 1
// give access to the var needed to quit the game
//extern bool running;

// give access to screen size data
//extern int window_x, window_y;


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
        if (event.type == SDL_QUIT) // window was closed (not a keyboard event)
            running = false;
        switch( event.type ) {
            //
            // key was pressed
            //
            case SDL_KEYDOWN:
                // ignore auto repeat
                if (event.key.repeat == 1)
                    continue;
                // used to detect bogus inputs
                just_keyed_down = true;
                //
                // find the key codes with this:
                //
                //printf("Key was pressed: %d\n", event.key.keysym.sym);
                switch(event.key.keysym.sym) {
                    // here are the keyboard inputs!
                    
                    // close the game
                    case SDLK_ESCAPE:
                    case SDLK_q:
                        running = false;
                        client->quitting = true;
                        break;
                    
                    // movement with WASD
                    case SDLK_a:
                        client->accel_dir.x -= 1;
                        break;
                    case SDLK_d:
                        client->accel_dir.x += 1;
                        break;
                    case SDLK_w:
                        client->accel_dir.y -= 1;
                        break;
                    case SDLK_s:
                        client->accel_dir.y += 1;
                        break;
                        
                    // sprint
                    case 1073742049: // shift key
                        client->sprinting = !client->sprinting;
                        break;
                        
                    // sneak
                    case 1073742048: // ctrl key
                        // toggle sneaking state
                        break;
                        
                    // aim left
                    case 1073741904: // left arrow
                        client->aim_dir_rotation -= 1;
                        break;
                        
                    // aim right
                    case 1073741903: // right arrow
                        client->aim_dir_rotation += 1;
                        break;
                    
                    // aim reverse (180 degree turn)
                    case 1073741906: // up arrow
                        //TODO redo this
                        client->aim_dir += 180;
                        printf("Rotating...\n");
                        playMusic(brown);
                        break;
                    
                    case SDLK_f:
                        if (fullscreen)
                            goWindowed();
                        else
                            goFullscreen();
                        break;
                        
                    // end of key down processing
                    default:
                        break;
                }
            //
            // key was released
            //
            // a bogus keyup is generated by the x server on key down (very annoying)
            case SDL_KEYUP:
                // ignore auto repeat
                if (event.key.repeat == 1 || just_keyed_down)
                    continue;
                switch(event.key.keysym.sym) {                        
                    // WASD
                    case SDLK_a:
                        client->accel_dir.x += 1;
                        break;
                    case SDLK_d:
                        client->accel_dir.x -= 1;
                        break;
                    case SDLK_w:
                       client->accel_dir.y += 1;
                        break;
                    case SDLK_s:
                        client->accel_dir.y -= 1;
                        break;
                        
                    // aim left
                    case 1073741904: // left arrow
                        client->aim_dir_rotation += 1;
                        break;
                        
                    // aim right
                    case 1073741903: // right arrow
                        client->aim_dir_rotation -= 1;
                        break;
                        
                        
                    default:
                        break;
            }
        //
        // mouse events
        //
        case (SDL_MOUSEMOTION):
            if (event.type != 769) { // ignore bogus "keyup" move event
                mouse_moved = true;
            }
            break;
        case (SDL_MOUSEBUTTONDOWN):
            just_clicked = true;
            switch(event.button.button) {
                case 1: // left click
                    //printf("Left click down.\n");
                    client->attacking = true;
                    break;
                case 2: // middle click
                    //printf("Middle click down.\n");
                    break;
                case 3: // right click
                    //printf("Right click down.\n");
                    client->building = true;
                    break;
                default:
                    break;
        }
        case (SDL_MOUSEBUTTONUP):
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
        default:
            break;
        }
    } // end of input event queue loop
    
    // send the rotation to the gun
    if (client->aim_dir_rotation < 0)
        client->aim_dir -= 1;
    if (client->aim_dir_rotation > 0)
        client->aim_dir += 1;
    // TODO change rotation
    //TODO something with this
    // only override keyboard aim if mouse is moving
    SDL_GetMouseState(&client->aim_pixel_pos.x, &client->aim_pixel_pos.y);
    client->aim_pixel_pos.x = (client->aim_pixel_pos.x*(RSIZE/tileWidth) - window_x/2*(RSIZE/tileWidth));
    client->aim_pixel_pos.y = (client->aim_pixel_pos.y*(RSIZE/tileWidth) - window_y/2*(RSIZE/tileWidth));
    if (mouse_moved == true) {
        client->aim_dir = atan2(client->aim_pixel_pos.y, client->aim_pixel_pos.x) * 180 / M_PI;
    }
    if (client->aim_dir < 0)
            client->aim_dir += 360;
}
