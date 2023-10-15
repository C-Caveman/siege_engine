#ifndef ANIMATIONS
#define ANIMATIONS 

#define MAX_TEXTURES 256 // space allocated for textures
#define BKGRND_TEX 8
#define ANIM_NAME_LEN 16 // max length for an animation filename

// each animation loaded by the game (filenames)
#define ANIMATION_LIST(DO) \
    DO(rocket_tank) \
    DO(gun_0) \
    DO(gun_chaingun) \
    DO(gun_grenade) \
    DO(gun_railgun) \
    DO(sand) \
    DO(stone) \
    DO(stonedk) \
    DO(tiledark) \
    DO(firepit) \
    DO(knight_1) \
    DO(wall_steel) \
    DO(wall_steel_side) \
    DO(floor_test) \

#define MAKE_ENUM(x) x, 
enum animation_ids {
    ANIMATION_LIST(MAKE_ENUM)
    NUM_ANIM
};

#endif
