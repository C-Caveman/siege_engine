#ifndef SFX
#define SFX

#include "../defs.h" 

#define SFX_LIST(f) \
    f(placeholderSound) \
    f(thud)

enum SFX_ENUM {
    SFX_LIST(TO_ENUM)
    NUM_SFX
};

#endif
