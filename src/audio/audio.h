#ifndef AUDIO
#define AUDIO

#include "sfx.h"
#include "music.h"

void init_audio();
void cleanup_audio();
void playSound(int soundIndex);
void playMusic(int musicIndex);

#endif
