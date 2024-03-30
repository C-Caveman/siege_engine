#include "audio.h" 

#define MAX_FILENAME_LEN 256
#define MAX_SFX_NAME_LEN MAX_FILENAME_LEN - 56
#define MAX_MUSIC_NAME_LEN MAX_FILENAME_LEN - 56

Mix_Music* musics[NUM_MUSIC];
Mix_Chunk* sfx[NUM_SFX];

char musicNames[NUM_SFX][MAX_MUSIC_NAME_LEN] = { MUSIC_LIST(TO_STRING) };
char sfxNames[NUM_SFX][MAX_SFX_NAME_LEN] = { SFX_LIST(TO_STRING) };

void loadSFX() {
    char fileName[MAX_FILENAME_LEN];
    Mix_Chunk* placeholderSoundPtr = Mix_LoadWAV("placeholders/audio/sfx/placeholderSound.wav");
    if (!placeholderSoundPtr) {
        printf("*** Error: Couldn't find placeholders/audio/sfx/placeholderSound.wav!\n");
        exit(-1);
    }
    for (int i=0; i<NUM_SFX; i++) {
        snprintf(fileName, sizeof(fileName), "assets/audio/sfx/%.*s.wav", (int)strnlen(sfxNames[i], MAX_SFX_NAME_LEN), sfxNames[i]);
        printf("Loading sound: '%s'\n", fileName);
        sfx[i] =  Mix_LoadWAV(fileName);
        if (!sfx[i])
            sfx[i] = placeholderSoundPtr;
    }
}
void loadMusic() {
    char fileName[MAX_FILENAME_LEN];
    Mix_Music* placeholderMusicPtr = Mix_LoadMUS("placeholders/audio/music/placeholderMusic.ogg");
    if (!placeholderMusicPtr) {
        printf("*** Error: Couldn't find placeholders/audio/music/placeholderMusic.ogg!\n");
        exit(-1);
    }
    for (int i=0; i<NUM_MUSIC; i++) {
        snprintf(fileName, sizeof(fileName), "assets/audio/music/%.*s.ogg", (int)strnlen(musicNames[i], MAX_MUSIC_NAME_LEN), musicNames[i]);
        printf("Loading music: '%s'\n", fileName);
        musics[i] = Mix_LoadMUS(fileName);
        if (!musics[i])
            musics[i] = placeholderMusicPtr;
    }
}

void init_audio() {
    if (Mix_Init(MIX_INIT_OGG) < 1) {
        printf("*** init_audio failed: %s", SDL_GetError());
        exit(-1);
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0) {
        printf("*** init_audio failed: %s\n", Mix_GetError());
    }
    // set initial volume TODO make configurable TODO
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
    Mix_Volume(-1, MIX_MAX_VOLUME / 4);
    loadSFX();
    loadMusic();
}

void cleanup_audio() {
    //TODO cleanup all sounds and music TODO
    Mix_Quit();
    Mix_CloseAudio();
}

void playSound(int index) {
    if (index < 0 || index >= NUM_SFX)
        index = placeholderSound;
    Mix_PlayChannel(-1, sfx[index], 0);
}
void playMusic(int index) {
    if (index < 0 || index >= NUM_SFX)
        index = placeholderMusic;
     Mix_PlayMusic(musics[index], 0);
}
