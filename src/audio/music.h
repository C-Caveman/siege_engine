#ifndef MUSIC
#define MUSIC
#include "../defs.h" 

#define MUSIC_LIST(f) \
    f(placeholderMusic) \
    f(brown) \
    f(campfire01) \
    f(dorian01) \
    f(loopSilence) \
    f(rocketEngineLoopMusic) \
    f(spookyWind1) \
    f(squaretoothX01) \

enum MUSIC_ENUM {
    MUSIC_LIST(TO_ENUM)
    NUM_MUSIC
};

#endif
