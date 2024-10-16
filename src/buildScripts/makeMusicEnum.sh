#!/bin/bash
# Generate music X-Macro:
musicEnumFile='src/audio/music.h'
printf '#ifndef MUSIC
#define MUSIC
#include "../defs.h" 

#define MUSIC_LIST(f) \\
    f(placeholderMusic) \\\n' > $musicEnumFile
ls -p assets/audio/music | 
    grep '.ogg' |
    sed 's/^/    f(/; s/\.ogg//; s/$/) \\/'  >> $musicEnumFile
printf '
enum MUSIC_ENUM {
    MUSIC_LIST(TO_ENUM)
    NUM_MUSIC
};

#endif\n' >> $musicEnumFile
