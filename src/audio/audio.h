#ifndef AUDIO
#define AUDIO

#include "sfx.h"
#include "music.h"
#define NUM_UNUSED_SOUND_CHANNELS 16

enum soundChannels {
    TYPER,
    TYPER2,
    TYPER3,
    TYPER4,
    CHAN_EXPLOSION,
    CHAN_WEAPON,
    
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
void pauseMusic();
void resumeMusic();
int isMusicPaused();

#endif
