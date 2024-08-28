#ifndef SFX
#define SFX

#include "../defs.h" 

#define SFX_LIST(f) \
    f(placeholderSound) \
    f(thud) \
    f(fwaf) \
    f(metalFadeIn) \
    f(metalPing) \
    f(metalThunder) \
    f(chow) \
    f(typewriterA01) \
    f(typewriterARattle1) \
    f(typewriterARattle2) \
    f(typewriterAPunct1) \
    f(typewriterAPunct2) \
    f(typewriterAPunct3) \
    f(tik) \
    f(tikLower) \
    f(voiceThudA1) \
    f(voiceThudA2) \
    f(voiceThudA3) \
    f(voiceMetalA) \
    f(voiceMetalB1) \
    f(voiceMetalB2) \
    f(voiceMetalB3) \
    f(voiceMetalB4) \
    f(voiceMetalB5) \
    f(voiceMetalB6)

enum SFX_ENUM {
    SFX_LIST(TO_ENUM)
    NUM_SFX
};

#endif
