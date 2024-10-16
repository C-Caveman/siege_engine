#ifndef MUSIC
#define MUSIC
#include "../defs.h" 

#define MUSIC_LIST(f) \
    f(placeholderMusic) \
    f(brown) \
    f(dorian01) \
    f(spookyWind1) \
    f(squaretoothX01) \

enum MUSIC_ENUM {
    MUSIC_LIST(TO_ENUM)
    NUM_MUSIC
};

#endif
