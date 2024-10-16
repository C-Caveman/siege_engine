#ifndef SFX
#define SFX
#include "../defs.h" 

#define SFX_LIST(f) \
    f(placeholderSound) \
    f(aa_1_purple_lake2) \
    f(arcLamp1) \
    f(chow) \
    f(chuh01) \
    f(chuh02) \
    f(fwaf) \
    f(metalFadeIn) \
    f(metalPing) \
    f(metalThunder) \
    f(thud) \
    f(tikLowerBackup) \
    f(tikLower) \
    f(tikLowerWhisper01) \
    f(tikLowerWhisper02) \
    f(tik) \
    f(typewriterA01) \
    f(typewriterA05) \
    f(typewriterAPunct1) \
    f(typewriterAPunct2) \
    f(typewriterAPunct3) \
    f(typewriterARattle1) \
    f(typewriterARattle2) \
    f(voiceDeadpan) \
    f(voiceDeep) \
    f(voiceEngine01) \
    f(voiceGraw) \
    f(voiceImpactLight) \
    f(voiceLady) \
    f(voiceLamb) \
    f(voiceMetalA) \
    f(voiceMetalB1) \
    f(voiceMetalB2) \
    f(voiceMetalB3) \
    f(voiceRaven) \
    f(voiceRobot01a) \
    f(voiceRobot01b) \
    f(voiceRobot02a) \
    f(voiceRobot02b) \
    f(voiceRobot02c) \
    f(voiceRobot02d) \
    f(voiceRobot03a) \
    f(voiceRobot03b) \
    f(voiceRobot03c) \
    f(voiceRobot03d) \
    f(voiceRobot03e) \
    f(voiceRobot03f) \
    f(voiceThudA1) \
    f(voiceThudA2) \
    f(voiceThudA3) \

enum SFX_ENUM {
    SFX_LIST(TO_ENUM)
    NUM_SFX
};

#endif
