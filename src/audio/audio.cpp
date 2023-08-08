#include "../audio.h" 

Mix_Music * music;
Mix_Chunk * sound;

void init_audio() {
    if (Mix_Init(MIX_INIT_OGG) < 1) {
        printf("*** init_audio failed: %s", SDL_GetError());
        exit(-1);
    }
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    music = Mix_LoadMUS("audio/music/brown.ogg");
    sound = Mix_LoadWAV("audio/sfx/aa_1_purple_lake2.wav");
    if (!music) {
        printf("*** init_audio failed: %s\n", Mix_GetError());
    }
    if (!sound) {
        printf("*** init_audio failed: %s\n", Mix_GetError());
    }
}

void cleanup_audio() {
    //TODO this
}
