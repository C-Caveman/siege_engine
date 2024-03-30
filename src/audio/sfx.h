#ifndef SFX
#define SFX

#include "../defs.h" 

#define SFX_LIST(f) \
    f(placeholderSound) \
    f(placeBlock) \
    f(destroyBlock)

enum SFX_ENUM {
    SFX_LIST(TO_ENUM)
    NUM_SFX
};

#endif
