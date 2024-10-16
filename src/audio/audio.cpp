#include "audio.h" 
#include "SDL2/SDL_mixer.h"

#define MAX_FILENAME_LEN 256
#define MAX_SFX_NAME_LEN MAX_FILENAME_LEN - 56
#define MAX_MUSIC_NAME_LEN MAX_FILENAME_LEN - 56

Mix_Music* musics[NUM_MUSIC] = {0};
Mix_Chunk* sfx[NUM_SFX]  = {0};

char musicNames[NUM_MUSIC][MAX_MUSIC_NAME_LEN] = { MUSIC_LIST(TO_STRING) };
char sfxNames[NUM_SFX][MAX_SFX_NAME_LEN] = { SFX_LIST(TO_STRING) };

void loadSFX() {
    char fileName[MAX_FILENAME_LEN];
    for (int i=0; i<NUM_SFX; i++) {
        sfx[i] = 0;
    }
    Mix_Chunk* placeholderSoundPtr = Mix_LoadWAV("placeholders/audio/sfx/placeholderSound.wav");
    sfx[placeholderSound] = placeholderSoundPtr;
    if (!placeholderSoundPtr) {
        printf("*** Error: Couldn't find placeholders/audio/sfx/placeholderSound.wav!\n");
        exit(-1);
    }
    for (int i=placeholderSound+1; i<NUM_SFX; i++) {
        snprintf(fileName, sizeof(fileName), "assets/audio/sfx/%.*s.wav", (int)strnlen(sfxNames[i], MAX_SFX_NAME_LEN), sfxNames[i]);
        sfx[i] =  Mix_LoadWAV(fileName);
        if (!sfx[i]) {
            sfx[i] = placeholderSoundPtr;
            printf("*** sfx file '%s' not found!\n", fileName);
        }
    }
}
void loadMusic() {
    char fileName[MAX_FILENAME_LEN];
    for (int i=0; i<NUM_MUSIC; i++) {
        musics[i] = 0;
    }
    Mix_Music* placeholderMusicPtr = Mix_LoadMUS("placeholders/audio/music/placeholderMusic.ogg");
    musics[placeholderMusic] = placeholderMusicPtr;
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
    Mix_Volume(-1, MIX_MAX_VOLUME / 3);
    loadSFX();
    loadMusic();
}

void cleanup_audio() {
    for (int i=0; i<NUM_SFX; i++) {
        if (sfx[i] != 0 && sfx[i] != sfx[placeholderSound]) {
            Mix_FreeChunk(sfx[i]);
        }
    }
    Mix_FreeChunk(sfx[placeholderSound]);
    for (int i=0; i<NUM_MUSIC; i++) {
        if (musics[i] != 0 && musics[i] != musics[placeholderMusic])
            Mix_FreeMusic(musics[i]);
    }
    Mix_FreeMusic(musics[placeholderMusic]);
    Mix_Quit();
    Mix_CloseAudio();
}

void playSound(int index) {
    if (index < 0 || index >= NUM_SFX)
        index = placeholderSound;
    Mix_PlayChannel(-1, sfx[index], 0);
}
void playSoundChannel(int index, int channelIndex) {
    if (index < 0 || index >= NUM_SFX)
        index = placeholderSound;
    Mix_PlayChannel(channelIndex, sfx[index], 0);
}
void playMusic(int index) {
    if (index < 0 || index >= NUM_SFX)
        index = placeholderMusic;
     Mix_PlayMusic(musics[index], 0);
}
void playMusicLoop(int index) {
    if (index < 0 || index >= NUM_SFX)
        index = placeholderMusic;
     Mix_PlayMusic(musics[index], -1);
}
void pauseMusic() {
    Mix_PauseMusic();
}
void resumeMusic() {
    Mix_ResumeMusic();
}
int isMusicPaused() {
    return Mix_PausedMusic();
}
