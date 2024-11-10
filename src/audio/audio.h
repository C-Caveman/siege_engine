#ifndef AUDIO
#define AUDIO

#include "sfx.h"
#include "music.h"
#define NUM_UNUSED_SOUND_CHANNELS 2

enum soundChannels {
    CHAN_EXPLOSION,
    CHAN_WEAPON,
    CHAN_WEAPON_ALT,
    CHAN_ENGINE,
    CHAN_STEAM,
    
    CHAN_UNUSED_RANGE_START,
    CHAN_UNUSED_RANGE_END=CHAN_UNUSED_RANGE_START+NUM_UNUSED_SOUND_CHANNELS,
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

#endif
