#include "SDL2/SDL_mixer.h" 

//SDL_AudioDeviceID audio_device;
extern Mix_Music * music;
extern Mix_Chunk * sound;

void init_audio();
void cleanup_audio();
