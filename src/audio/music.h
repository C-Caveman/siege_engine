#ifndef MUSIC
#define MUSIC

#include "../defs.h" 

#define MUSIC_LIST(f) \
    f(placeholderMusic) \
    f(brown) \
    f(spookyWind1)

enum MUSIC_ENUM {
    MUSIC_LIST(TO_ENUM)
    NUM_MUSIC
};

#endif 
