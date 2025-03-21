#ifndef AUDIO
#define AUDIO

#include "sfx.h"
#include "music.h"

enum soundChannels {
    CHAN_EXPLOSION,
    CHAN_WEAPON,
    CHAN_WEAPON_ALT,
    CHAN_ENGINE,
    CHAN_STEAM,
    CHAN_MONSTER,
    CHAN_VOICE,
    CHAN_WORLD,
    CHAN_AMBIENT,
    NUM_CHANNELS
};

void init_audio();
void cleanup_audio();
void playSound(int soundIndex);
void playSoundChannel(int soundIndex, int channelIndex);
void playMusic(int musicIndex);
void playMusicLoop(int index);
void playMusicChannelLoop(int index, int channel);
void pauseMusic();
void resumeMusic();
int isMusicPaused();
void setMusicVolume(float v);
void setSfxVolume(float v);
int isChannelPlaying(int channel);

#endif
