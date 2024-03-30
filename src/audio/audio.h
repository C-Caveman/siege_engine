#ifndef AUDIO
#define AUDIO

#include "sfx.h"
#include "music.h"
#include "SDL2/SDL_mixer.h"

void init_audio();
void cleanup_audio();
void playSound(int soundIndex);
void playMusic(int musicIndex);

#endif
