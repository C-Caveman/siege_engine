#!/bin/bash
# Generate sfx X-Macro:
sfxEnumFile='src/audio/sfx.h'
printf '#ifndef SFX
#define SFX
#include "../defs.h" 

#define SFX_LIST(f) \\
    f(placeholderSound) \\\n' > $sfxEnumFile
ls -p assets/audio/sfx | 
    grep '.wav' |
    sed 's/^/    f(/; s/\.wav//; s/$/) \\/'  >> $sfxEnumFile
printf '
enum SFX_ENUM {
    SFX_LIST(TO_ENUM)
    NUM_SFX
};

#endif\n' >> $sfxEnumFile

#ls assets/graphics/animations
