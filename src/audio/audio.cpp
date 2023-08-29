#include "../audio.h" 

Mix_Music* music;
Mix_Chunk* sound;

void init_audio() {
    if (Mix_Init(MIX_INIT_OGG) < 1) {
        printf("*** init_audio failed: %s", SDL_GetError());
        exit(-1);
    }
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    music = Mix_LoadMUS("audio/music/brown.ogg");
    if (!music) {
        printf("*** init_audio failed: %s\n", Mix_GetError());
    }
    sound = Mix_LoadWAV("audio/sfx/thud.wav");
    if (!sound) {
        printf("*** init_audio failed: %s\n", Mix_GetError());
    }
    // set initial volume TODO make configurable TODO
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
    Mix_Volume(-1, MIX_MAX_VOLUME / 4);
}

void cleanup_audio() {
    Mix_Quit();
    Mix_CloseAudio();
}
