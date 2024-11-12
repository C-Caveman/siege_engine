#ifndef SFX
#define SFX
#include "../defs.h" 

#define SFX_LIST(f) \
    f(placeholderSound) \
    f(arcLamp1) \
    f(bam01) \
    f(bam02) \
    f(boop01) \
    f(chow) \
    f(chuh01) \
    f(chuh02) \
    f(click01) \
    f(click02) \
    f(explosion01) \
    f(explosion02) \
    f(explosion03) \
    f(explosion04) \
    f(fsHeavy01) \
    f(fump) \
    f(fuseLight) \
    f(fwaf) \
    f(meow) \
    f(metalFadeIn) \
    f(metalPing) \
    f(metalThunder) \
    f(rocketBoostEnd) \
    f(rocketBoostEngage02) \
    f(rocketBoostEngageNoiseless02) \
    f(rocketBoostEngage) \
    f(rocketClick01) \
    f(rocketClick02) \
    f(rocketClick03) \
    f(rocketEngineLoop02) \
    f(rocketEngineLoopFast) \
    f(rocketEngineLoopSingle) \
    f(rocketEngineLoop) \
    f(rocketEngineShutdown) \
    f(rocketReverse) \
    f(rocketSteamHiss) \
    f(rocketSteamRelease) \
    f(shlick) \
    f(tack) \
    f(thud) \
    f(thump01) \
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
    f(voiceAlert01) \
    f(voiceAlert02) \
    f(voiceAlert03) \
    f(voiceAlert04) \
    f(voiceAlert05) \
    f(voiceComs01) \
    f(voiceComs02) \
    f(voiceComs03) \
    f(voiceDeadpan) \
    f(voiceDeep) \
    f(voiceEngine01) \
    f(voiceGraw) \
    f(voiceHarsh01) \
    f(voiceHarsh02) \
    f(voiceHarsh03) \
    f(voiceHarsh04) \
    f(voiceHum01) \
    f(voiceHum02) \
    f(voiceHum03) \
    f(voiceHum04) \
    f(voiceImpactLight) \
    f(voiceJolly01) \
    f(voiceJolly02) \
    f(voiceJolly03) \
    f(voiceJolly04) \
    f(voiceLady) \
    f(voiceLamb01) \
    f(voiceLamb02) \
    f(voiceLamb03) \
    f(voiceLamb04) \
    f(voiceLamb05) \
    f(voiceLamb06) \
    f(voiceMetalA) \
    f(voiceMetalB1) \
    f(voiceMetalB2) \
    f(voiceMetalB3) \
    f(voiceRaven) \
    f(voiceRobot01a) \
    f(voiceRobot01b) \
    f(voiceRobot02a) \
    f(voiceRobot02bb) \
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
    f(voiceTickerTape01) \
    f(voiceTickerTape02) \
    f(voiceTickerTape03) \
    f(voiceTickerTape04) \
    f(wAutoCannon) \
    f(wHeavyCannon01) \
    f(wShotgun01) \
    f(zombieDie01) \

enum SFX_ENUM {
    SFX_LIST(TO_ENUM)
    NUM_SFX
};

#endif
