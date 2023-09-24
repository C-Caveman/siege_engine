#ifndef ANIMATIONS
#define ANIMATIONS 
/* 
WARNING: 
don't add any curly braces before the enum,
as the curly brace character is used by
load_animation_names() to find the enum
*/

#define MAX_TEXTURES 256 // space allocated for textures
#define BKGRND_TEX 8
#define ANIM_NAME_LEN 16 // max length for an animation filename

// each animation loaded by the game (filenames)
enum animation_ids {
    rocket_tank,
    gun_0,
    gun_chaingun,
    gun_grenade,
    gun_railgun,
    sand,
    stone,
    stonedk, // feel free to add comments here
    tiledark,
    firepit,
    knight_1,
    wall_steel,
    wall_steel_side,
    NUM_ANIM,
};


#endif
