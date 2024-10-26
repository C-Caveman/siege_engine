#ifndef ANIMATIONS
#define ANIMATIONS 

#define ANIMATION_LIST(f) \
    f(big_chungus) \
    f(black) \
    f(crosshair01) \
    f(cubeFace) \
    f(cubeFace2) \
    f(faceBook01) \
    f(faceBookTalk01) \
    f(faceDemonA) \
    f(faceNecronomicon) \
    f(facePig01) \
    f(facePigTalk01) \
    f(faceRobot01) \
    f(faceSkullGoat01) \
    f(firepit) \
    f(floor_test) \
    f(grass1Floor) \
    f(grass1Side) \
    f(grenade01Blink) \
    f(grenade01Explode) \
    f(gun_0) \
    f(gun_chaingun) \
    f(gun_grenade) \
    f(gun_grenade_backup) \
    f(gun_plasma) \
    f(gun_railgun) \
    f(knight_1) \
    f(missing) \
    f(player_2) \
    f(player_old) \
    f(projectilePlasma) \
    f(rabbitHop01) \
    f(_reference) \
    f(rocket_tank) \
    f(sand) \
    f(solid_grey) \
    f(stone) \
    f(stonedk) \
    f(stonedk02) \
    f(sword) \
    f(tileCobble01) \
    f(tileCobble02) \
    f(tiledark) \
    f(tiledark_backup) \
    f(tileDirt01) \
    f(tileGold01) \
    f(tilegrate01) \
    f(tileMetal01) \
    f(wall_steel) \
    f(wall_steel_side) \
    f(zombie) \
    f(zzz_unused) \

enum animation_ids {
    ANIMATION_LIST(TO_ENUM)
    NUM_ANIM
};

#endif
