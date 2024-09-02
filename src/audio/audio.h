#ifndef AUDIO
#define AUDIO

#include "sfx.h"
#include "music.h"

enum soundChannels {
    TYPER,
    TYPER2,
    TYPER3,
    TYPER4,
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
